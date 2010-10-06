using System;
using System.Windows;
using System.Collections.Generic;
using System.Windows.Input;
using System.Windows.Controls;
using Microsoft.Win32;
using Sensor;
using WpfWebcamRecorder.MVP;

namespace WpfWebcamRecorder
{
    public partial class MainWindow : Window, IDeviceView
    {
        private DevicePresenter _dvPresenter;

        public MainWindow()
        {
            InitializeComponent();
            lstStatus.Items.Add("Starting...");
            lstStatus.Items.Add("Configuring environment...");

            Port = Properties.Settings.Default.Port;
            txtPort.Text = Port.ToString();
            txtSound.Text = Properties.Settings.Default.Sound;

            _dvPresenter = new DevicePresenter(this);
            sensitivitySlider.ValueChanged += sensitivitySlider_ValueChanged;
            sensitivitySlider.Value = 0.05;

            lstStatus.Items.Add("Ready!");
        }

        public string SoundPath { get; private set; }
        public int Port { get; private set; }

        public string Status
        {
            set { Dispatcher.BeginInvoke(new Action(() => lstStatus.Items.Add(value))); }
        }

        public string Notification { set { MessageBox.Show(value); } }

        public string[] Devices
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() =>
                    {
                        cbxDevice.Items.Clear();
                        foreach (string v in value)
                            cbxDevice.Items.Add(v);
                        if (!cbxDevice.Items.IsEmpty && cbxDevice.SelectedIndex == -1)
                            cbxDevice.SelectedIndex = 0;
                    }));
            }
        }

        public List<ISensor> Sensors
        {
            set
            {
                Dispatcher.BeginInvoke(new Action(() =>
                    {
                        cbxSensor.Items.Clear();
                        foreach (ISensor s in value)
                            cbxSensor.Items.Add(s.GetType().Name);
                        if (!cbxSensor.Items.IsEmpty && cbxSensor.SelectedIndex == -1)
                            cbxSensor.SelectedIndex = 0;
                    }));

            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            _dvPresenter.Dispose();
        }

        private void sensitivitySlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            _dvPresenter.SensorSensitivity = sensitivitySlider.Value;
        }

        private void selectButton_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "WAV Files(*.wav)|*.wav";
            dialog.CheckFileExists = true;
            if (dialog.ShowDialog() == true)
            {
                txtSound.Text = dialog.FileName;
                Properties.Settings.Default.Sound = dialog.FileName;
                Properties.Settings.Default.Save();
            }
        }

        private void portButton_Click(object sender, RoutedEventArgs e)
        {
            if ((string)portButton.Content == "Change")
            {
                txtPort.IsEnabled = true;
                portButton.Content = "Save";
            }
            else
            {
                try
                {
                    Properties.Settings.Default.Port = Convert.ToInt32(txtPort.Text);
                    Properties.Settings.Default.Save();
                    txtPort.IsEnabled = false;
                    portButton.Content = "Change";
                    MessageBox.Show("Changes will take effect after you restart the application", "Message",
                        MessageBoxButton.OK, MessageBoxImage.Information);
                }
                catch
                {
                    MessageBox.Show("Invalid port number. Valid values are from 0 to 65535", "Input Error",
                        MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void txtExit_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void cbxDevice_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _dvPresenter.SelectedDevice = cbxDevice.SelectedIndex;
        }

        private void cbxSensor_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _dvPresenter.SelectedSensor = cbxSensor.SelectedIndex;
        }

        private void txtSound_TextChanged(object sender, TextChangedEventArgs e)
        {
            SoundPath = txtSound.Text;
        }
    }
}
