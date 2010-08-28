using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading;
using System.IO;
using AForge.Video.VFW;

namespace WpfWebcamServer.Webcams
{
    internal class Webcam : IDisposable
    {
        private ImageReceiver imageReceiver;
        private string name;
        private AVIWriter aviWriter;
        private string folderPath;
        private TcpClient tcpClient;
        private NetworkStream networkStream;

        public Webcam(TcpClient client, string basePath)
        {
            tcpClient = client;
            networkStream = tcpClient.GetStream();
            name = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address.ToString();
            folderPath = basePath;
            RecordVideo = false;
            MotionDetected = false;

            ThreadStart listener = new ThreadStart(OnTcpReceived);
            Thread thread = new Thread(listener);
            thread.Start();

            imageReceiver = new ImageReceiver();
            imageReceiver.FrameReceived += new FrameHandler(imageReceiver_FrameReceived);
            SendMessage("hello " + imageReceiver.ListenPort);
        }

        ~Webcam()
        {
            Dispose();
        }

        public bool RecordVideo { get; set; }
        public bool MotionDetected { get; set; }
        public string Name { get { return name; } }

        public void AddClient(FrameHandler callback)
        {
            lock (this)
            {
                if (FrameReceived == null)
                    SendMessage("start");
                FrameReceived += callback;
            }
        }

        public void RemoveClient(FrameHandler callback)
        {
            lock (this)
            {
                FrameReceived -= callback;
                if (FrameReceived == null)
                    SendMessage("stop");
            }
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
                    AddClient(Record);
                    MotionAlarmed(this, "recording");

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
            RemoveClient(Record);
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
            if (imageReceiver != null)
                imageReceiver.Dispose();
            if (networkStream != null)
                networkStream.Close();
            if (tcpClient != null)
                tcpClient.Close();

            imageReceiver = null;
            networkStream = null;
            tcpClient = null;
        }

        private void SendMessage(string msg)
        {
            byte[] buffer = Encoding.ASCII.GetBytes(msg);
            tcpClient.Client.Send(Encoding.ASCII.GetBytes((buffer.Length).ToString("d8") + "\r\n"));
            tcpClient.Client.Send(buffer);
        }

        // Client has sent data, or has disconnected
        private void OnTcpReceived()
        {
            int iBytesComing, iBytesRead, iOffset;

            try
            {
                do
                {
                    byte[] byteBuffer = new byte[10];

                    // Read the fixed length string that tells the message size
                    iBytesRead = networkStream.Read(byteBuffer, 0, 10);

                    if (iBytesRead != 10)
                    {
                        if (Disconnected != null)
                            Disconnected(this, name + " has disconnected");
                        break;
                    }

                    // Read the message
                    iBytesComing = Convert.ToInt32(Encoding.ASCII.GetString(byteBuffer));
                    byteBuffer = new byte[iBytesComing];
                    iOffset = 0;

                    do
                    {
                        iBytesRead = networkStream.Read(byteBuffer, iOffset, iBytesComing - iOffset);
                        if (iBytesRead != 0)
                            iOffset += iBytesRead;
                        else if (Disconnected != null)
                            Disconnected(this, name + " has disconnected");
                    } while (iOffset != iBytesComing);

                    ProcessMessage(Encoding.ASCII.GetString(byteBuffer));

                } while (true);
            }
            catch
            {
                if (Disconnected != null)
                    Disconnected(this, name + " has disconnected");
            }
        }

        private void imageReceiver_FrameReceived(object sender, byte[] b)
        {
            lock (this)
            {
                if (FrameReceived != null)
                    FrameReceived(this, b);
            }
        }

        private void ProcessMessage(string msg)
        {
            string[] message = msg.Split(' ');

            if (message[0] == "hello")
            {
                name = message[1];
                folderPath += "\\" + name;
                if (Connected != null)
                    Connected(this, name);
            }
            else if (message[0] == "record")
            {
                if (MotionAlarmed != null)
                    MotionAlarmed(this, "start");
            }
            else if (message[0] == "stop-record")
            {
                if (MotionAlarmed != null)
                    MotionAlarmed(this, "stop");
            }
        }

        private void Record(object Sender, byte[] buffer)
        {
            if(aviWriter != null)
                aviWriter.AddFrame(new System.Drawing.Bitmap(new MemoryStream(buffer)));
        }

        public event ConnectionHandler Connected;
        public event ConnectionHandler Disconnected;
        public event ConnectionHandler MotionAlarmed;
        private FrameHandler FrameReceived;
    }
}
