using System;
using System.Linq;
using WpfWebcamServer.Views;
using System.Net.Sockets;
using System.Net;
using System.Text;

namespace WpfWebcamServer.Webcams
{
    public sealed class WebcamPresenter : IFrameHandler, IDisposable
    {
        private System.Collections.Generic.HashSet<Webcam> _webcams = new System.Collections.Generic.HashSet<Webcam>();
        private System.Threading.ReaderWriterLockSlim rwl = new System.Threading.ReaderWriterLockSlim();
        private IWebcamView _view;
        private TcpListener _webcamListener;
        private volatile bool _listening = true;

        public WebcamPresenter(IWebcamView view)
        {
            _view = view;
            _webcamListener = new TcpListener(IPAddress.Any, _view.WebcamPort);
            _webcamListener.Start();
            
            AcceptWebcamConnections();
        }

        public Webcam FindWebcam(string name)
        {
            rwl.EnterReadLock();
            Webcam w = _webcams.Where((webcam) => webcam.Name == name).SingleOrDefault();
            rwl.ExitReadLock();
            return w;
        }

        public string[] GetWebcamNames()
        {
            rwl.EnterReadLock();
            string[] names = _webcams.Select((webcam) => webcam.Name).ToArray();
            rwl.ExitReadLock();
            return names;
        }

        public void Discover()
        {
            Discover(IPAddress.Broadcast.ToString());
        }

        public void Discover(string address)
        {
            try
            {
                byte[] msg = Encoding.ASCII.GetBytes("dis " + ((IPEndPoint)_webcamListener.LocalEndpoint).Port);
                UdpClient udpClient = new UdpClient(address, _view.BroadcastPort);
                udpClient.EnableBroadcast = true;
                udpClient.Send(msg, msg.Length);
                udpClient.Close();
            }
            catch (SocketException ex)
            {
                _view.Notification = ex.Message;
            }
        }

        public void Dispose()
        {
            if (_listening)
            {
                _listening = false;
                IPEndPoint ep = (IPEndPoint)_webcamListener.LocalEndpoint;
                TcpClient t = new TcpClient("127.0.0.1", ep.Port);
                if (t != null)
                    t.Close();
                _webcamListener.Stop();

                rwl.EnterReadLock();
                foreach (Webcam w in _webcams)
                    w.Dispose();
                rwl.ExitReadLock();
            }
        }

        public void HandleFrame(string sender, byte[] buffer)
        {
            _view.UpdateViewer(sender, new System.IO.MemoryStream(buffer));
        }

        private void AcceptWebcamConnections()
        {
            System.Threading.Tasks.Task.Factory.StartNew(() =>
                {
                    while (_listening)
                    {
                        try
                        {
                            Webcam w = new Webcam(_webcamListener.AcceptTcpClient(), _view.BasePath);

                            w.Connected += OnConnect;
                            w.Disconnected += OnDisconnect;
                            w.MotionDetected += OnMotionDetect;
                            w.Recorded += (source) => _view.Status = "Webcam " + source.Name + " started recording";
                            w.MotionStopped += (source) =>
                                {
                                    source.StopRecording();
                                    _view.Status = "Webcam " + source.Name + " stopped recording";
                                };
                        }
                        catch (SocketException ex)
                        {
                            _view.Status = ex.Message;
                        }
                    }
                });
        }

        private void OnConnect(Webcam source)
        {
            rwl.EnterWriteLock();
            _webcams.Add(source);
            rwl.ExitWriteLock();

            source.AddClient(this);
            _view.AddedWebcam = source.Name;
            _view.Status = "Webcam " + source.Name + " is now connected";
        }

        private void OnDisconnect(Webcam source)
        {
            rwl.EnterWriteLock();
            _webcams.Remove(source);
            rwl.ExitWriteLock();

            _view.RemovedWebcam = source.Name;
            _view.Status = "Webcam " + source.Name + " has disconnected";
            source.Disconnected -= OnDisconnect;
            source.Dispose();
        }

        private void OnMotionDetect(Webcam source)
        {
            if (source.FirstAlarm)
                _view.Status = "Webcam " + source.Name + " detected motion";

            if (_view.AllowVideoRecord)
                source.StartRecording();
            if (_view.AllowSoundAlarm)
                source.Alarm();
        }
    }
}
