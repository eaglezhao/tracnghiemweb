using System;
using System.Text;
using System.Net.Sockets;
using System.Net;
using WpfWebcamServer.Webcams;
using System.Threading.Tasks;
using WpfWebcamServer.Exceptions;

namespace WpfWebcamServer.Clients
{
    public class Client : IFrameHandler, IDisposable
    {
        private TcpClient tcpClient;
        private NetworkStream networkStream;
        private WebcamPresenter wcPresenter;
        private int count = 0;

        public Client(TcpClient client, WebcamPresenter presenter)
        {
            tcpClient = client;
            networkStream = tcpClient.GetStream();
            wcPresenter = presenter;
            IP = ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString();

            ReceiveMessage();
        }

        public void Dispose()
        {
            networkStream.Close();
            tcpClient.Close();
        }

        public string IP { get; private set; }
        public string EventArg { get; private set; }

        public void HandleFrame(string sender, byte[] buffer)
        {
            Send(buffer, count++);
        }

        public void Send(byte[] buffer, int frameNumber)
        {
            try
            {
                string header = frameNumber >= 0 ? frameNumber.ToString("d10") : frameNumber.ToString("d9");
                tcpClient.Client.Send(Encoding.ASCII.GetBytes(header + (buffer.Length).ToString("d8") + "\r\n"));
                tcpClient.Client.Send(buffer);
            }
            catch (ObjectDisposedException) { }
            catch (SocketException) { RaiseDisconnectEvent(); }
        }

        public void Send(string message)
        {
            Send(Encoding.ASCII.GetBytes(message), -1);
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

        private void ProcessMessage(string msg)
        {
            string[] message = msg.Split(' ');

            if (message[0] == "start")
            {
                Webcam w = wcPresenter.FindWebcam(message[1]);

                if (w != null)
                {
                    w.AddClient(this);

                    EventArg = "Client " + IP + " requested webcam " + w.Name;
                    RaiseWebcamChangeEvent();
                }
            }
            else if (message[0] == "stop")
            {
                Webcam w = wcPresenter.FindWebcam(message[1]);

                if (w != null)
                {
                    w.RemoveClient(this);

                    EventArg = "Client " + IP + " stopped viewing webcam " + w.Name;
                    RaiseWebcamChangeEvent();
                }
            }
            else if (message[0] == "get-list")
            {
                string wcList = "webcam-list";
                foreach (string s in wcPresenter.GetWebcamNames())
                    wcList += " " + s;
                Send(wcList);
            }
        }

        private void RaiseDisconnectEvent()
        {
            ClientEventHandler handler = Disconnected;
            if (handler != null)
                handler(this);
        }

        private void RaiseWebcamChangeEvent()
        {
            if (WebcamChanged != null)
                WebcamChanged(this);
        }

        public event ClientEventHandler WebcamChanged;
        public event ClientEventHandler Disconnected;
    }

    public delegate void ClientEventHandler(Client source);
}
