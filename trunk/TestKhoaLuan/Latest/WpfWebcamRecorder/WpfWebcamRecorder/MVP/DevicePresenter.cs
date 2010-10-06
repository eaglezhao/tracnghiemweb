using System;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Drawing.Imaging;
using System.IO;
using System.Drawing;
using System.Media;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using Sensor;
using DirectShowLib;
using Network;

namespace WpfWebcamRecorder.MVP
{
    public sealed class DevicePresenter : IDisposable
    {
        private static readonly ImageCodecInfo _codecInfo;
        private Device _device;
        private Messenger _messenger;
        private IDeviceView _view;
        private EncoderParameters _encoderParams;
        private SoundPlayer _player;
        private List<ISensor> _sensors = new List<ISensor>();
        private ISensor _currentSensor;
        private ManualResetEvent disposeReady = new ManualResetEvent(false);
        private volatile bool _done;
        private volatile bool _requested;
        private bool _disposed;
        private bool _isRecording;
        private bool _isAlarming;
        private int _port;
        private int _deviceIndex;
        private int _sensorIndex;
        private double _sensitivity;
        private int _quality;

        public DevicePresenter(IDeviceView view)
        {
            _view = view;
            _view.Devices = GetWebcamList();

            LoadPlugins();
            _view.Sensors = _sensors;

            FrameRate = 15;
            VideoWidth = 640;
            VideoHeight = 480;

            Listen();
        }

        static DevicePresenter()
        {
            ImageCodecInfo[] encoders = ImageCodecInfo.GetImageEncoders();
            for (int j = 0; j < encoders.Length; ++j)
            {
                if (encoders[j].MimeType == "image/jpeg")
                    _codecInfo = encoders[j];
            }
        }

        ~DevicePresenter()
        {
            Dispose(false);
        }

        public int FrameRate { get; private set; }
        public int VideoWidth { get; private set; }
        public int VideoHeight { get; private set; }

        public int SelectedDevice
        {
            get { return _deviceIndex; }
            set
            {
                try
                {
                    _deviceIndex = value;
                    if (_device != null)
                    {
                        disposeReady.Reset();
                        if (_requested)
                        {
                            _requested = false;
                            disposeReady.WaitOne();
                            _requested = true;
                        }
                        _device.Dispose();
                    }
                    _device = new Device(_deviceIndex, FrameRate, VideoWidth, VideoHeight);

                    if (_requested)
                        StreamVideo(_codecInfo, _encoderParams);
                }
                catch (Exception ex) { _view.Notification = ex.Message; }
            }
        }

        public int SelectedSensor
        {
            get { return _sensorIndex; }
            set
            {
                try
                {
                    _sensorIndex = value;
                    _currentSensor = _sensors[_sensorIndex];
                    _currentSensor.Sensitivity = SensorSensitivity;
                }
                catch (Exception ex) { _view.Notification = ex.Message; }
            }
        }

        public int ImageQuality
        {
            get { return _quality; }
            set
            {
                if (value != 0)
                {
                    _quality = value;
                    EncoderParameter param = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, _quality);
                    _encoderParams = new EncoderParameters(1);
                    _encoderParams.Param[0] = param;
                }
            }
        }

        public double SensorSensitivity
        {
            get { return _sensitivity; }
            set
            {
                _sensitivity = value;
                if (_currentSensor != null)
                    _currentSensor.Sensitivity = value;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    _done = true;
                    UdpClient client = new UdpClient("localhost", _port);
                    client.Send(new byte[] { 1 }, 1);
                    client.Close();

                    if (_messenger != null)
                        _messenger.Dispose();
                    if (_device != null)
                    {
                        disposeReady.Reset();
                        if (_requested)
                        {
                            _requested = false;
                            disposeReady.WaitOne();
                        }
                        _device.Dispose();
                    }
                }
                _disposed = true;
            }
        }

        private string[] GetWebcamList()
        {
            DsDevice[] capDevices = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            string[] wcNames = new string[capDevices.Length];

            for (int i = 0; i < capDevices.Length; i++)
                wcNames[i] = capDevices[i].Name;

            return wcNames;
        }

        private void LoadPlugins()
        {
            string path = Directory.GetCurrentDirectory();
            path = path.Substring(0, path.IndexOf(@"\bin\Debug")) + @"\Sensors";

            foreach (string f in Directory.GetFiles(path, "*.*"))
            {
                try
                {
                    Assembly assembly = Assembly.LoadFile(f);
                    foreach (Type type in assembly.GetTypes())
                    {
                        if (type.IsClass && type.IsPublic)
                        {
                            Type[] interfaces = type.GetInterfaces();
                            if (((IList)interfaces).Contains(typeof(ISensor)))
                            {
                                ISensor sensor = (ISensor)Activator.CreateInstance(type);
                                sensor.IsActive = true;
                                _sensors.Add(sensor);
                            }
                        }
                    }
                }
                catch (Exception ex) { _view.Notification = ex.Message; }
            }
        }

        private void Listen()
        {
            Task.Factory.StartNew(() =>
                {
                    _port = _view.Port;
                    UdpClient client = new UdpClient(_port);
                    IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);

                    _done = false;
                    while (!_done)
                    {
                        try
                        {
                            string[] messages = Encoding.ASCII.GetString(client.Receive(ref remoteEP)).Split(' ');
                            if (messages[0] == "discovery")
                            {
                                _messenger = new Messenger(new TcpClient(remoteEP.Address.ToString(), Convert.ToInt32(messages[1])));
                                _messenger.SendMessage("WC:" + Dns.GetHostName());
                                _view.Status = "Connected to server " + remoteEP.Address.ToString() + " : " + messages[1];
                                _requested = true;

                                StreamVideo(_codecInfo, _encoderParams);

                                MessageType type;
                                while (true)
                                {
                                    byte[] buffer = _messenger.ReceiveMessage(out type);
                                    ProcessMessage(buffer, type);
                                }
                            }
                        }
                        catch
                        {
                            _requested = false;
                            _messenger.Dispose();
                            _messenger = null;
                            _view.Status = "Server has disconnected";
                        }
                    }
                    client.Close();
                });
        }

        private void ProcessMessage(byte[] buffer, MessageType type)
        {
            string[] message = Encoding.ASCII.GetString(buffer).Split(' ');

            if (message[0] == "recording")
                _isRecording = true;
            else if (message[0] == "alarm")
                Alarm();
        }

        private void StreamVideo(ImageCodecInfo codecInfo, EncoderParameters encoderParams)
        {
            MemoryStream m = new MemoryStream(20000);
            Bitmap image = null;
            IntPtr ip = IntPtr.Zero;
            Font fontOverlay = new Font("Times New Roman", 14, System.Drawing.FontStyle.Bold,
                System.Drawing.GraphicsUnit.Point);
            DateTime lastMotion = DateTime.Now;
            int interval = 5;

            _isRecording = false;
            _player = new SoundPlayer();
            _player.LoadCompleted += new System.ComponentModel.AsyncCompletedEventHandler((obj, arg) => _player.PlayLooping());

            Task.Factory.StartNew(() =>
                {
                    _device.Start();
                    while (_requested)
                    {
                        try
                        {
                            // capture image
                            ip = _device.GetBitMap();
                            image = new Bitmap(_device.Width, _device.Height, _device.Stride, System.Drawing.Imaging.PixelFormat.Format24bppRgb, ip);
                            image.RotateFlip(RotateFlipType.RotateNoneFlipY);

                            _currentSensor.ProcessFrame(ref image);

                            if (_currentSensor.IsAlarming)
                            {
                                if (!_isRecording && (DateTime.Now.Second % interval == 0))
                                    _messenger.SendMessage("record");
                                lastMotion = DateTime.Now;
                            }
                            else
                            {
                                if (DateTime.Now.Subtract(lastMotion).Seconds > interval)
                                {
                                    if (_isRecording)
                                    {
                                        _messenger.SendMessage("stop-record");
                                        _isRecording = false;
                                    }
                                    if (_isAlarming)
                                    {
                                        _player.Stop();
                                        _isAlarming = false;
                                    }
                                }
                            }

                            // add text that displays date time to the image
                            image.AddText(fontOverlay, 10, 10, DateTime.Now.ToString());

                            // save it to jpeg using quality options
                            image.Save(m, codecInfo, encoderParams);

                            // send the jpeg image if server requests it
                            if (_messenger != null)
                                _messenger.SendMessage(m.GetBuffer(), MessageType.Image);
                        }
                        catch (SocketException)
                        {
                            _messenger.Dispose();
                            _messenger = null;
                            _requested = false;
                        }
                        catch { }
                        finally
                        {
                            // clean up
                            m.SetLength(0);
                            if (image != null)
                                image.Dispose();
                            if (ip != IntPtr.Zero)
                            {
                                Marshal.FreeCoTaskMem(ip);
                                ip = IntPtr.Zero;
                            }
                        }
                    }

                    _device.Pause();
                    _player.Stop();
                    _player.Dispose();

                    fontOverlay.Dispose();
                    disposeReady.Set();
                });
        }

        private void Alarm()
        {
            if (!_isAlarming)
            {
                try
                {
                    if (!string.IsNullOrEmpty(_view.SoundPath))
                        _player.SoundLocation = _view.SoundPath;
                    else
                        _player.Stream = Properties.Resources.alarm;

                    _player.LoadAsync();
                    _isAlarming = true;
                }
                catch
                {
                    try
                    {
                        _player.Stream = Properties.Resources.alarm;
                        _player.LoadAsync();
                        _isAlarming = true;
                    }
                    catch { }
                }
            }
        }
    }
}
