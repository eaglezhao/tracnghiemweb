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
using System.IO;

namespace WpfWebcamClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private ConnectionManager conManager;

        public MainWindow()
        {
            InitializeComponent();

            conManager = new ConnectionManager();
            conManager.MessageReceived += new ConnectionHandler(ProcessMessage);
            conManager.FrameReceived += new FrameHandler(ProcessFrame);
            conManager.Disconnected += new ConnectionHandler(OnDisconnect);
        }

        private void btnConnect_Click(object sender, RoutedEventArgs e)
        {
            if (((string)btnConnect.Content) == "Connect")
            {
                try
                {
                    conManager.Connect(txtAddress.Text, Convert.ToInt32(txtPort.Text));
                }
                catch
                {
                    MessageBox.Show("Cannot connect to server! Please make sure you enter the valid address and port number", "Connection failed",
                        MessageBoxButton.OK, MessageBoxImage.Error);
                    UpdateStatus("Failed to connect to server");
                }
            }
            else
            {
                conManager.Disconnect();
            }
        }

        private void ProcessMessage(object sender, string message)
        {
            string[] msg = message.Split(' ');

            if (msg[0] == "hello")
            {
                this.Dispatcher.BeginInvoke(new Action(() =>
                    {
                        txtAddress.IsEnabled = false;
                        txtPort.IsEnabled = false;
                        btnConnect.Content = "Disconnect";
                        btnStart.IsEnabled = true;
                        cbxWebcam.IsEnabled = true;
                        UpdateStatus("Connected to server " + txtAddress.Text);
                    }));
                conManager.SendMessage("get-list");
            }
            if (msg[0] == "webcam-list")
            {
                cbxWebcam.Dispatcher.BeginInvoke(new Action(() =>
                    {
                        cbxWebcam.Items.Clear();
                        for (int i = 1; i < msg.Length; i++)
                            cbxWebcam.Items.Add(msg[i]);
                    }));
            }
        }

        private void ProcessFrame(object sender, byte[] buffer)
        {
            this.Dispatcher.BeginInvoke(new Action(() =>
            {
                MemoryStream ms = new MemoryStream(buffer);
                BitmapImage bi = new BitmapImage();

                bi.BeginInit();
                bi.StreamSource = ms;
                bi.EndInit();

                pictureBox.Source = bi;
            }));
        }

        private void btnStart_Click(object sender, RoutedEventArgs e)
        {
            conManager.SendMessage("start " + cbxWebcam.SelectedItem);
            btnStart.IsEnabled = false;
            btnStop.IsEnabled = true;
            cbxWebcam.IsEnabled = false;
            UpdateStatus("Requesting video from webcam " + cbxWebcam.SelectedItem);
        }

        private void OnDisconnect(object sender, string message)
        {
            this.Dispatcher.BeginInvoke(new Action(() =>
                {
                    UpdateStatus(message);
                    txtAddress.IsEnabled = true;
                    txtPort.IsEnabled = true;
                    btnStart.IsEnabled = false;
                    btnStop.IsEnabled = false;
                    cbxWebcam.IsEnabled = false;
                    cbxWebcam.Items.Clear();
                    btnConnect.Content = "Connect";
                }));
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            conManager.Disconnect();
        }

        private void btnStop_Click(object sender, RoutedEventArgs e)
        {
            conManager.SendMessage("stop " + cbxWebcam.SelectedItem);
            btnStart.IsEnabled = true;
            btnStop.IsEnabled = false;
            cbxWebcam.IsEnabled = true;
            UpdateStatus("Stop viewing webcam " + cbxWebcam.SelectedItem);
        }

        private void UpdateStatus(string status)
        {
            lstStatus.Items.Add(string.Format("{0, -30}{1}", DateTime.Now.ToLongTimeString(), status));
        }
    }
}
