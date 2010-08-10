/****************************************************************************
While the underlying libraries are covered by LGPL, this sample is released 
as public domain.  It is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  
*****************************************************************************/

using System;
using System.Net.Sockets;
using System.IO;
using System.Net;
using System.Text;
using System.Threading;

namespace WpfWebcamRecorder
{
    internal class ConnectionManager : IDisposable
    {
        #region Member variables

        private UdpClient udpReceiver;
        private UdpClient udpSender;
        private TcpClient tcpClient;
        private int listenPort;
        private volatile bool done;
        private NetworkStream networkStream;

        #endregion

        public ConnectionManager(int nPortListen)
        {
            listenPort = nPortListen;
            udpReceiver = new UdpClient(listenPort);
            done = false;

            udpReceiver.BeginReceive(new AsyncCallback(OnUdpReceived), udpReceiver);
        }

        public void Dispose()
        {
            lock (this)
            {
                // close everything down
                if (udpReceiver != null)
                    udpReceiver.Close();

                udpReceiver = null;
                done = true;
            }
        }
        ~ConnectionManager()
        {
            Dispose();
        }

        public void SendImage(byte[] b)
        {
            if (udpSender != null)
                udpSender.Send(b, b.Length);
        }

        public void SendImage(MemoryStream m)
        {
            if(udpSender != null)
                SendImage(m.GetBuffer());
        }

        public void SendMessage(string msg)
        {
            if (tcpClient != null)
            {
                byte[] buffer = Encoding.ASCII.GetBytes(msg);

                tcpClient.Client.Send(Encoding.ASCII.GetBytes((buffer.Length).ToString("d8") + "\r\n"));
                tcpClient.Client.Send(buffer);
            }
        }

        private void OnUdpReceived(IAsyncResult ar)
        {
            /* If the received message is a discovery message, answer by establishing a connection to the server.
             * Because we can only connect to one server at a time, there's no need to reset the callback. But if an
             * exception occurred, or if the message is not a discovery message, reset the callback to continue
             * waiting for future message.
             */
            if (udpReceiver != null)
            {
                try
                {
                    IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);
                    string[] messages = Encoding.ASCII.GetString(udpReceiver.EndReceive(ar, ref remoteEP)).Split(' ');

                    if (messages[0] == "dis")
                    {
                        tcpClient = new TcpClient();

                        tcpClient.Connect(remoteEP.Address.ToString(), Convert.ToInt32(messages[1]));
                        networkStream = tcpClient.GetStream();

                        if (Connected != null)
                            Connected(this, "Connected to server " + remoteEP.Address.ToString() + " : " + messages[1]);

                        done = false;
                        ThreadStart listener = new ThreadStart(OnTcpReceived);
                        Thread thread = new Thread(listener);
                        thread.Start();
                    }
                    else
                        udpReceiver.BeginReceive(new AsyncCallback(OnUdpReceived), udpReceiver);
                }
                catch
                {
                    if (tcpClient != null)
                        tcpClient.Close();
                    if (udpSender != null)
                        udpSender.Close();

                    tcpClient = null;
                    udpSender = null;

                    udpReceiver.BeginReceive(new AsyncCallback(OnUdpReceived), udpReceiver);
                }
            }
        }

        // Client has sent data, or has disconnected
        private void OnTcpReceived()
        {
            int iBytesComing, iBytesRead, iOffset;
            string message;

            do
            {
                try
                {
                    byte[] byteBuffer = new byte[10];

                    // Read the fixed length string that tells the message size
                    iBytesRead = networkStream.Read(byteBuffer, 0, 10);

                    if (iBytesRead != 10)
                    {
                        if (Disconnected != null)
                            Disconnected(this, "Server has disconnected");
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
                            if (Disconnected != null)
                            {
                                Disconnected(this, "Server has disconnected");
                                done = true;
                            }
                    } while ((iOffset != iBytesComing) && (!done));

                    message = Encoding.ASCII.GetString(byteBuffer);

                    if (message.StartsWith("hello"))
                    {
                        udpSender = new UdpClient(((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address.ToString(),
                            Convert.ToInt32(message.Substring(6)));
                        SendMessage("hello " + Dns.GetHostName());
                    }
                    else if (MessageReceived != null)
                        MessageReceived(this, message);
                }
                catch (Exception e)
                {
                    if (Disconnected != null)
                        Disconnected(this, "Server has disconnected");
                    done = true;
                }
            } while (!done);

            if(networkStream != null)
                networkStream.Close();
            if(tcpClient != null)
                tcpClient.Close();
            if(udpSender != null)
                udpSender.Close();
            
            networkStream = null;
            tcpClient = null;
            udpSender = null;

            if (udpReceiver != null)
                udpReceiver.BeginReceive(new AsyncCallback(OnUdpReceived), udpReceiver);
        }

        public event ConnectionHandler Connected;
        public event ConnectionHandler Disconnected;
        public event MessageHandler MessageReceived;
    }

    public delegate void ConnectionHandler(Object sender, string message);
    public delegate void MessageHandler(Object sender, string message);
}