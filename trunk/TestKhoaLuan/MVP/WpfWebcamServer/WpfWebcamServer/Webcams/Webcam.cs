using System;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using AForge.Video.VFW;
using WpfWebcamServer.Exceptions;

namespace WpfWebcamServer.Webcams
{
    public class Webcam : IFrameHandler, IDisposable
    {
        private System.Collections.Generic.HashSet<IFrameHandler> frameHandlers =
            new System.Collections.Generic.HashSet<IFrameHandler>();
        private System.Threading.ReaderWriterLockSlim rwl = new System.Threading.ReaderWriterLockSlim();
        private UdpClient udpClient = new UdpClient(0);
        private AVIWriter aviWriter;
        private TcpClient tcpClient;
        private NetworkStream networkStream;
        private string folderPath;

        public Webcam(TcpClient client, string basePath)
        {
            tcpClient = client;
            networkStream = tcpClient.GetStream();
            folderPath = basePath;
            RecordVideo = false;
            FirstAlarm = true;

            ReceiveMessage();
            udpClient.BeginReceive(new AsyncCallback(ReceiveFrame), udpClient);
            SendMessage("hello " + ((IPEndPoint)udpClient.Client.LocalEndPoint).Port);
        }

        public bool RecordVideo { get; set; }
        public bool FirstAlarm { get; private set; }
        public string Name { get; private set; }

        public void AddClient(IFrameHandler handler)
        {
            rwl.EnterWriteLock();
            frameHandlers.Add(handler);
            if (frameHandlers.Count == 1)
                SendMessage("start");
            rwl.ExitWriteLock();

            if (handler is Clients.Client)
                ((Clients.Client)handler).Disconnected += OnClientDisconnect;
        }

        public void RemoveClient(IFrameHandler handler)
        {
            rwl.EnterWriteLock();
            frameHandlers.Remove(handler);
            if (frameHandlers.Count == 0)
                SendMessage("stop");
            rwl.ExitWriteLock();

            if (handler is Clients.Client)
                ((Clients.Client)handler).Disconnected -= OnClientDisconnect;
        }

        public void StartRecording()
        {
            if (!RecordVideo)
            {
                DateTime date = DateTime.Now;
                string filePath = folderPath + String.Format("\\{0}-{1}-{2} {3}-{4}-{5}.avi",
                    date.Year, date.Month, date.Day, date.Hour, date.Minute, date.Second);
                int width = 640;
                int height = 480;
                int frameRate = 15;

                try
                {
                    if (!Directory.Exists(folderPath))
                        Directory.CreateDirectory(folderPath);

                    SendMessage("recording");
                    // create AVI writer
                    aviWriter = new AVIWriter("wmv3");
                    aviWriter.FrameRate = frameRate;
                    // open AVI file
                    aviWriter.Open(filePath, width, height);
                    // register as a client to receive frame
                    AddClient(this);
                    RaiseRecordEvent();

                    RecordVideo = true;
                }
                catch (ApplicationException)
                {
                    if (aviWriter != null)
                    {
                        aviWriter.Dispose();
                        aviWriter = null;
                    }
                }
            }
        }

        public void StopRecording()
        {
            RemoveClient(this);
            RecordVideo = false;
            if (aviWriter != null)
            {
                aviWriter.Dispose();
                aviWriter = null;
            }
        }

        public void Alarm()
        {
            SendMessage("alarm");
        }

        public void Dispose()
        {
            udpClient.Close();
            networkStream.Close();
            tcpClient.Close();
        }

        private void SendMessage(string msg)
        {
            byte[] buffer = Encoding.ASCII.GetBytes(msg);
            tcpClient.Client.Send(Encoding.ASCII.GetBytes((buffer.Length).ToString("d8") + "\r\n"));
            tcpClient.Client.Send(buffer);
        }

        public void HandleFrame(string sender, byte[] buffer)
        {
            if (aviWriter != null)
                aviWriter.AddFrame(new System.Drawing.Bitmap(new MemoryStream(buffer)));
        }

        // Client has sent data, or has disconnected
        private void ReceiveMessage()
        {
            Task.Factory.StartNew(() =>
                {
                    try
                    {
                        int iBytesComing, iBytesRead, iOffset;
                        while (true)
                        {
                            byte[] byteBuffer = new byte[10];

                            // Read the fixed length string that tells the message size
                            iBytesRead = networkStream.Read(byteBuffer, 0, 10);
                            if (iBytesRead != 10)
                                throw new ConnectionException();

                            // Read the message
                            iBytesComing = Convert.ToInt32(Encoding.ASCII.GetString(byteBuffer));
                            byteBuffer = new byte[iBytesComing];
                            iOffset = 0;

                            do
                            {
                                iBytesRead = networkStream.Read(byteBuffer, iOffset, iBytesComing - iOffset);
                                if (iBytesRead != 0)
                                    iOffset += iBytesRead;
                                else
                                    throw new ConnectionException();
                            } while (iOffset != iBytesComing);

                            ProcessMessage(Encoding.ASCII.GetString(byteBuffer));
                        }
                    }
                    catch
                    {
                        RaiseDisconnectEvent();
                    }
                });
        }

        private void ReceiveFrame(IAsyncResult ar)
        {
            try
            {
                IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);
                byte[] buffer = udpClient.EndReceive(ar, ref remoteEP);

                DispatchFrame(buffer);
                udpClient.BeginReceive(new AsyncCallback(ReceiveFrame), udpClient);
            }
            catch
            {
                RaiseDisconnectEvent();
            }
        }

        private void DispatchFrame(byte[] b)
        {
            rwl.EnterReadLock();
            foreach (IFrameHandler handler in frameHandlers)
                handler.HandleFrame(Name, b);
            rwl.ExitReadLock();
        }

        private void ProcessMessage(string msg)
        {
            string[] message = msg.Split(' ');

            if (message[0] == "hello")
            {
                Name = message[1];
                folderPath += "\\" + Name;
                RaiseConnectEvent();
            }
            else if (message[0] == "record")
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

        private void OnClientDisconnect(Clients.Client source)
        {
            Task.Factory.StartNew(() => RemoveClient(source));
        }

        private void RaiseConnectEvent()
        {
            if (Connected != null)
                Connected(this);
        }

        private void RaiseDisconnectEvent()
        {
            if (Disconnected != null)
                Disconnected(this);
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

        public event WebcamEventHandler Connected;
        public event WebcamEventHandler Disconnected;
        public event WebcamEventHandler MotionDetected;
        public event WebcamEventHandler Recorded;
        public event WebcamEventHandler MotionStopped;
    }

    public delegate void WebcamEventHandler(Webcam source);
}
