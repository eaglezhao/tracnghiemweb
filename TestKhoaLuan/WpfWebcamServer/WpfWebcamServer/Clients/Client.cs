using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Threading;
using System.Net;

namespace WpfWebcamServer.Clients
{
    internal class Client : IDisposable
    {
        private TcpClient tcpClient;
        private NetworkStream networkStream;
        private string ip;

        public Client(TcpClient client)
        {
            tcpClient = client;
            networkStream = tcpClient.GetStream();
            ip = ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString();

            ThreadStart listener = new ThreadStart(OnTcpReceived);
            Thread thread = new Thread(listener);
            thread.Start();
        }

        public void Dispose()
        {
            lock (this)
            {
                if (networkStream != null)
                    networkStream.Close();
                if (tcpClient != null)
                    tcpClient.Close();

                networkStream = null;
                tcpClient = null;
            }
        }

        public TcpClient TcpClient { get { return tcpClient; } }
        public string IP { get { return ip; } }

        public void Send(object sender, byte[] buffer)
        {
            Send(sender, buffer, MessageType.Image);
        }

        public void Send(object sender, byte[] buffer, MessageType type)
        {
            //try
            //{
                tcpClient.Client.Send(Encoding.ASCII.GetBytes(((int)type) + (buffer.Length).ToString("d8") + "\r\n"));
                tcpClient.Client.Send(buffer);
            //}
            //catch (SocketException) { }
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
                            Disconnected(this, "Client has disconnected");
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
                                Disconnected(this, "Client has disconnected");
                        }
                    } while (iOffset != iBytesComing);

                    if (RequestReceived != null)
                        RequestReceived(this, Encoding.ASCII.GetString(byteBuffer));
                } while (true);
            }
            catch
            {
                if (Disconnected != null)
                    Disconnected(this, "Client has disconnected");
            }
        }

        public event ConnectionHandler Disconnected;
        public event ConnectionHandler RequestReceived;
    }

    public enum MessageType { Command, Image }
}
