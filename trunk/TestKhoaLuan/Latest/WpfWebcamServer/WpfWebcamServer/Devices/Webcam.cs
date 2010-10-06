using System;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using AForge.Video.VFW;
using Network;

namespace WpfWebcamServer.Devices
{
    public class Webcam : Device, IFrameHandler
    {
        private System.Collections.Generic.HashSet<IFrameHandler> _frameHandlers =
            new System.Collections.Generic.HashSet<IFrameHandler>();
        private System.Threading.ReaderWriterLockSlim _rwl = new System.Threading.ReaderWriterLockSlim();
        private AVIWriter _aviWriter;
        private string _folderPath;

        public Webcam(Messenger messenger, string name, string path)
            : base(messenger)
        {
            _folderPath = path;
            Name = name;
            RecordVideo = false;
            FirstAlarm = true;
        }

        public bool RecordVideo { get; set; }
        public bool FirstAlarm { get; private set; }

        public void AddClient(IFrameHandler handler)
        {
            _rwl.EnterWriteLock();
            _frameHandlers.Add(handler);
            _rwl.ExitWriteLock();
        }

        public void RemoveClient(IFrameHandler handler)
        {
            _rwl.EnterWriteLock();
            _frameHandlers.Remove(handler);
            _rwl.ExitWriteLock();
        }

        public void StartRecording()
        {
            if (!RecordVideo)
            {
                DateTime date = DateTime.Now;
                string filePath = _folderPath + String.Format("\\{0}-{1}-{2} {3}-{4}-{5}.avi",
                    date.Year, date.Month, date.Day, date.Hour, date.Minute, date.Second);
                int width = 640;
                int height = 480;
                int frameRate = 15;
                
                try
                {
                    if (!Directory.Exists(_folderPath))
                        Directory.CreateDirectory(_folderPath);

                    _aviWriter = new AVIWriter("wmv3");
                    _aviWriter.FrameRate = frameRate;
                    _aviWriter.Open(filePath, width, height);

                    SendMessage("recording");
                    AddClient(this);
                    RaiseRecordEvent();
                    RecordVideo = true;
                }
                catch
                {
                    if (_aviWriter != null)
                    {
                        _aviWriter.Dispose();
                        _aviWriter = null;
                    }
                }
            }
        }

        public void StopRecording()
        {
            RemoveClient(this);
            RecordVideo = false;
            if (_aviWriter != null)
            {
                _aviWriter.Dispose();
                _aviWriter = null;
            }
        }

        public void Alarm()
        {
            SendMessage("alarm");
        }

        public void HandleFrame(string sender, byte[] buffer)
        {
            if (_aviWriter != null)
                _aviWriter.AddFrame(new System.Drawing.Bitmap(new MemoryStream(buffer)));
        }

        protected override void ProcessMessage(byte[] buffer, MessageType type)
        {
            if (type == MessageType.Image)
                DispatchFrame(buffer);
            else
            {
                string[] message = Encoding.ASCII.GetString(buffer).Split(' ');

                if (message[0] == "record")
                {
                    RaiseMotionDetectEvent();
                    FirstAlarm = false;
                }
                else if (message[0] == "stop-record")
                {
                    RaiseMotionStopEvent();
                    FirstAlarm = true;
                }
            }
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (_aviWriter != null)
                _aviWriter.Dispose();
        }

        private void DispatchFrame(byte[] buffer)
        {
            _rwl.EnterReadLock();
            foreach (IFrameHandler handler in _frameHandlers)
            {
                try { handler.HandleFrame(Name, buffer); }
                catch { Task.Factory.StartNew(() => RemoveClient(handler)); }
            }
            _rwl.ExitReadLock();
        }

        private void RaiseMotionDetectEvent()
        {
            if (MotionDetected != null)
                MotionDetected(this);
        }

        private void RaiseRecordEvent()
        {
            if (Recorded != null)
                Recorded(this);
        }

        private void RaiseMotionStopEvent()
        {
            if (MotionStopped != null)
                MotionStopped(this);
        }

        public event DeviceEventHandler MotionDetected;
        public event DeviceEventHandler Recorded;
        public event DeviceEventHandler MotionStopped;
    }
}
