using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Linq;
using Network;

namespace WpfWebcamClient.MVP
{
    public sealed class ClientPresenter : IDisposable
    {
        private IClientView _view;
        private Messenger _msgr;
        private bool _disposed;

        public ClientPresenter(IClientView view)
        {
            _view = view;
        }

        ~ClientPresenter()
        {
            Dispose(false);
        }

        public void Connect(string address, int port)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            try
            {
                _msgr = new Messenger(new TcpClient(address, port));
                _msgr.SendMessage("CL:" + ((IPEndPoint)_msgr.TcpClient.Client.LocalEndPoint).Address.ToString());
                _msgr.SendMessage("get-list");

                _view.Status = "Connected to server " + address;
                _view.IsConnected = true;

                Listen();
            }
            catch
            {
                Disconnect();
                _view.Status = "Failed to connect to server";
            }
        }

        public void GetWebcamList()
        {
            _msgr.SendMessage("get-list");
        }

        public void StartStreaming(string wcName)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            _msgr.SendMessage("start " + wcName);
            _view.IsStreaming = true;
        }

        public void StopStreaming(string wcName)
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            _msgr.SendMessage("start " + wcName);
            _view.IsStreaming = false;
        }

        public void Disconnect()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            if (_msgr != null)
                _msgr.Dispose();
            if (_view.IsConnected)
            {
                _view.Status = "Disconnected to server";
                _view.IsConnected = false;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    Disconnect();
                }
                _disposed = true;
            }
        }

        private void Listen()
        {
            Task.Factory.StartNew(() =>
            {
                try
                {
                    MessageType type;
                    while (true)
                    {
                        byte[] buffer = _msgr.ReceiveMessage(out type);
                        ProcessMessage(buffer, type);
                    }
                }
                catch { Disconnect(); }
            });
        }

        private void ProcessMessage(byte[] buffer, MessageType type)
        {
            if (type == MessageType.Image)
                _view.CurrentFrame = buffer;
            else
            {
                string[] msg = Encoding.ASCII.GetString(buffer).Split(' ');
                if (msg[0] == "webcam-list")
                    _view.Webcams = msg.Skip(1).ToArray();
            }
        }
    }
}
