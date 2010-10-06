using System;
using System.Windows;
using System.Windows.Input;

namespace WpfWebcamServer
{
    /// <summary>
    /// Interaction logic for ConfigDialog.xaml
    /// </summary>
    internal sealed partial class ConfigDialog : Window
    {
        public ConfigDialog(MainWindow owner)
        {
            InitializeComponent();

            this.Owner = owner;
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            filePathTextBox.Text = owner.BasePath;
            txtBroadcast.Text = Properties.Settings.Default.broadcastPort.ToString();
            txtWebcam.Text = Properties.Settings.Default.webcamPort.ToString();
            txtClient.Text = Properties.Settings.Default.clientPort.ToString();
            cbxRecord.IsChecked = owner.AllowVideoRecord;
            cbxSound.IsChecked = owner.AllowSoundAlarm;
            
            cbxRecord_Checked(null, null);
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                int bPort = Convert.ToInt32(txtBroadcast.Text);
                int wPort = Convert.ToInt32(txtWebcam.Text);
                int cPort = Convert.ToInt32(txtClient.Text);
                int maxPort = System.Net.IPEndPoint.MaxPort;
                int minPort = System.Net.IPEndPoint.MinPort;

                if (bPort > maxPort || wPort > maxPort || cPort > maxPort || bPort < minPort || wPort < minPort || cPort < minPort)
                    throw new OverflowException();

                Properties.Settings.Default.broadcastPort = bPort;
                Properties.Settings.Default.webcamPort = wPort;
                Properties.Settings.Default.clientPort = cPort;
                Properties.Settings.Default.basePath = filePathTextBox.Text;
                Properties.Settings.Default.Save();
                DialogResult = true;
                Close();
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

        private void browseButton_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FolderBrowserDialog fbDialog = new System.Windows.Forms.FolderBrowserDialog();
            if (fbDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                filePathTextBox.Text = fbDialog.SelectedPath;
        }

        private void cbxRecord_Checked(object sender, RoutedEventArgs e)
        {
            filePathTextBox.IsEnabled = cbxRecord.IsChecked ?? false;
            browseButton.IsEnabled = cbxRecord.IsChecked ?? false;
        }

        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }
    }
}
