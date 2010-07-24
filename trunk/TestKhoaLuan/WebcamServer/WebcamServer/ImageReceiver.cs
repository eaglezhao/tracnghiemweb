using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace WebcamServer
{
    public class ImageReceiver
    {
        public delegate void ErrorHandler(object sender, string msg);
        public delegate void FrameHandler(object sender, byte[] b);
        public event ErrorHandler ErrorRaised;
        public event FrameHandler FrameReceived;

        // Abort indicator
        public bool Done;

        // Client connection to server
        private TcpClient tcpClient;

        // stream to read from
        private NetworkStream networkStream;

        public ImageReceiver(string address, int nPort)
        {
            Done = false;

            // Connect to the server and get the stream
            tcpClient = new TcpClient(address, nPort);
            tcpClient.NoDelay = false;
            tcpClient.ReceiveTimeout = 5000;
            tcpClient.ReceiveBufferSize = 20000;
            networkStream = tcpClient.GetStream();
        }

        public void ThreadProc()
        {
            string s;
            int iBytesComing, iBytesRead, iOffset;
            byte[] byLength = new byte[10];
            byte[] byImage = new byte[1000];

            do
            {
                try
                {
                    // Read the fixed length string that
                    // tells the image size
                    iBytesRead = networkStream.Read(byLength, 0, 10);

                    if (iBytesRead != 10)
                    {
                        if (ErrorRaised != null)
                            ErrorRaised(this, "No response from host");
                        break;
                    }
                    s = Encoding.ASCII.GetString(byLength);
                    iBytesComing = Convert.ToInt32(s);

                    // Make sure our buffer is big enough
                    if (iBytesComing > byImage.Length)
                    {
                        byImage = new byte[iBytesComing];
                        tcpClient.ReceiveBufferSize = iBytesComing + 10;
                    }

                    // Read the image
                    iOffset = 0;

                    do
                    {
                        iBytesRead = networkStream.Read(byImage, iOffset, iBytesComing - iOffset);
                        if (iBytesRead != 0)
                        {
                            iOffset += iBytesRead;
                        }
                        else
                        {
                            if (ErrorRaised != null)
                                ErrorRaised(this, "No response from host");
                        }
                    } while ((iOffset != iBytesComing) && (!Done));


                    if (!Done)
                    {
                        // Write back a byte
                        networkStream.Write(byImage, 0, 1);

                        // trigger the event that we have a frame now
                        if (FrameReceived != null)
                            FrameReceived(this, byImage);
                    }
                }
                catch (Exception e)
                {
                    // If we get out of sync, we're probably toast, since
                    // there is currently no resync mechanism
                    if (ErrorRaised != null)
                        ErrorRaised(this, e.Message);
                }

            } while (!Done);

            networkStream.Close();
            tcpClient.Close();
            networkStream = null;
            tcpClient = null;
        }
    }
}
