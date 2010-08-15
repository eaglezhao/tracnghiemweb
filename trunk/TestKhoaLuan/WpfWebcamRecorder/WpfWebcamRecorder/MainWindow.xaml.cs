using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Drawing;
using Imaging;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;

namespace WpfWebcamRecorder
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        protected Thread thread;
        private Webcam cam;
        private ConnectionManager conManager;
        private EncoderParameters myEncoderParameters;
        private ImageCodecInfo myImageCodecInfo;
        private StreamWriter sw;
        private volatile bool stopCondition;
        private volatile bool requested;
        private double alarmLevel;

        public MainWindow()
        {
            InitializeComponent();
            msgListBox.Items.Add("Starting...");

            portTextBox.Text = Properties.Settings.Default.Port;
            alarmLevel = sensitivitySlider.Value;
            requested = false;
            string[] webcamList = Webcam.GetWebcamList();

            if (webcamList.Length > 0)
            {
                foreach (string s in Webcam.GetWebcamList())
                    deviceComboBox.Items.Add(s);
                deviceComboBox.SelectedIndex = 0;

                msgListBox.Items.Add("Configuring environment...");

                try
                {
                    // Set up logging
                    sw = File.AppendText(Environment.GetFolderPath(Environment.SpecialFolder.Personal) + "\\WebCam.log");

                    ConfigureImageQuality();
                    ConfigureDevice();
                    ConfigureConnection();

                    msgListBox.Items.Add("Ready!");
                }
                catch (Exception ex)
                {
                    MessageBox.Show(String.Format("{0}: Failed on startup {1}", DateTime.Now.ToString(), ex));
                }
            }
        }

        private void ConfigureImageQuality()
        {
            const long JPEGQUALITY = 30; // 1-100 or 0 for default

            myImageCodecInfo = GetEncoderInfo("image/jpeg");

            if (JPEGQUALITY != 0)
            {
                // If not using the default jpeg quality setting
                EncoderParameter myEncoderParameter = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, JPEGQUALITY);
                myEncoderParameters = new EncoderParameters(1);
                myEncoderParameters.Param[0] = myEncoderParameter;
            }
        }

        private void ConfigureDevice()
        {
            const int FRAMERATE = 15;  // Depends on video device caps.  Generally 4-30.
            const int VIDEOWIDTH = 640; // Depends on video device caps
            const int VIDEOHEIGHT = 480; // Depends on video device caps

            int index = (int)deviceComboBox.Dispatcher.Invoke(new Func<int>(() => deviceComboBox.SelectedIndex));
            cam = new Webcam(index, FRAMERATE, VIDEOWIDTH, VIDEOHEIGHT);
        }

        private void ConfigureConnection()
        {
            // Set up connection manager
            int portNumber = (int)portTextBox.Dispatcher.Invoke(new Func<int>(() => Convert.ToInt32(portTextBox.Text)));
            conManager = new ConnectionManager(portNumber);
            conManager.MessageReceived += new MessageHandler(ProcessMessage);
            conManager.Connected += new ConnectionHandler(OnConnect);
            conManager.Disconnected += new ConnectionHandler(OnDisconnect);
        }

        // Start serving up frames
        private void StartWebcam(Webcam cam, ConnectionManager conManager, StreamWriter sw, ImageCodecInfo myImageCodecInfo, EncoderParameters myEncoderParameters)
        {
            MemoryStream m = new MemoryStream(20000);
            Bitmap image = null;
            IntPtr ip = IntPtr.Zero;
            Font fontOverlay = new Font("Times New Roman", 14, System.Drawing.FontStyle.Bold,
                System.Drawing.GraphicsUnit.Point);
            MotionDetector motionDetector = new MotionDetector();
            motionDetector.MotionLevelCalculation = true;
            stopCondition = false;
            bool isRecording = false;
            DateTime lastMotion = DateTime.Now;

            cam.Start();

            while (!stopCondition)
            {
                try
                {
                    // capture image
                    ip = cam.GetBitMap();
                    image = new Bitmap(cam.Width, cam.Height, cam.Stride, System.Drawing.Imaging.PixelFormat.Format24bppRgb, ip);
                    image.RotateFlip(RotateFlipType.RotateNoneFlipY);
                    
                    motionDetector.ProcessFrame(ref image);
                   
                    if (motionDetector.MotionLevel >= alarmLevel)
                    {
                        if (!isRecording)
                        {
                            conManager.SendMessage("record");
                            isRecording = true;
                        }
                        lastMotion = DateTime.Now;
                    }
                    else
                    {
                        if (DateTime.Now.Subtract(lastMotion).Seconds > 5)
                        {
                            conManager.SendMessage("stop-record");
                            isRecording = false;
                        }
                    }

                    // add text that displays date time to the image
                    image.AddText(fontOverlay, 10, 10, DateTime.Now.ToString());

                    // save it to jpeg using quality options
                    image.Save(m, myImageCodecInfo, myEncoderParameters);

                    // send the jpeg image if server requests it
                    if (requested)
                        conManager.SendImage(m);

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

            cam.Pause();
            fontOverlay.Dispose();
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            // Set exit condition
            stopCondition = true;

            if (conManager != null)
            {
                conManager.Dispose();
            }

            if (cam != null)
            {
                cam.Dispose();
            }
            sw.Close();
        }

        private void ProcessMessage(object sender, string msg)
        {
            string[] message = msg.Split(' ');

            if (message[0] == "start")
            {
                requested = true;
            }
        }

        private void OnConnect(object sender, string msg)
        {
            ShowMessage(msg);

            ThreadStart listener = new ThreadStart(() => StartWebcam(cam, conManager, sw, myImageCodecInfo, myEncoderParameters));
            Thread thread = new Thread(listener);
            thread.Start();
        }

        private void OnDisconnect(object sender, string msg)
        {
            ShowMessage(msg);
            stopCondition = true;
            requested = false;
        }

        private void ShowMessage(string msg)
        {
            msgListBox.Dispatcher.BeginInvoke(new Action(() => msgListBox.Items.Add(msg)));
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

        private void sensitivitySlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            alarmLevel = sensitivitySlider.Value;
        }
    }
}
