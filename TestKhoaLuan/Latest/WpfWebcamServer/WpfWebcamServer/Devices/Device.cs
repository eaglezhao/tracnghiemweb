using System;
using System.Net.Sockets;
using System.Threading.Tasks;
using Network;

namespace WpfWebcamServer.Devices
{
    public abstract class Device : IPingable, IDisposable
    {
        private Messenger _msgr;
        private bool _disposed = false;
        private volatile bool _ping = true;

        public Device(TcpClient client)
        {
            _msgr = new Messenger(client);
        }

        public Device(Messenger messenger)
        {
            _msgr = messenger;
        }

        ~Device()
        {
            Dispose(false);
        }

        public string Name { get; protected set; }

        public void SendMessage(string message)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            try
            {
                _msgr.SendMessage(message);
                _ping = false;
            }
            catch (SocketException) { RaiseDisconnectEvent(); }
        }

        public void SendMessage(byte[] buffer)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            try
            {
                _msgr.SendMessage(buffer, MessageType.Image);
                _ping = false;
            }
            catch (SocketException) { RaiseDisconnectEvent(); }
        }

        public void Listen()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            Task.Factory.StartNew(() =>
            {
                try
                {
                    MessageType type;
                    while (true)
                    {
                        _ping = false;
                        byte[] buffer = _msgr.ReceiveMessage(out type);
                        ProcessMessage(buffer, type);
                    }
                }
                catch { RaiseDisconnectEvent(); }
            });
        }

        public void Ping()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            if (_ping)
                SendMessage("ping");
            else
                _ping = true;
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
                    _msgr.Dispose();
                }
                _disposed = true;
            }
        }

        protected abstract void ProcessMessage(byte[] buffer, MessageType type);

        private void RaiseDisconnectEvent()
        {
            DeviceEventHandler handler = Disconnected;
            if (handler != null)
                handler(this);
        }

        public event DeviceEventHandler Disconnected;
    }

    public delegate void DeviceEventHandler(Device source);
}
