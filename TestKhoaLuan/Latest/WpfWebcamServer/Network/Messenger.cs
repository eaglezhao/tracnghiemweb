using System;
using System.Net.Sockets;
using System.Text;

namespace Network
{
    public class Messenger : IMessenger, IDisposable
    {
        private bool _disposed = false;
        private NetworkStream _networkStream;

        public Messenger(TcpClient client)
        {
            TcpClient = client;
            _networkStream = TcpClient.GetStream();
        }

        ~Messenger()
        {
            Dispose(false);
        }

        public TcpClient TcpClient { get; private set; }

        public void SendMessage(string message)
        {
            SendMessage(Encoding.ASCII.GetBytes(message), MessageType.Command);
        }

        public void SendMessage(byte[] buffer, MessageType type)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            string s = ((int)type) + buffer.Length.ToString("d8") + "\r\n";
            TcpClient.Client.Send(Encoding.ASCII.GetBytes(((int)type) + buffer.Length.ToString("d8") + "\r\n"));
            TcpClient.Client.Send(buffer);
        }

        public byte[] ReceiveMessage(out MessageType type)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            // Read the fixed length string that tells the message size and type
            byte[] byteBuffer = new byte[11];
            int bytesRead = _networkStream.Read(byteBuffer, 0, 11);
            if (bytesRead != 11)
                throw new ConnectionException(bytesRead.ToString());

            string header = Encoding.ASCII.GetString(byteBuffer);
            int bytesComing = Convert.ToInt32(header.Substring(1));
            type = (MessageType)Convert.ToInt32(header.Substring(0, 1));
            byteBuffer = new byte[bytesComing];

            // Read the message
            int offset = 0;
            do
            {
                bytesRead = _networkStream.Read(byteBuffer, offset, bytesComing - offset);
                if (bytesRead != 0)
                    offset += bytesRead;
                else
                    throw new ConnectionException(header);
            } while (offset != bytesComing);

            return byteBuffer;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    _networkStream.Close();
                    TcpClient.Close();
                }
                _disposed = true;
            }
        }
    }
}
