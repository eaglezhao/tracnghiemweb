using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace WebcamServer
{
    internal class Camera
    {
        public event ImageReceiver.FrameHandler FrameReceived;

        private string ipAddress;
        private List<string> clients;
        private ImageReceiver imageReceiver;
        private UdpClient udpSender;

        public Camera(string ip)
        {
            ipAddress = ip;
            clients = new List<string>();
            udpSender = new UdpClient();
        }

        ~Camera()
        {
            udpSender.Close();
        }

        public string IPAddress
        {
            get { return ipAddress; }
        }

        public string[] GetClients()
        {
            return clients.ToArray();
        }

        public void AddClient(string ip)
        {
            lock (this)
            {
                clients.Add(ip);

                if (imageReceiver == null)
                {
                    imageReceiver = new ImageReceiver(ipAddress, 399);
                    imageReceiver.ErrorRaised += new ImageReceiver.ErrorHandler(imageReceiver_ErrorRaised);
                    imageReceiver.FrameReceived += new ImageReceiver.FrameHandler(imageReceiver_FrameReceived);

                    ThreadStart o = new ThreadStart(imageReceiver.ThreadProc);
                    Thread thread = new Thread(o);
                    thread.Name = "Imaging";
                    thread.Start();
                }
            }
        }

        public void RemoveClient(string ip)
        {
            lock (this)
            {
                clients.Remove(ip);

                if (clients.Count == 0)
                    Stop();
            }
        }

        public void RemoveClientAt(int index)
        {
            lock (this)
            {
                clients.RemoveAt(index);
            }
        }

        void imageReceiver_FrameReceived(object sender, byte[] b)
        {
            lock (this)
            {
                foreach (string s in clients)
                {
                    try
                    {
                        if (s == "localhost")
                            FrameReceived(this, b);
                        else
                            udpSender.Send(b, b.Length, s, 9999);
                    }
                    catch(Exception e)
                    {
                        //MessageBox.Show(e.Message);
                    }
                }
            }
        }

        private void imageReceiver_ErrorRaised(object sender, string msg)
        {
            Stop();
        }

        private void Stop()
        {
            if (imageReceiver != null)
            {
                imageReceiver.Done = true;
                imageReceiver = null;
            }
        }
    }
}
