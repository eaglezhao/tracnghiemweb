using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Collections;
using System.IO;
using System.Net;
using System.Threading;

namespace WpfWebcamServer
{
    class ConnectionManager : IDisposable
    {
        private TcpListener webcamListener;
        private TcpListener clientListener;
        private WebcamManager webcamManager;
        private ClientManager clientManager;
        private MainWindow mainWindow;
        private Dictionary<Client, Webcam> relations;

        public ConnectionManager(MainWindow mainWindow, int broadcastPort, int camPort, int clientPort, string basePath)
        {
            this.mainWindow = mainWindow;
            this.BasePath = basePath;
            this.BroadcastPort = broadcastPort;
            webcamManager = new WebcamManager();
            clientManager = new ClientManager();
            relations = new Dictionary<Client, Webcam>();

            webcamListener = new TcpListener(IPAddress.Any, camPort);
            webcamListener.Start();

            ThreadStart wcListener = new ThreadStart(ListenForWebcam);
            Thread wcThread = new Thread(wcListener);
            wcThread.Start();

            clientListener = new TcpListener(IPAddress.Any, clientPort);
            clientListener.Start();

            ThreadStart clListener = new ThreadStart(ListenForClient);
            Thread clThread = new Thread(clListener);
            clThread.Start();

            ////////////////////////////////////////////////////////////////////////////////////
            AutoDetect();
        }

        public string BasePath { get; set; }
        public int BroadcastPort { get; set; }

        public void Dispose()
        {
            lock (this)
            {
                try
                {
                    foreach (Webcam w in webcamManager.GetWebcams())
                        w.Dispose();
                    foreach (Client c in clientManager.GetClients())
                        c.Dispose();

                    if (webcamListener != null)
                        webcamListener.Stop();
                    if (webcamListener != null)
                        clientListener.Stop();

                    webcamListener = null;
                    clientListener = null;
                }
                catch { }
            }
        }

        ~ConnectionManager()
        {
            Dispose();
        }

        public void AutoDetect()
        {
            try
            {
                byte[] msg = Encoding.ASCII.GetBytes("dis " + ((IPEndPoint)webcamListener.LocalEndpoint).Port);
                UdpClient udpClient = new UdpClient(IPAddress.Broadcast.ToString(), BroadcastPort);
                udpClient.EnableBroadcast = true;
                udpClient.Send(msg, msg.Length);
            }
            catch (SocketException)
            {
            }
        }

        public void ViewCamera(string name)
        {
            Webcam wc = webcamManager.FindWebcam(name);
            if (wc != null)
                wc.AddClient(mainWindow.OnFrameReceived);
        }

        public void StopViewing(string name)
        {
            Webcam wc = webcamManager.FindWebcam(name);
            if (wc != null)
                wc.RemoveClient(mainWindow.OnFrameReceived);
        }

        private void ListenForWebcam()
        {
            try
            {
                do
                {
                    TcpClient client = webcamListener.AcceptTcpClient();
                    Webcam w = new Webcam(client, BasePath);
                    webcamManager.AddWebcam(w);
                    w.Connected += new ConnectionHandler(OnDeviceDetected);
                    w.Disconnected += new ConnectionHandler((sender, msg) => webcamManager.RemoveWebcam((Webcam)sender));
                }
                while (true);
            }
            catch (SocketException)
            {
            }
        }

        private void ListenForClient()
        {
            try
            {
                do
                {
                    TcpClient client = clientListener.AcceptTcpClient();
                    Client c = new Client(client);
                    clientManager.AddClient(c);

                    if (ClientDetected != null)
                        ClientDetected(c, ((IPEndPoint)c.TcpClient.Client.RemoteEndPoint).Address.ToString());

                    c.Disconnected += new ConnectionHandler(OnClientDisconnect);
                    c.RequestReceived += new ConnectionHandler(ProcessClientRequest);
                    c.Send(null, Encoding.ASCII.GetBytes("hello"), MessageType.Command);
                }
                while (true);
            }
            catch (SocketException)
            {
            }
        }

        private void ProcessClientRequest(object sender, string msg)
        {
            string[] message = msg.Split(' ');
            Client c = (Client)sender;

            if (message[0] == "start")
            {
                Webcam wc = webcamManager.FindWebcam(message[1]);
                if (wc != null)
                {
                    wc.AddClient(c.Send);
                    lock (this)
                    {
                        relations.Add(c, wc);
                    }
                }
            }
            else if (message[0] == "stop")
            {
                Webcam wc = webcamManager.FindWebcam(message[1]);
                if (wc != null)
                {
                    wc.RemoveClient(c.Send);
                    lock (this)
                    {
                        relations.Remove(c);
                    }
                }
            }
            else if (message[0] == "get-list")
            {
                IEnumerable<string> webcamList = webcamManager.GetWebcams().Select((wc) => wc.Name);
                string wcList = "webcam-list";

                foreach (string s in webcamList)
                    wcList += " " + s;
                
                c.Send(this, Encoding.ASCII.GetBytes(wcList), MessageType.Command);
            }
        }

        private void OnDeviceDetected(object sender, string message)
        {
            if (WebcamDetected != null)
                WebcamDetected(sender, message);
        }

        /*private void OnWebcamDisconnect(object sender, string message)
        {
            Webcam w = (Webcam)sender;
            webcamL
            webcamManager.RemoveWebcam(w);
        }*/

        private void OnClientDisconnect(object sender, string message)
        {
            Client c = (Client) sender;
            Webcam w = relations[c];

            if (w != null)
            {
                lock (this)
                {
                    w.RemoveClient(c.Send);
                }
                clientManager.RemoveClient(c);
                relations.Remove(c);
                c.Dispose();
            }
        }

        public event ConnectionHandler ClientDetected;
        public event ConnectionHandler WebcamDetected;
    }
}
