using System;
using System.Text;
using WpfWebcamServer.Views;
using System.Net.Sockets;
using System.Net;

namespace WpfWebcamServer.Clients
{
    public sealed class ClientPresenter : IDisposable
    {
        private System.Collections.Generic.HashSet<Client> _clients = new System.Collections.Generic.HashSet<Client>();
        private System.Threading.ReaderWriterLockSlim rwl = new System.Threading.ReaderWriterLockSlim();
        private IClientView _view;
        private TcpListener _clientListener;
        private volatile bool _listening = true;

        public ClientPresenter(IClientView view)
        {
            _view = view;
            _clientListener = new TcpListener(IPAddress.Any, _view.ClientPort);
            _clientListener.Start();

            AcceptClientConnections();
        }

        public void Dispose()
        {
            if (_listening)
            {
                _listening = false;
                IPEndPoint ep = (IPEndPoint)_clientListener.LocalEndpoint;
                TcpClient t = new TcpClient("127.0.0.1", ep.Port);
                if (t != null)
                    t.Close();
                _clientListener.Stop();

                rwl.EnterReadLock();
                foreach (Client w in _clients)
                    w.Dispose();
                rwl.ExitReadLock();
            }
        }

        private void AcceptClientConnections()
        {
            System.Threading.Tasks.Task.Factory.StartNew(() =>
                {
                    while (_listening)
                    {
                        try
                        {
                            Client c = new Client(_clientListener.AcceptTcpClient(), _view.WCPresenter);
                            _view.AddedClient = c.IP;
                            _view.Status = "Client " + c.IP + " is now connected";

                            rwl.EnterWriteLock();
                            _clients.Add(c);
                            rwl.ExitWriteLock();

                            c.Disconnected += OnDisconnect;
                            c.WebcamChanged += (source) => _view.Status = source.EventArg;

                            c.Send("hello");
                        }
                        catch (SocketException ex)
                        {
                            _view.Status = ex.Message;
                        }
                    }
                });
        }

        private void OnDisconnect(Client source)
        {
            rwl.EnterWriteLock();
            if (_clients.Contains(source))
            {
                _clients.Remove(source);
                source.Dispose();
            }
            rwl.ExitWriteLock();

            _view.RemovedClient = source.IP;
            _view.Status = "Client " + source.IP + " has disconnected";
        }
    }
}
