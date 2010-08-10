using System;
using System.Net;
using System.Net.Sockets;

namespace WpfWebcamServer
{
    public class ImageReceiver : IDisposable
    {
        // udp port for listening
        private UdpClient udpClient;

        public ImageReceiver()
        {
            udpClient = new UdpClient(0);
            udpClient.BeginReceive(new AsyncCallback(OnReceive), udpClient);
        }

        public int ListenPort
        {
            get
            {
                return ((IPEndPoint)udpClient.Client.LocalEndPoint).Port;
            }
        }

        private void OnReceive(IAsyncResult ar)
        {
            lock(this)
            {
                if (udpClient != null)
                {
                    IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);
                    byte[] buffer = udpClient.EndReceive(ar, ref remoteEP);

                    if (FrameReceived != null)
                        FrameReceived(this, buffer);

                    udpClient.BeginReceive(new AsyncCallback(OnReceive), udpClient);
                }
            }
        }

        public void Dispose()
        {
            lock (this)
            {
                udpClient.Close();
                udpClient = null;
            }
        }

        public event FrameHandler FrameReceived;
    }

    public delegate void FrameHandler(object sender, byte[] b);
}
