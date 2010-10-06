using System;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.IO;
using System.Windows.Media.Imaging;
using System.Windows.Input;
using WpfWebcamClient.MVP;

namespace WpfWebcamClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, IClientView
    {
        private ClientPresenter _clPresenter;
        private bool _isConnected;

        public MainWindow()
        {
            InitializeComponent();

            _clPresenter = new ClientPresenter(this);
        }

        public string Status
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() => UpdateStatus(value)));
            }
        }

        public string[] Webcams
        {
            set
            {
                cbxWebcam.Dispatcher.BeginInvoke(new Action(() =>
                {
                    cbxWebcam.Items.Clear();
                    foreach (string v in value)
                        cbxWebcam.Items.Add(v);
                }));
            }
        }

        public byte[] CurrentFrame
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    MemoryStream ms = new MemoryStream(value);
                    BitmapImage bi = new BitmapImage();

                    bi.BeginInit();
                    bi.StreamSource = ms;
                    bi.EndInit();

                    pictureBox.Source = bi;
                }));
            }
        }

        public bool IsConnected
        {
            get { return _isConnected; }
            set
            {
                _isConnected = value;
                if (_isConnected)
                {
                    Dispatcher.BeginInvoke(new Action(() =>
                        {
                            txtAddress.IsEnabled = false;
                            txtPort.IsEnabled = false;
                            btnConnect.Content = "Disconnect";
                            btnStart.IsEnabled = true;
                            cbxWebcam.IsEnabled = true;
                        }));
                }
                else
                {
                    Dispatcher.BeginInvoke(new Action(() =>
                        {
                            txtAddress.IsEnabled = true;
                            txtPort.IsEnabled = true;
                            btnStart.IsEnabled = false;
                            btnStop.IsEnabled = false;
                            cbxWebcam.IsEnabled = false;
                            cbxWebcam.Items.Clear();
                            btnConnect.Content = "Connect";
                        }));
                }
            }
        }

        public bool IsStreaming
        {
            set
            {
                if (value)
                {
                    Dispatcher.BeginInvoke(new Action(() =>
                        {
                            txtRefresh.IsEnabled = false;
                            btnStart.IsEnabled = false;
                            btnStop.IsEnabled = true;
                            cbxWebcam.IsEnabled = false;
                            UpdateStatus("Start viewing webcam " + cbxWebcam.SelectedItem);
                        }));
                }
                else
                {
                    Dispatcher.BeginInvoke(new Action(() =>
                        {
                            txtRefresh.IsEnabled = true;
                            btnStart.IsEnabled = true;
                            btnStop.IsEnabled = false;
                            cbxWebcam.IsEnabled = true;
                            UpdateStatus("Stop viewing webcam " + cbxWebcam.SelectedItem);
                        }));
                }
            }
        }

        private void btnConnect_Click(object sender, RoutedEventArgs e)
        {
            if (((string)btnConnect.Content) == "Connect")
            {
                try { _clPresenter.Connect(txtAddress.Text, Convert.ToInt32(txtPort.Text)); }
                catch { MessageBox.Show("The port number you entered is invalid", "Error", MessageBoxButton.OK, MessageBoxImage.Error); }
            }
            else
                _clPresenter.Disconnect();
        }

        private void ProcessFrame(object sender, byte[] buffer)
        {
            Dispatcher.BeginInvoke(new Action(() =>
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
            if (cbxWebcam.SelectedItem != null)
                _clPresenter.StartStreaming(cbxWebcam.SelectedItem.ToString());
        }

        private void btnStop_Click(object sender, RoutedEventArgs e)
        {
            _clPresenter.StopStreaming(cbxWebcam.SelectedItem.ToString());
        }

        private void txtRefresh_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            _clPresenter.GetWebcamList();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            _clPresenter.Dispose();
        }

        private void UpdateStatus(string status)
        {
            lstStatus.Items.Add(string.Format("{0, -30}{1}", DateTime.Now.ToLongTimeString(), status));
        }
    }
}
