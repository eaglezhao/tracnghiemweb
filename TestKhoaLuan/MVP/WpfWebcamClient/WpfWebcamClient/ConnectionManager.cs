using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Threading;

namespace WpfWebcamClient
{
    internal sealed class ConnectionManager
    {
        private TcpClient tcpClient;
        private NetworkStream networkStream;
        private int count = 0;

        ~ConnectionManager()
        {
            Disconnect();
        }

        public void Connect(string address, int port)
        {
            try
            {
                tcpClient = new TcpClient(address, port);
                networkStream = tcpClient.GetStream();

                ThreadStart listener = new ThreadStart(OnReceive);
                Thread thread = new Thread(listener);
                thread.Start();
            }
            catch
            {
                Disconnect();
                throw;
            }
        }

        public void Disconnect()
        {
            if (networkStream != null)
                networkStream.Close();
            if (tcpClient != null)
                tcpClient.Close();

            networkStream = null;
            tcpClient = null;
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

        private void OnReceive()
        {
            int bytesComing, bytesRead, offset, frameNumber;
            string message;

            try
            {
                bool done = false;

                do
                {
                    byte[] byteBuffer = new byte[20];

                    // Read the fixed length string that tells the message size and type
                    bytesRead = networkStream.Read(byteBuffer, 0, 20);

                    if (bytesRead != 20)
                    {
                        if (Disconnected != null)
                            Disconnected(this, "Disconnected to server");
                        break;
                    }

                    frameNumber = Convert.ToInt32(Encoding.ASCII.GetString(byteBuffer.Take(10).ToArray()));
                    byteBuffer = byteBuffer.Skip(10).ToArray();
                    bytesComing = Convert.ToInt32(Encoding.ASCII.GetString(byteBuffer));
                    byteBuffer = new byte[bytesComing];
                    
                    // Read the message
                    offset = 0;

                    do
                    {
                        bytesRead = networkStream.Read(byteBuffer, offset, bytesComing - offset);
                        if (bytesRead != 0)
                            offset += bytesRead;
                        else
                        {
                            if (Disconnected != null)
                                Disconnected(this, "Disconnected to server");
                            done = true;
                        }
                    } while ((offset != bytesComing) && !done);

                    if (frameNumber < 0 && MessageReceived != null)
                    {
                        message = Encoding.ASCII.GetString(byteBuffer);
                        MessageReceived(this, message);
                    }
                    else if (frameNumber > count && FrameReceived != null)
                    {
                        count = frameNumber;
                        FrameReceived(this, byteBuffer);
                    }
                } while (!done);
            }
            catch
            {
                if (Disconnected != null)
                    Disconnected(this, "Disconnected to server");
            }
            finally
            {
                Disconnect();
            }
        }

        public event ConnectionHandler Disconnected;
        public event ConnectionHandler MessageReceived;
        public event FrameHandler FrameReceived;
    }

    public delegate void ConnectionHandler(Object sender, string message);
    public delegate void FrameHandler(object sender, byte[] b);
}
