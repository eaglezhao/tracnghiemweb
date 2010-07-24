using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Collections;
using System.IO;
using System.Net;
using System.Threading;
using System.Windows.Forms;

namespace WebcamServer
{
    class ConnectionManager
    {
        public event EventHandler DeviceDetected;

        const int BROADCASTPORT = 1111;

        private UdpClient udpListener;
        private UdpClient udpSender;
        private CameraManager camManager;
        private Camera current = null;
        private List<string> clients;
        private WebcamServer server;

        public ConnectionManager(WebcamServer s)
        {
            server = s;
            camManager = new CameraManager();
            clients = new List<string>();
            udpSender = new UdpClient();
            udpListener = new UdpClient(1112);
            udpListener.BeginReceive(new AsyncCallback(OnReceive), udpListener);
        }

        public CameraManager CameraManager
        {
            get { return camManager; }
        }

        public List<string> Clients
        {
            get { return clients; }
        }

        public void AutoDetect()
        {
            try
            {
                byte[] msg = Encoding.ASCII.GetBytes("helu " + Dns.GetHostAddresses(Dns.GetHostName())[0].ToString());
                UdpClient udpClient = new UdpClient(IPAddress.Broadcast.ToString(), BROADCASTPORT);
                udpClient.EnableBroadcast = true;
                udpClient.Send(msg, msg.Length);
            }
            catch (SocketException e)
            {
            }
        }

        public void DiscoverCam(string address)
        {
            try
            {
                byte[] msg = Encoding.ASCII.GetBytes("helu " + Dns.GetHostAddresses(Dns.GetHostName())[0].ToString());
                udpSender.Send(msg, msg.Length, address, BROADCASTPORT);
            }
            catch (SocketException e)
            {
            }
        }

        public void ViewCamera(string ip)
        {
            current = camManager.FindCamera(ip);
            if (current != null)
            {
                current.AddClient("localhost");
                current.FrameReceived += new ImageReceiver.FrameHandler(server.OnFrameReceived);
            }
        }

        public void StopViewing()
        {
            if (current != null)
            {
                current.RemoveClient("localhost");
            }
        }

        private void OnReceive(IAsyncResult ar)
        {
            IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 1112);
            string[] msg = Encoding.ASCII.GetString(udpListener.EndReceive(ar, ref remoteEP)).Split(' ');

            if (msg[0] == "cam")
            {
                camManager.AddCamera(msg[1]);

                if (DeviceDetected != null)
                    DeviceDetected(this, EventArgs.Empty);
            }
            else if (msg[0] == "cli")
            {
                clients.Add(msg[1]);
            }
            else if (msg[0] == "req")
            {
                //if (clients.Contains(msg[1]))
                //{
                    Camera c = camManager.FindCamera(msg[2]);
                    if (c != null)
                        c.AddClient(msg[1]);
                //}
            }
            else if (msg[0] == "sto")
            {
                Camera c = camManager.FindCamera(msg[2]);
                if(c != null)
                    c.RemoveClient(msg[1]);
            }
            udpListener.BeginReceive(new AsyncCallback(OnReceive), udpListener);
        }
    }
}
