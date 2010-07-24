using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using TCP;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.IO;

namespace WebcamRecorder
{
    public partial class RecorderForm : Form
    {
        public RecorderForm()
        {
            InitializeComponent();

            udpListener = new UdpListener(this);

            ThreadStart starter = new ThreadStart(Run);
            thread = new Thread(starter);
            thread.Start();

            msgListBox.Items.Add("Starting...");
        }

        // temporary use
        public ListBox MsgListBox
        {
            get { return msgListBox; }
        }

        private const int MAXOUTSTANDINGPACKETS = 3;

        /// <summary>
        /// The thread will run the job.
        /// The job is the Method Run() below
        /// </summary>
        protected Thread thread = null;
        private ManualResetEvent ConnectionReady;
        private volatile bool bShutDown;
        private volatile int iConnectionCount;
        private UdpListener udpListener;
        private Font fontOverlay;

        private void RecorderForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            // Set exit condition
            bShutDown = true;

            if(fontOverlay != null)
                fontOverlay.Dispose();

            // Need to get out of wait
            ConnectionReady.Set();
        }

        public void Run()
        {
            msgListBox.Items.Add("Preparing environment...");
            const int VIDEODEVICE = 0; // zero based index of video capture device to use
            const int FRAMERATE = 15;  // Depends on video device caps.  Generally 4-30.
            const int VIDEOWIDTH = 640; // Depends on video device caps
            const int VIDEOHEIGHT = 480; // Depends on video device caps
            const long JPEGQUALITY = 30; // 1-100 or 0 for default
            const int TCPLISTENPORT = 399;

            int fSize = 14;
            fontOverlay = new Font("Times New Roman", fSize, System.Drawing.FontStyle.Bold,
                System.Drawing.GraphicsUnit.Point);

            WebCamRecorder.Capture cam = null;
            TcpServer serv = null;
            ImageCodecInfo myImageCodecInfo;
            EncoderParameters myEncoderParameters;

            // Set up logging
            StreamWriter sw = File.AppendText(@"c:\WebCam.log");

            try
            {
                // Set up member vars
                ConnectionReady = new ManualResetEvent(false);
                bShutDown = false;

                // Set up tcp server
                iConnectionCount = 0;
                serv = new TcpServer(TCPLISTENPORT, TcpServer.GetAddresses()[0]);
                serv.Connected += new TcpConnected(Connected);
                serv.Disconnected += new TcpConnected(Disconnected);
                serv.DataReceived += new TcpReceive(Receive);
                serv.Send += new TcpSend(Send);

                myEncoderParameters = null;
                myImageCodecInfo = GetEncoderInfo("image/jpeg");

                if (JPEGQUALITY != 0)
                {
                    // If not using the default jpeg quality setting
                    EncoderParameter myEncoderParameter;
                    myEncoderParameter = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, JPEGQUALITY);
                    myEncoderParameters = new EncoderParameters(1);
                    myEncoderParameters.Param[0] = myEncoderParameter;
                }

                cam = new WebCamRecorder.Capture(VIDEODEVICE, FRAMERATE, VIDEOWIDTH, VIDEOHEIGHT);

                // Initialization succeeded.  Now, start serving up frames
                DoIt(cam, serv, sw, myImageCodecInfo, myEncoderParameters);
            }
            catch (Exception ex)
            {
                try
                {
                    MessageBox.Show(String.Format("{0}: Failed on startup {1}", DateTime.Now.ToString(), ex));
                }
                catch { }
            }
            finally
            {
                // Cleanup
                if (serv != null)
                {
                    serv.Dispose();
                }

                if (cam != null)
                {
                    cam.Dispose();
                }
                sw.Close();
            }
        }

        // Start serving up frames
        private void DoIt(WebCamRecorder.Capture cam, TcpServer serv, StreamWriter sw, ImageCodecInfo myImageCodecInfo, EncoderParameters myEncoderParameters)
        {
            MemoryStream m = new MemoryStream(20000);
            Bitmap image = null;
            IntPtr ip = IntPtr.Zero;

            msgListBox.Items.Add("Ready to serve!");

            do
            {
                // Wait til a client connects before we start the graph
                ConnectionReady.WaitOne();
                cam.Start();
                msgListBox.Items.Add("requested");
                // While not shutting down, and still at least one client
                while ((!bShutDown) && (serv.Connections > 0))
                {
                    try
                    {

                        // capture image
                        ip = cam.GetBitMap();
                        image = new Bitmap(cam.Width, cam.Height, cam.Stride, PixelFormat.Format24bppRgb, ip);
                        image.RotateFlip(RotateFlipType.RotateNoneFlipY);

                        Bitmap bitmapOverlay = new Bitmap(image.Width, image.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                        Graphics g = Graphics.FromImage(bitmapOverlay);
                        g.Clear(System.Drawing.Color.Transparent);
                        g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

                        // Prepare to put the specified string on the image
                        string text = DateTime.Now.ToString();
                        SizeF d = g.MeasureString(text, fontOverlay);

                        float sLeft = 10;
                        float sTop = 10;

                        g.FillRectangle(System.Drawing.Brushes.Black, 7, 7, 250, 30);
                        g.DrawString(text, fontOverlay, System.Drawing.Brushes.White,
                            sLeft, sTop, System.Drawing.StringFormat.GenericTypographic);
                        g.Dispose();

                        g = Graphics.FromImage(image);
                        g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

                        // draw the overlay bitmap over the video's bitmap
                        g.DrawImage(bitmapOverlay, 0, 0, bitmapOverlay.Width, bitmapOverlay.Height);

                        // dispose of the various objects
                        g.Dispose();

                        // save it to jpeg using quality options
                        m.Position = 10;
                        image.Save(m, myImageCodecInfo, myEncoderParameters);

                        // Send the length as a fixed length string
                        m.Position = 0;
                        m.Write(Encoding.ASCII.GetBytes((m.Length - 10).ToString("d8") + "\r\n"), 0, 10);

                        // send the jpeg image
                        serv.SendToAll(m);

                        // Empty the stream
                        m.SetLength(0);

                        // remove the image from memory
                        image.Dispose();
                        image = null;
                    }
                    catch (Exception ex)
                    {
                        try
                        {
                            sw.WriteLine(DateTime.Now.ToString());
                            sw.WriteLine(ex);
                        }
                        catch { }
                    }
                    finally
                    {
                        if (ip != IntPtr.Zero)
                        {
                            Marshal.FreeCoTaskMem(ip);
                            ip = IntPtr.Zero;
                        }
                    }
                }

                // Clients have all disconnected.  Pause, then sleep and wait for more
                cam.Pause();
                sw.WriteLine("Dropped frames: " + cam.m_Dropped.ToString());

            } while (!bShutDown);
        }

        class PacketCount : IDisposable
        {
            private int m_PacketCount;
            private int m_MaxPackets;

            public PacketCount(int i)
            {
                m_MaxPackets = i;
                m_PacketCount = 0;
            }

            public bool AddPacket()
            {
                bool b;

                lock (this)
                {
                    b = m_PacketCount < m_MaxPackets;
                    if (b)
                    {
                        m_PacketCount++;
                    }
                    else
                    {
                        throw new Exception("Max outstanding Packets reached");
                    }
                }

                return b;
            }

            public void RemovePacket()
            {
                lock (this)
                {
                    if (m_PacketCount > 0)
                    {
                        m_PacketCount--;
                    }
                    else
                    {
                        throw new Exception("Packet count is messed up");
                    }
                }
            }

            public int Count()
            {
                return m_PacketCount;
            }

            #region IDisposable Members

            public void Dispose()
            {
#if DEBUG
                if (m_PacketCount != 0)
                {
                    throw new Exception("Packets left over: " + m_PacketCount.ToString());
                }
#endif
            }

            #endregion
        }

        // A client attached to the tcp port
        private void Connected(object sender, ref object t)
        {
            lock (this)
            {
                t = new PacketCount(MAXOUTSTANDINGPACKETS);
                iConnectionCount++;

                if (iConnectionCount == 1)
                {
                    msgListBox.Items.Add("A client has just connected");
                    ConnectionReady.Set();
                }
            }
        }

        // A client detached from the tcp port
        private void Disconnected(object sender, ref object t)
        {
            lock (this)
            {
                iConnectionCount--;
                if (iConnectionCount == 0)
                {
                    msgListBox.Items.Add("A client has just disconnected");
                    ConnectionReady.Reset();
                }
            }
        }

        private void Receive(Object sender, ref object o, ref byte[] b, int ByteCount)
        {
            PacketCount pc = (PacketCount)o;
            pc.RemovePacket();
        }

        private void Send(Object sender, ref object o, ref bool b)
        {
            PacketCount pc = (PacketCount)o;

            b = pc.AddPacket();
        }

        // Find the appropriate encoder
        private ImageCodecInfo GetEncoderInfo(String mimeType)
        {
            int j;
            ImageCodecInfo[] encoders = ImageCodecInfo.GetImageEncoders();
            for (j = 0; j < encoders.Length; ++j)
            {
                if (encoders[j].MimeType == mimeType)
                    return encoders[j];
            }
            return null;
        }
    }
}
