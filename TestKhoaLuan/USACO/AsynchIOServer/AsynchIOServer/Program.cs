using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Threading;
using System.Net;
using System.IO;
using System.Windows.Forms;


namespace AsynchIOServer
{
    //class Program
    //{
    //    public static void Main()
    //    {
    //        TcpListener tcpListener = new TcpListener(10);
    //        tcpListener.Start();
            
    //        Socket socketForClient = tcpListener.AcceptSocket();//Accept();
            
    //        if (socketForClient.Connected)
    //        {
    //            Console.WriteLine("Client connected");
    //            NetworkStream networkStream = new NetworkStream(socketForClient);

    //            System.IO.StreamWriter streamWriter = new System.IO.StreamWriter(networkStream);
    //            System.IO.StreamReader streamReader = new System.IO.StreamReader(networkStream);

    //            string theString = "Sending";
    //            streamWriter.WriteLine(theString);
    //            Console.WriteLine(theString);
    //            streamWriter.Flush();
    //            theString = streamReader.ReadLine();
    //            Console.WriteLine(theString);
    //            streamReader.Close();
    //            networkStream.Close();
    //            streamWriter.Close();
    //        }
    //        socketForClient.Close();
    //        Console.WriteLine("Exiting...");
    //    }
    //}
    ////public class StateObject
    ////{
    ////    // Client socket.
    ////    public Socket workSocket = null;
    ////    // Size of receive buffer.
    ////    public const int BufferSize = 1024;
    ////    // Receive buffer.
    ////    public byte[] buffer = new byte[BufferSize];
    ////    // Received data string.
    ////    public StringBuilder sb = new StringBuilder();
    ////}

    ////public class AsynchronousSocketListener
    ////{
    ////    // Thread signal.
    ////    public static ManualResetEvent allDone = new ManualResetEvent(false);

    ////    public AsynchronousSocketListener()
    ////    {
    ////    }

    ////    public static void StartListening()
    ////    {
    ////        // Temp storage for incoming data.
    ////        byte[] recvDataBytes = new Byte[1024];

    ////        // Make endpoint for the socket.
    ////        //IPAddress serverAdd = Dns.Resolve("localhost"); - That line was wrong 
    ////        //'baaelSiljan' has noticed it and then I've modified that line, correct line will be as:
    ////        IPHostEntry ipHost = Dns.Resolve("localhost");
    ////        IPAddress serverAdd = ipHost.AddressList[0];

    ////        IPEndPoint ep = new IPEndPoint(serverAdd, 5656);

    ////        // Create a TCP/IP socket for listner.
    ////        Socket listenerSock = new Socket(AddressFamily.InterNetwork,
    ////        SocketType.Stream, ProtocolType.Tcp);

    ////        // Bind the socket to the endpoint and wait for listen for incoming connections.
    ////        try
    ////        {
    ////            listenerSock.Bind(ep);
    ////            listenerSock.Listen(10);

    ////            while (true)
    ////            {
    ////                // Set the event to nonsignaled state.
    ////                allDone.Reset();

    ////                // Start an asynchronous socket to listen for connections.
    ////                Console.WriteLine("Waiting for Client...");
    ////                listenerSock.BeginAccept(
    ////                new AsyncCallback(AcceptCallback),
    ////                listenerSock);

    ////                // Wait until a connection is made before continuing.
    ////                allDone.WaitOne();
    ////            }

    ////        }
    ////        catch (Exception e)
    ////        {
    ////            Console.WriteLine(e.ToString());
    ////        }

    ////        Console.WriteLine("\nPress ENTER to continue...");
    ////        Console.Read();

    ////    }

    ////    public static void AcceptCallback(IAsyncResult ar)
    ////    {
    ////        // Signal the main thread to continue.
    ////        allDone.Set();

    ////        // Get the socket that handles the client request.
    ////        Socket listener = (Socket)ar.AsyncState;
    ////        Socket handler = listener.EndAccept(ar);

    ////        // Create the state object.
    ////        StateObject state = new StateObject();
    ////        state.workSocket = handler;
    ////        handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
    ////        new AsyncCallback(ReadCallback), state);
    ////    }

    ////    public static void ReadCallback(IAsyncResult ar)
    ////    {
    ////        String content = String.Empty;

    ////        // Retrieve the state object and the handler socket
    ////        // from the asynchronous state object.
    ////        StateObject state = (StateObject)ar.AsyncState;
    ////        Socket handler = state.workSocket;

    ////        // Read data from the client socket.
    ////        int bytesRead = handler.EndReceive(ar);

    ////        if (bytesRead > 0)
    ////        {
    ////            // There might be more data, so store the data received so far.
    ////            state.sb.Append(Encoding.ASCII.GetString(
    ////            state.buffer, 0, bytesRead));

    ////            // Check for end-of-file tag. If it is not there, read
    ////            // more data.
    ////            content = state.sb.ToString();
    ////            if (content.IndexOf("") > -1)
    ////            {
    ////                // All the data has been read from the
    ////                // client. Display it on the console.
    ////                Console.WriteLine("Read {0} bytes from socket. \n Data : {1}",
    ////                content.Length, content);
    ////                // Echo the data back to the client.
    ////                Send(handler, content);
    ////            }
    ////            else
    ////            {
    ////                // Not all data received. Get more.
    ////                handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
    ////                new AsyncCallback(ReadCallback), state);
    ////            }
    ////        }
    ////    }

    ////    private static void Send(Socket handler, String data)
    ////    {
    ////        // Convert the string data to byte data using ASCII encoding.
    ////        byte[] byteData = Encoding.ASCII.GetBytes(data);

    ////        // Begin sending the data to the remote device.
    ////        handler.BeginSend(byteData, 0, byteData.Length, 0,
    ////        new AsyncCallback(SendCallback), handler);
    ////    }

    ////    private static void SendCallback(IAsyncResult ar)
    ////    {
    ////        try
    ////        {
    ////            // Retrieve the socket from the state object.
    ////            Socket handler = (Socket)ar.AsyncState;

    ////            // Complete sending the data to the remote device.
    ////            int bytesSent = handler.EndSend(ar);
    ////            Console.WriteLine("Sent {0} bytes to client.", bytesSent);

    ////            handler.Shutdown(SocketShutdown.Both);
    ////            handler.Close();

    ////        }
    ////        catch (Exception e)
    ////        {
    ////            Console.WriteLine(e.ToString());
    ////        }
    ////    }


    ////    public static int Main(String[] args)
    ////    {
    ////        StartListening();
    ////        return 0;
    ////    }
    ////}

    /// <summary>
    /// Description of SocketServer.	
    /// </summary>
    public class SocketServer : System.Windows.Forms.Form
    {
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.RichTextBox richTextBoxReceivedMsg;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox textBoxMsg;
        private System.Windows.Forms.Button buttonStopListen;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.RichTextBox richTextBoxSendMsg;
        private System.Windows.Forms.TextBox textBoxIP;
        private System.Windows.Forms.Button buttonStartListen;
        private System.Windows.Forms.Button buttonSendMsg;
        private System.Windows.Forms.Button buttonClose;


        const int MAX_CLIENTS = 10;

        public AsyncCallback pfnWorkerCallBack;
        private Socket m_mainSocket;
        private Socket[] m_workerSocket = new Socket[10];
        private int m_clientCount = 0;

        public SocketServer()
        {
            //
            // The InitializeComponent() call is required for Windows Forms designer support.
            //
            InitializeComponent();

            // Display the local IP address on the GUI
            textBoxIP.Text = GetIP();
        }

        [STAThread]
        public static void Main(string[] args)
        {
            Application.Run(new SocketServer());
        }

        #region Windows Forms Designer generated code
        /// <summary>
        /// This method is required for Windows Forms designer support.
        /// Do not change the method contents inside the source code editor. The Forms designer might
        /// not be able to load this method if it was changed manually.
        /// </summary>
        private void InitializeComponent()
        {
            this.buttonClose = new System.Windows.Forms.Button();
            this.buttonSendMsg = new System.Windows.Forms.Button();
            this.buttonStartListen = new System.Windows.Forms.Button();
            this.textBoxIP = new System.Windows.Forms.TextBox();
            this.richTextBoxSendMsg = new System.Windows.Forms.RichTextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.buttonStopListen = new System.Windows.Forms.Button();
            this.textBoxMsg = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.richTextBoxReceivedMsg = new System.Windows.Forms.RichTextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // buttonClose
            // 
            this.buttonClose.Location = new System.Drawing.Point(321, 232);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(88, 24);
            this.buttonClose.TabIndex = 11;
            this.buttonClose.Text = "Close";
            this.buttonClose.Click += new System.EventHandler(this.ButtonCloseClick);
            // 
            // buttonSendMsg
            // 
            this.buttonSendMsg.Location = new System.Drawing.Point(16, 192);
            this.buttonSendMsg.Name = "buttonSendMsg";
            this.buttonSendMsg.Size = new System.Drawing.Size(192, 24);
            this.buttonSendMsg.TabIndex = 7;
            this.buttonSendMsg.Text = "Send Message";
            this.buttonSendMsg.Click += new System.EventHandler(this.ButtonSendMsgClick);
            // 
            // buttonStartListen
            // 
            this.buttonStartListen.BackColor = System.Drawing.Color.Blue;
            this.buttonStartListen.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.buttonStartListen.ForeColor = System.Drawing.Color.Yellow;
            this.buttonStartListen.Location = new System.Drawing.Point(227, 16);
            this.buttonStartListen.Name = "buttonStartListen";
            this.buttonStartListen.Size = new System.Drawing.Size(88, 40);
            this.buttonStartListen.TabIndex = 4;
            this.buttonStartListen.Text = "Start Listening";
            this.buttonStartListen.Click += new System.EventHandler(this.ButtonStartListenClick);
            // 
            // textBoxIP
            // 
            this.textBoxIP.Location = new System.Drawing.Point(88, 16);
            this.textBoxIP.Name = "textBoxIP";
            this.textBoxIP.ReadOnly = true;
            this.textBoxIP.Size = new System.Drawing.Size(120, 20);
            this.textBoxIP.TabIndex = 12;
            this.textBoxIP.Text = "";
            // 
            // richTextBoxSendMsg
            // 
            this.richTextBoxSendMsg.Location = new System.Drawing.Point(16, 87);
            this.richTextBoxSendMsg.Name = "richTextBoxSendMsg";
            this.richTextBoxSendMsg.Size = new System.Drawing.Size(192, 104);
            this.richTextBoxSendMsg.TabIndex = 6;
            this.richTextBoxSendMsg.Text = "";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 40);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(48, 16);
            this.label1.TabIndex = 1;
            this.label1.Text = "Port";
            // 
            // buttonStopListen
            // 
            this.buttonStopListen.BackColor = System.Drawing.Color.Red;
            this.buttonStopListen.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.buttonStopListen.ForeColor = System.Drawing.Color.Yellow;
            this.buttonStopListen.Location = new System.Drawing.Point(321, 16);
            this.buttonStopListen.Name = "buttonStopListen";
            this.buttonStopListen.Size = new System.Drawing.Size(88, 40);
            this.buttonStopListen.TabIndex = 5;
            this.buttonStopListen.Text = "Stop Listening";
            this.buttonStopListen.Click += new System.EventHandler(this.ButtonStopListenClick);
            // 
            // textBoxMsg
            // 
            this.textBoxMsg.BackColor = System.Drawing.SystemColors.Control;
            this.textBoxMsg.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBoxMsg.ForeColor = System.Drawing.SystemColors.HotTrack;
            this.textBoxMsg.Location = new System.Drawing.Point(120, 240);
            this.textBoxMsg.Name = "textBoxMsg";
            this.textBoxMsg.ReadOnly = true;
            this.textBoxMsg.Size = new System.Drawing.Size(192, 13);
            this.textBoxMsg.TabIndex = 14;
            this.textBoxMsg.Text = "None";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(16, 71);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(192, 16);
            this.label4.TabIndex = 8;
            this.label4.Text = "Broadcast Message To Clients";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(217, 71);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(192, 16);
            this.label5.TabIndex = 10;
            this.label5.Text = "Message Received From Clients";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(88, 40);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(40, 20);
            this.textBoxPort.TabIndex = 0;
            this.textBoxPort.Text = "8000";
            // 
            // richTextBoxReceivedMsg
            // 
            this.richTextBoxReceivedMsg.BackColor = System.Drawing.SystemColors.InactiveCaptionText;
            this.richTextBoxReceivedMsg.Location = new System.Drawing.Point(217, 87);
            this.richTextBoxReceivedMsg.Name = "richTextBoxReceivedMsg";
            this.richTextBoxReceivedMsg.ReadOnly = true;
            this.richTextBoxReceivedMsg.Size = new System.Drawing.Size(192, 129);
            this.richTextBoxReceivedMsg.TabIndex = 9;
            this.richTextBoxReceivedMsg.Text = "";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(16, 16);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(56, 16);
            this.label2.TabIndex = 2;
            this.label2.Text = "Server IP";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(0, 240);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(112, 16);
            this.label3.TabIndex = 13;
            this.label3.Text = "Status Message:";
            // 
            // SocketServer
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(424, 260);
            this.Controls.Add(this.textBoxMsg);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.textBoxIP);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.richTextBoxReceivedMsg);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.buttonSendMsg);
            this.Controls.Add(this.richTextBoxSendMsg);
            this.Controls.Add(this.buttonStopListen);
            this.Controls.Add(this.buttonStartListen);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBoxPort);
            this.Name = "SocketServer";
            this.Text = "SocketServer";
            this.ResumeLayout(false);
        }
        #endregion
        void ButtonStartListenClick(object sender, System.EventArgs e)
        {
            try
            {
                // Check the port value
                if (textBoxPort.Text == "")
                {
                    MessageBox.Show("Please enter a Port Number");
                    return;
                }
                string portStr = textBoxPort.Text;
                int port = System.Convert.ToInt32(portStr);
                // Create the listening socket...
                m_mainSocket = new Socket(AddressFamily.InterNetwork,
                                          SocketType.Stream,
                                          ProtocolType.Tcp);
                IPEndPoint ipLocal = new IPEndPoint(IPAddress.Any, port);
                // Bind to local IP Address...
                m_mainSocket.Bind(ipLocal);
                // Start listening...
                m_mainSocket.Listen(4);
                // Create the call back for any client connections...
                m_mainSocket.BeginAccept(new AsyncCallback(OnClientConnect), null);

                UpdateControls(true); Socket e;e.sendfile

            }
            catch (SocketException se)
            {
                MessageBox.Show(se.Message);
            }

        }
        private void UpdateControls(bool listening)
        {
            buttonStartListen.Enabled = !listening;
            buttonStopListen.Enabled = listening;
        }
        // This is the call back function, which will be invoked when a client is connected
        public void OnClientConnect(IAsyncResult asyn)
        {
            try
            {
                // Here we complete/end the BeginAccept() asynchronous call
                // by calling EndAccept() - which returns the reference to
                // a new Socket object
                m_workerSocket[m_clientCount] = m_mainSocket.EndAccept(asyn);
                // Let the worker Socket do the further processing for the 
                // just connected client
                WaitForData(m_workerSocket[m_clientCount]);
                // Now increment the client count
                ++m_clientCount;
                // Display this client connection as a status message on the GUI	
                String str = String.Format("Client # {0} connected", m_clientCount);
                textBoxMsg.Text = str;

                // Since the main Socket is now free, it can go back and wait for
                // other clients who are attempting to connect
                m_mainSocket.BeginAccept(new AsyncCallback(OnClientConnect), null);
            }
            catch (ObjectDisposedException)
            {
                System.Diagnostics.Debugger.Log(0, "1", "\n OnClientConnection: Socket has been closed\n");
            }
            catch (SocketException se)
            {
                MessageBox.Show(se.Message);
            }

        }
        public class SocketPacket
        {
            public System.Net.Sockets.Socket m_currentSocket;
            public byte[] dataBuffer = new byte[1];
        }
        // Start waiting for data from the client
        public void WaitForData(System.Net.Sockets.Socket soc)
        {
            try
            {
                if (pfnWorkerCallBack == null)
                {
                    // Specify the call back function which is to be 
                    // invoked when there is any write activity by the 
                    // connected client
                    pfnWorkerCallBack = new AsyncCallback(OnDataReceived);
                }
                SocketPacket theSocPkt = new SocketPacket();
                theSocPkt.m_currentSocket = soc;
                // Start receiving any data written by the connected client
                // asynchronously
                soc.BeginReceive(theSocPkt.dataBuffer, 0,
                                   theSocPkt.dataBuffer.Length,
                                   SocketFlags.None,
                                   pfnWorkerCallBack,
                                   theSocPkt);
            }
            catch (SocketException se)
            {
                MessageBox.Show(se.Message);
            }

        }
        // This the call back function which will be invoked when the socket
        // detects any client writing of data on the stream
        public void OnDataReceived(IAsyncResult asyn)
        {
            try
            {
                SocketPacket socketData = (SocketPacket)asyn.AsyncState;

                int iRx = 0;
                // Complete the BeginReceive() asynchronous call by EndReceive() method
                // which will return the number of characters written to the stream 
                // by the client
                iRx = socketData.m_currentSocket.EndReceive(asyn);
                char[] chars = new char[iRx + 1];
                System.Text.Decoder d = System.Text.Encoding.UTF8.GetDecoder();
                int charLen = d.GetChars(socketData.dataBuffer,
                                         0, iRx, chars, 0);
                System.String szData = new System.String(chars);
                richTextBoxReceivedMsg.AppendText(szData);

                // Continue the waiting for data on the Socket
                WaitForData(socketData.m_currentSocket);
            }
            catch (ObjectDisposedException)
            {
                System.Diagnostics.Debugger.Log(0, "1", "\nOnDataReceived: Socket has been closed\n");
            }
            catch (SocketException se)
            {
                MessageBox.Show(se.Message);
            }
        }
        void ButtonSendMsgClick(object sender, System.EventArgs e)
        {
            try
            {
                Object objData = richTextBoxSendMsg.Text;
                byte[] byData = System.Text.Encoding.ASCII.GetBytes(objData.ToString());
                for (int i = 0; i < m_clientCount; i++)
                {
                    if (m_workerSocket[i] != null)
                    {
                        if (m_workerSocket[i].Connected)
                        {
                            m_workerSocket[i].Send(byData);
                        }
                    }
                }

            }
            catch (SocketException se)
            {
                MessageBox.Show(se.Message);
            }
        }

        void ButtonStopListenClick(object sender, System.EventArgs e)
        {
            CloseSockets();
            UpdateControls(false);
        }

        String GetIP()
        {
            String strHostName = Dns.GetHostName();

            // Find host by name
            IPHostEntry iphostentry = Dns.GetHostByName(strHostName);

            // Grab the first IP addresses
            String IPStr = "";
            foreach (IPAddress ipaddress in iphostentry.AddressList)
            {
                IPStr = ipaddress.ToString();
                return IPStr;
            }
            return IPStr;
        }
        void ButtonCloseClick(object sender, System.EventArgs e)
        {
            CloseSockets();
            Close();
        }
        void CloseSockets()
        {
            if (m_mainSocket != null)
            {
                m_mainSocket.Close();
            }
            for (int i = 0; i < m_clientCount; i++)
            {
                if (m_workerSocket[i] != null)
                {
                    m_workerSocket[i].Close();
                    m_workerSocket[i] = null;
                }
            }
        }
    }

}
