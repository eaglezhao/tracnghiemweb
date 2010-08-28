using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WpfWebcamServer.Clients
{
    internal class ClientManager
    {
        private List<Client> clients = new List<Client>();

        public void AddClient(Client client)
        {
            lock (this)
            {
                clients.Add(client);
            }
        }

        public bool RemoveClient(Client client)
        {
            lock (this)
            {
                if (clients.Contains(client))
                {
                    clients.Remove(client);
                    return true;
                }
                else
                    return false;
            }
        }

        public List<Client> GetClients()
        {
            return clients;
        }

        public Client GetClientAt(int index)
        {
            return clients[index];
        }

        public Client FindClient(System.Net.Sockets.TcpClient tcpClient)
        {
            lock (this)
            {
                foreach (Client c in clients)
                    if (c.TcpClient == tcpClient)
                        return c;

                return null;
            }
        }
    }
}
