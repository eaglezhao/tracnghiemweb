using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace WebcamServer
{
    public partial class WebcamServer : Form
    {
        private ConnectionManager conManager;

        public WebcamServer()
        {
            InitializeComponent();

            conManager = new ConnectionManager(this);
            conManager.DeviceDetected += new EventHandler(conManager_DeviceDetected);
        }

        void conManager_DeviceDetected(object sender, EventArgs e)
        {
            deviceComboBox.Items.Clear();

            foreach (Camera c in conManager.CameraManager.GetCameras())
                deviceComboBox.Items.Add(c.IPAddress);
        }

        private void deviceComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            conManager.StopViewing();
            conManager.ViewCamera((string) deviceComboBox.SelectedItem);
        }

        public void OnFrameReceived(object sender, byte[] b)
        {
            pictureBox.Image = new Bitmap(new MemoryStream(b));
        }

        private void detectButton_Click(object sender, EventArgs e)
        {
            conManager.AutoDetect();
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            conManager.StopViewing();
        }
    }
}
