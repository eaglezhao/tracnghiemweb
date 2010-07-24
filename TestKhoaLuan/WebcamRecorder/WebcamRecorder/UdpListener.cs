using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace WebcamRecorder
{
    class UdpListener
    {
        private string hostAddr;
        private string remoteAddr;
        private UdpClient udpClient = new UdpClient(1111);
        private RecorderForm f;

        public UdpListener(RecorderForm f)
        {
            // listBox.Items.Add("Client starting... host address is " + Dns.GetHostAddresses(Dns.GetHostName())[0].ToString());
            this.f = f;
            udpClient.BeginReceive(new AsyncCallback(OnReceive), udpClient);
        }

        private void OnReceive(IAsyncResult ar)
        {
            IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 1111);
            string msg = Encoding.ASCII.GetString(udpClient.EndReceive(ar, ref remoteEP));

            f.MsgListBox.Items.Add("here");

            if (msg.StartsWith("helu"))
            {
                remoteAddr = msg.Substring(5);

                f.MsgListBox.Items.Add("Server " + remoteAddr + " is requesting info");
                IPAddress[] addresses = Dns.GetHostAddresses(Dns.GetHostName());

                foreach (IPAddress address in addresses)
                {
                    string addr = address.ToString();
                    while (addr.Contains("."))
                    {
                        addr = addr.Substring(0, addr.LastIndexOf("."));
                        if (remoteAddr.Contains(addr))
                            hostAddr = address.ToString();
                        // listBox.Items.Add("addr: " + addr + " --- remoteAddr: " + remoteAddr);
                    }
                }

                byte[] messageToSend = Encoding.ASCII.GetBytes("cam " + hostAddr);
                // listBox.Items.Add("hostAddr: " + hostAddr + "message: " + Encoding.ASCII.GetString(messageToSend));
                udpClient.Send(messageToSend, messageToSend.Length, remoteAddr, 1112);
                // listBox.Items.Add("Sending info to server " + remoteAddr);
            }
            //udpClient.BeginReceive(new AsyncCallback(OnReceive), udpClient);
        }
    }
}
