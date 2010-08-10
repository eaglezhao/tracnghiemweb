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

namespace WpfWebcamServer
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

            int broadcastPort = Properties.Settings.Default.broadcastPort;
            int webcamPort = Properties.Settings.Default.webcamPort;
            int clientPort = Properties.Settings.Default.clientPort;
            string basePath = Properties.Settings.Default.basePath;

            if (string.IsNullOrEmpty(basePath))
                basePath = Environment.GetFolderPath(Environment.SpecialFolder.Personal);

            broadcastTextBox.Text = broadcastPort.ToString();
            webcamTextBox.Text = webcamPort.ToString();
            clientTextBox.Text = clientPort.ToString();

            conManager = new ConnectionManager(this, broadcastPort, webcamPort, clientPort, basePath);
            conManager.WebcamDetected += new ConnectionHandler(conManager_DeviceDetected);
        }

        private void conManager_DeviceDetected(object sender, string message)
        {
            deviceListBox.Dispatcher.BeginInvoke(new Action(() => deviceListBox.Items.Add(message)));
        }

        public void OnFrameReceived(object sender, byte[] b)
        {
            this.Dispatcher.BeginInvoke(new Action(() =>
                {
                    MemoryStream ms = new MemoryStream(b);
                    BitmapImage bi = new BitmapImage();

                    bi.BeginInit();
                    bi.StreamSource = ms;
                    bi.EndInit();

                    pictureBox.Source = bi;
                }));
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            conManager.Dispose();
        }

        private void deviceListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            conManager.StopViewing((string)deviceListBox.SelectedItem);

            if (deviceListBox.SelectedItem != null)
                conManager.ViewCamera((string)deviceListBox.SelectedItem);
        }

        private void TextBlock_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            TextBlock t = (TextBlock)sender;

            if (t.Text == "Change")
            {
                broadcastTextBox.IsEnabled = true;
                webcamTextBox.IsEnabled = true;
                clientTextBox.IsEnabled = true;
                t.Text = "Save";
            }
            else
            {
                try
                {
                    int bPort = Convert.ToInt32(broadcastTextBox.Text);
                    int wPort = Convert.ToInt32(webcamTextBox.Text);
                    int cPort = Convert.ToInt32(clientTextBox.Text);

                    Properties.Settings.Default.broadcastPort = bPort;
                    Properties.Settings.Default.webcamPort = wPort;
                    Properties.Settings.Default.clientPort = cPort;

                    broadcastTextBox.IsEnabled = false;
                    webcamTextBox.IsEnabled = false;
                    clientTextBox.IsEnabled = false;
                    t.Text = "Change";

                    MessageBox.Show("Changes will take effect after you restart the application", "Message",
                        MessageBoxButton.OK, MessageBoxImage.Information);
                }
                catch (FormatException)
                {
                    MessageBox.Show("Invalid port number. Valid values are from 0 to 65535", "Input Error",
                        MessageBoxButton.OK, MessageBoxImage.Error);
                }
                catch (OverflowException)
                {
                    MessageBox.Show("Invalid port number. Valid values are from 0 to 65535", "Input Error",
                        MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }
    }
}
