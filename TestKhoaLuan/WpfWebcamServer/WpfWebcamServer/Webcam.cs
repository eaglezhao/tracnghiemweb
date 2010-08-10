using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading;

namespace WpfWebcamServer
{
    internal class Webcam : IDisposable
    {
        private ImageReceiver imageReceiver;
        private string name;
        private Video.IO.AVIWriter aviWriter;
        private string filePath;
        private TcpClient tcpClient;
        private NetworkStream networkStream;

        public Webcam(TcpClient client, string basePath)
        {
            tcpClient = client;
            networkStream = tcpClient.GetStream();
            name = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address.ToString();
            filePath = basePath + "\\" + name + "\\" + DateTime.Now.ToString() + ".avi";
            IsRecording = false;
            aviWriter = new Video.IO.AVIWriter();

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

        public bool IsRecording { get; set; }
        public string Name { get { return name; } }

        public void AddClient(FrameHandler callback)
        {
            lock (this)
            {
                if(FrameReceived == null)
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

        }

        public void StopRecording()
        {

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
                        Dispose();
                        break;
                    }

                    iBytesComing = Convert.ToInt32(Encoding.ASCII.GetString(byteBuffer));
                    byteBuffer = new byte[iBytesComing];

                    // Read the message
                    iOffset = 0;

                    do
                    {
                        iBytesRead = networkStream.Read(byteBuffer, iOffset, iBytesComing - iOffset);
                        if (iBytesRead != 0)
                            iOffset += iBytesRead;
                        else
                        {
                            if (Disconnected != null)
                                Disconnected(this, name + " has disconnected");
                            Dispose();
                        }
                    } while (iOffset != iBytesComing);

                    ProcessMessage(Encoding.ASCII.GetString(byteBuffer));

                } while (true);
            }
            catch
            {
                if (Disconnected != null)
                    Disconnected(this, name + " has disconnected");
                Dispose();
            }
        }

        private void imageReceiver_FrameReceived(object sender, byte[] b)
        {
            if(FrameReceived != null)
                FrameReceived(this, b);
        }

        private void ProcessMessage(string msg)
        {
            string[] message = msg.Split(' ');

            if (message[0] == "hello")
            {
                name = message[1];
                if (Connected != null)
                    Connected(this, name);
            }
        }

        private void SendMessage(string msg)
        {
            byte[] buffer = Encoding.ASCII.GetBytes(msg);
            tcpClient.Client.Send(Encoding.ASCII.GetBytes((buffer.Length).ToString("d8") + "\r\n"));
            tcpClient.Client.Send(buffer);
        }

        public event ConnectionHandler Connected;
        public event ConnectionHandler Disconnected;
        private FrameHandler FrameReceived;
    }

    public delegate void ConnectionHandler(Object sender, string message);
}
