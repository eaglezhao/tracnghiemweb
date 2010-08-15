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
            int iBytesComing, iBytesRead, iOffset;
            string message, type;

            try
            {
                bool done = false;

                do
                {
                    byte[] byteBuffer = new byte[11];

                    // Read the fixed length string that tells the message size and type
                    iBytesRead = networkStream.Read(byteBuffer, 0, 11);

                    if (iBytesRead != 11)
                    {
                        if (Disconnected != null)
                            Disconnected(this, "Disconnected to server");
                        break;
                    }

                    type = Encoding.ASCII.GetString(new byte[] { byteBuffer[0] });
                    byteBuffer = byteBuffer.Skip(1).ToArray();
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
                                Disconnected(this, "Disconnected to server");
                            done = true;
                        }
                    } while ((iOffset != iBytesComing) && !done);

                    if (type == "0" && MessageReceived != null)
                    {
                        message = Encoding.ASCII.GetString(byteBuffer);
                        MessageReceived(this, message);
                    }
                    else if (type == "1" && FrameReceived != null)
                    {
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
