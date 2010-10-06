using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using System.Text;
using System.Linq;
using WpfWebcamServer.Devices.View;
using Network;

namespace WpfWebcamServer.Devices.Presenter
{
    public sealed class DevicePresenter : IFrameHandler, IDisposable
    {
        private ConcurrentDictionary<string, Device> _devices = new ConcurrentDictionary<string, Device>();
        private IDeviceView _view;
        private TcpListener _listener;
        private volatile bool _listening = true;
        private bool _disposed = false;

        public DevicePresenter(IDeviceView view)
        {
            _view = view;
            _listener = new TcpListener(IPAddress.Any, _view.DevicePort);
            _listener.Start();

            AcceptConnections();
            VerifyConnections();
        }

        ~DevicePresenter()
        {
            Dispose(false);
        }

        public void Discover()
        {
            Discover(IPAddress.Broadcast.ToString(), _view.BroadcastPort);
        }

        public void Discover(string address, int port)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            try
            {
                byte[] msg = Encoding.ASCII.GetBytes("discovery " + ((IPEndPoint)_listener.LocalEndpoint).Port);
                UdpClient udpClient = new UdpClient(address, port);
                udpClient.EnableBroadcast = true;
                udpClient.Send(msg, msg.Length);
                udpClient.Close();
            }
            catch (SocketException ex)
            {
                _view.Notification = ex.Message;
            }
        }

        public Device FindDevice(string name)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            Device d = null;
            _devices.TryGetValue(name, out d);
            return d;
        }

        public IEnumerable<string> GetWebcamNames()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            return _devices.Keys.Where((key) => key.StartsWith("WC:"));
        }

        public void HandleFrame(string sender, byte[] buffer)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);
            
            _view.UpdateViewer(sender, new System.IO.MemoryStream(buffer));
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
                    if (_listening)
                    {
                        _listening = false;
                        IPEndPoint ep = (IPEndPoint)_listener.LocalEndpoint;
                        TcpClient t = new TcpClient("localhost", ep.Port);
                        if (t != null)
                            t.Close();
                        _listener.Stop();

                        foreach (Device d in _devices.Values)
                            d.Dispose();
                    }
                }
                _disposed = true;
            }
        }

        private void AcceptConnections()
        {
            Task.Factory.StartNew(() =>
            {
                while (_listening)
                {
                    try
                    {
                        Messenger msgr = new Messenger(_listener.AcceptTcpClient());
                        Task.Factory.StartNew(() => IdentifyDevice(msgr));
                    }
                    catch (SocketException ex) { _view.Status = ex.Message; }
                }
            });
        }

        private void IdentifyDevice(Messenger msgr)
        {
            try
            {
                MessageType type;
                msgr.TcpClient.ReceiveTimeout = 30000;
                byte[] buffer = msgr.ReceiveMessage(out type);
                msgr.TcpClient.ReceiveTimeout = 0;

                if (type == MessageType.Command)
                {
                    string id = Encoding.ASCII.GetString(buffer);
                    Device d = null;

                    if (id.StartsWith("WC:"))
                    {
                        Webcam w = new Webcam(msgr, id, _view.BasePath + @"\" + id.Substring(4));
                        w.Disconnected += OnDisconnect;
                        w.MotionDetected += OnMotionDetect;
                        w.Recorded += (source) => _view.Status = "Webcam " + source.Name + " started recording";
                        w.MotionStopped += (source) =>
                            {
                                ((Webcam)source).StopRecording();
                                _view.Status = "Webcam " + source.Name + " stopped recording";
                            };
                        w.AddClient(this);
                        _view.AddedWebcam = w.Name;
                        _view.Status = "Webcam " + w.Name + " is now connected";
                        d = w;
                    }
                    else if (id.StartsWith("CL:"))
                    {
                        Client c = new Client(msgr, this, id);
                        c.Disconnected += OnDisconnect;
                        c.WebcamChanged += (source) => _view.Status = ((Client)source).EventArg;
                        _view.AddedClient = c.Name;
                        _view.Status = "Client " + c.Name + " is now connected";
                        d = c;
                    }

                    if (d == null)
                        msgr.Dispose();
                    else
                    {
                        _devices.AddOrUpdate(id, d, (key, value) =>
                            {
                                value.Dispose();
                                return d;
                            });
                        d.Listen();
                    }
                }
            }
            catch { msgr.Dispose(); }
        }

        private void VerifyConnections()
        {
            Thread t = new Thread(new ThreadStart(() =>
            {
                while (_listening)
                {
                    foreach (Device d in _devices.Values)
                        d.Ping();
                    Thread.Sleep(3000);
                }
            }));
            t.IsBackground = true;
            t.Start();
        }

        private void OnDisconnect(Device source)
        {
            source.Disconnected -= OnDisconnect;
            _devices.TryRemove(source.Name, out source);

            string deviceType = source.GetType().Name;
            if (deviceType == "Webcam")
                _view.RemovedWebcam = source.Name;
            else
                _view.RemovedClient = source.Name;

            _view.Status = string.Format("{0} {1} has disconnected", deviceType, source.Name);
            source.Dispose();
        }

        private void OnMotionDetect(Device source)
        {
            Webcam w = (Webcam)source;
            if (w.FirstAlarm)
                _view.Status = "Webcam " + w.Name + " detected motion";

            if (_view.AllowVideoRecord)
                w.StartRecording();
            if (_view.AllowSoundAlarm)
                w.Alarm();
        }
    }
}
