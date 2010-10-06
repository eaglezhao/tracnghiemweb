using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using Fluent;
using System.Windows.Media;
using WpfWebcamServer.Devices.View;
using WpfWebcamServer.Resources;
using WpfWebcamServer.Devices.Presenter;

namespace WpfWebcamServer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : RibbonWindow, IDeviceView, IResourceView
    {
        private ResourcePresenter _rsPresenter;
        private Dictionary<string, Image> _viewers = new Dictionary<string, Image>();

        private const int viewerWidth = 200;
        private const int viewerHeight = 150;

        public MainWindow()
        {
            InitializeComponent();

            UpdateStatus("Starting server...");

            BroadcastPort = Properties.Settings.Default.broadcastPort;
            DevicePort = Properties.Settings.Default.webcamPort;
            BasePath = Properties.Settings.Default.basePath;

            if (string.IsNullOrEmpty(BasePath))
                BasePath = Environment.GetFolderPath(Environment.SpecialFolder.Personal);

            DVPresenter = new DevicePresenter(this);
            _rsPresenter = new ResourcePresenter(this);

            cbxDevice.Items.Add("All webcams");
            cbxDevice.SelectedIndex = 0;
            UpdateStatus("Ready!");
        }

        // IMessageView properties
        public string Notification { set { MessageBox.Show(value); } }
        public string Status { set { Dispatcher.BeginInvoke(new Action(() => UpdateStatus(value))); } }

        // IWebcamView properties & methods
        public string AddedWebcam
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() =>
                    {
                        cbxDevice.Items.Add(value);
                        Image viewer = new Image();
                        viewer.Tag = value;
                        viewer.Width = viewerWidth;
                        viewer.Height = viewerHeight;
                        viewer.Cursor = Cursors.Hand;
                        viewer.Margin = new Thickness(10);
                        viewer.MouseLeftButtonUp += new MouseButtonEventHandler(viewer_MouseLeftButtonUp);
                        _viewers.Add(value, viewer);
                        if (cbxDevice.SelectedIndex == 0)
                            pnlWebcam.Children.Add(viewer);
                    }));
            }
        }
        public string RemovedWebcam
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() =>
                    {
                        cbxDevice.Items.Remove(value);
                        Image viewer;
                        _viewers.TryGetValue(value, out viewer);
                        if (cbxDevice.SelectedIndex == 0 && viewer != null)
                            pnlWebcam.Children.Remove(viewer);
                        _viewers.Remove(value);
                    }));
            }
        }
        public int BroadcastPort { get; private set; }
        public int DevicePort { get; private set; }
        public string BasePath { get; private set; }
        public bool AllowVideoRecord { get; private set; }
        public bool AllowSoundAlarm { get; private set; }
        public void UpdateViewer(string name, MemoryStream ms)
        {
            Dispatcher.BeginInvoke(new Action(() =>
                {
                    BitmapImage bi = new BitmapImage();
                    bi.BeginInit();
                    bi.StreamSource = ms;
                    bi.EndInit();

                    Image image;
                    _viewers.TryGetValue(name, out image);
                    if(image != null)
                        image.Source = bi;
                }));
        }

        // IClientView properties
        public string AddedClient { set { Dispatcher.BeginInvoke(new Action(() => lstClient.Items.Add(value))); } }
        public string RemovedClient { set { Dispatcher.BeginInvoke(new Action(() => lstClient.Items.Remove(value))); } }
        public DevicePresenter DVPresenter { get; private set; }

        // IResourceView properties
        public FileInfo[] RecordedVideos
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    videoPanel.Children.Clear();
                    if (value != null)
                    {
                        BitmapImage bmp = new BitmapImage(new Uri("/Images/recorded_video.png", UriKind.Relative));

                        foreach (FileInfo f in value)
                        {
                            StackPanel panel = new StackPanel();
                            panel.Orientation = Orientation.Horizontal;
                            panel.Margin = new Thickness(5);
                            panel.Cursor = Cursors.Hand;
                            panel.Tag = f.FullName;
                            panel.MouseLeftButtonUp += (sender, e) => _rsPresenter.PlayVideo((string)((StackPanel)sender).Tag);

                            Image img = new Image();
                            img.Width = 80;
                            img.Height = 60;
                            img.Source = bmp;

                            TextBlock textBlock = new TextBlock();
                            textBlock.Text = f.Name;

                            panel.Children.Add(img);
                            panel.Children.Add(textBlock);
                            videoPanel.Children.Add(panel);
                        }
                    }
                }));
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            DVPresenter.Dispose();
        }

        private void viewer_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            cbxDevice.SelectedItem = ((Image)sender).Tag.ToString();
        }

        private void cbxDevice_SelectionChanged(object sender, EventArgs e)
        {
            pnlWebcam.Children.Clear();
            if (cbxDevice.SelectedItem != null)
            {
                if (cbxDevice.SelectedIndex == 0)
                {
                    pnlWebcam.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
                    pnlWebcam.VerticalAlignment = System.Windows.VerticalAlignment.Top;
                    foreach (Image viewer in _viewers.Values)
                    {
                        viewer.Width = viewerWidth;
                        viewer.Height = viewerHeight;
                        pnlWebcam.Children.Add(viewer);
                    }
                }
                else
                {
                    Image viewer;
                    _viewers.TryGetValue(cbxDevice.SelectedItem.ToString(), out viewer);
                    if (viewer != null)
                    {
                        viewer.Width = viewer.Source.Width;
                        viewer.Height = viewer.Source.Height;
                        pnlWebcam.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                        pnlWebcam.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                        pnlWebcam.Children.Add(viewer);
                    }
                }
            }
        }

        private void UpdateStatus(string status)
        {
            lstStatus.Items.Add(string.Format("{0, -20}{1}", DateTime.Now.ToLongTimeString(), status));
        }

        private void ConfigButton_Click(object sender, RoutedEventArgs e)
        {
            ConfigDialog configDialog = new ConfigDialog(this);
            if (configDialog.ShowDialog() == true)
            {
                MessageBox.Show("Changes will take effect after you restart the application", "Notice",
                    MessageBoxButton.OK, MessageBoxImage.Information);

                chkRecord.IsChecked = configDialog.cbxRecord.IsChecked ?? false;
                AllowVideoRecord = chkRecord.IsChecked;
                chkSound.IsChecked = configDialog.cbxSound.IsChecked ?? false;
                AllowSoundAlarm = chkSound.IsChecked;
            }
        }

        private void chkRecord_Checked(object sender, EventArgs e)
        {
            AllowVideoRecord = chkRecord.IsChecked;
        }

        private void chkSound_Checked(object sender, EventArgs e)
        {
            AllowSoundAlarm = chkSound.IsChecked;
        }

        private void DetectButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateStatus("Detecting devices...");
            DVPresenter.Discover();
        }

        private void AddButton_Click(object sender, RoutedEventArgs e)
        {
            AddRecorderDialog dialog = new AddRecorderDialog();
            dialog.Owner = this;
            dialog.WindowStartupLocation = System.Windows.WindowStartupLocation.CenterOwner;
            if (dialog.ShowDialog() == true)
            {
                DVPresenter.Discover(dialog.txtAddress.Text, Convert.ToInt32(dialog.txtPort.Text));
            }
        }

        private void btnRefresh_Click(object sender, RoutedEventArgs e)
        {
            _rsPresenter.LoadVideoList();
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (tabControl.SelectedIndex == 1)
                _rsPresenter.LoadVideoList();
        }
    }
}
