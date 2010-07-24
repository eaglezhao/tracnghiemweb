using System;
using System.Net;
using System.Net.Sockets;

namespace WebcamServer
{
    // Wrapper for each client (stored in m_aryClients)
    internal class SockWrapper
    {
        // The buffer is used by receive
        public Socket Client;
        public byte[] byBuff;
        public object obj;

        public SockWrapper(Socket client)
        {
            Client = client;
            byBuff = new byte[256];
            obj = new object();
        }
    }
}
