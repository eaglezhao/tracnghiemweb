using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Collections;
using System.IO;
using System.Net;
using System.Threading;
using WpfWebcamServer.Webcams;
using WpfWebcamServer.Clients;

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
        }

        public string BasePath { get; set; }
        public int BroadcastPort { get; set; }

        public void Dispose()
        {
            lock (this)
            {
                for (int i = 0; i < webcamManager.GetWebcams().Count; i++)
                    webcamManager.GetWebcamAt(i).Dispose();
                for (int i = 0; i < clientManager.GetClients().Count; i++)
                    clientManager.GetClientAt(i).Dispose();

                if (webcamListener != null)
                    webcamListener.Stop();
                if (webcamListener != null)
                    clientListener.Stop();

                webcamListener = null;
                clientListener = null;
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
                udpClient.Close();
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
                    w.Connected += new ConnectionHandler(OnWebcamDetected);
                    w.Disconnected += new ConnectionHandler(OnWebcamDisconnect);
                    w.MotionAlarmed += new ConnectionHandler(OnMotionAlarm);
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

                    c.Disconnected += new ConnectionHandler(OnClientDisconnect);
                    c.RequestReceived += new ConnectionHandler(ProcessClientRequest);
                    c.Send(null, Encoding.ASCII.GetBytes("hello"), MessageType.Command);

                    OnClientConnect(c, "connect " + c.IP);
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
                Webcam wc = null;

                lock (this)
                {
                    if (relations.ContainsKey(c))
                        wc = relations[c];
                    if (wc != null)
                    {
                        wc.RemoveClient(c.Send);
                        relations.Remove(c);
                    }

                    wc = webcamManager.FindWebcam(message[1]);
                    if (wc != null)
                    {
                        wc.AddClient(c.Send);
                        relations.Add(c, wc);
                        if (ClientEventRaised != null)
                            ClientEventRaised(c, string.Format("start {0} {1}",
                                c.IP, wc.Name));
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
                    if (ClientEventRaised != null)
                        ClientEventRaised(c, string.Format("stop {0} {1}",
                            c.IP, wc.Name));
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

        private void OnWebcamDetected(object sender, string message)
        {
            if (WebcamEventRaised != null)
                WebcamEventRaised(sender, "connect " + message);
        }

        private void OnWebcamDisconnect(object sender, string message)
        {
            Webcam w = (Webcam)sender;
            Client[] clients = relations.Keys.Where(k => relations[k] == w).ToArray();

            lock (this)
            {
                foreach (Client c in clients)
                    relations.Remove(c);
            }

            if (WebcamEventRaised != null)
                WebcamEventRaised(sender, "disconnect " + w.Name);

            webcamManager.RemoveWebcam(w);
            w.Dispose();
        }

        private void OnMotionAlarm(object sender, string message)
        {
            Webcam w = (Webcam)sender;

            if (message == "start")
            {
                if (WebcamEventRaised != null && w.MotionDetected == false)
                {
                    WebcamEventRaised(sender, "motion " + w.Name);
                    w.MotionDetected = true;
                }

                if (mainWindow.AllowRecord)
                    w.StartRecording();
                if (mainWindow.AllowSoundAlarm)
                    w.Alarm();
            }
            else if(message == "recording")
                WebcamEventRaised(sender, "record " + w.Name);
            else if (message == "stop")
            {
                w.StopRecording();
                w.MotionDetected = false;
                WebcamEventRaised(sender, "stop-record " + w.Name);
            }
        }

        private void OnClientConnect(object sender, string message)
        {
            if (ClientEventRaised != null)
                ClientEventRaised(sender, message);
        }

        private void OnClientDisconnect(object sender, string message)
        {
            Client c = (Client)sender;
            Webcam w = null;

            lock (this)
            {
                if (relations.ContainsKey(c))
                {
                    w = relations[c];
                    w.RemoveClient(c.Send);
                    relations.Remove(c);
                }
            }
            
            if (ClientEventRaised != null)
                ClientEventRaised(c, "disconnect " + c.IP);

            clientManager.RemoveClient(c);
            c.Dispose();
        }

        public event ConnectionHandler ClientEventRaised;
        public event ConnectionHandler WebcamEventRaised;
    }

    public delegate void ConnectionHandler(Object sender, string message);
}
