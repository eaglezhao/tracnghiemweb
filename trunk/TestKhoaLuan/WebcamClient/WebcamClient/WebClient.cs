/****************************************************************************
While the underlying libraries are covered by LGPL, this sample is released 
as public domain.  It is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  
*****************************************************************************/

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Text;
using System.IO;

namespace WebCamClient
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
    public class Form1 : System.Windows.Forms.Form
    {
        private UdpClient udpListener = new UdpClient(9999);

        public Form1()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            udpListener.BeginReceive(new AsyncCallback(OnReceive), udpListener);
        }

        private void OnReceive(IAsyncResult ar)
        {
            IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 1112);
            MemoryStream m = new MemoryStream(udpListener.EndReceive(ar, ref remoteEP));

            pictureBox1.Image = new Bitmap(m);

            udpListener.BeginReceive(new AsyncCallback(OnReceive), udpListener);
        }

		#region Windows Form Designer generated code

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.TextBox txtServer;
        private System.Windows.Forms.Button btnPress;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtMessage;
        private System.Windows.Forms.TextBox txtPort;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtFrames;
        private System.ComponentModel.IContainer components;

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.btnPress = new System.Windows.Forms.Button();
            this.txtServer = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtMessage = new System.Windows.Forms.TextBox();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txtFrames = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBox1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pictureBox1.Location = new System.Drawing.Point(0, 0);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(384, 220);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // btnPress
            // 
            this.btnPress.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnPress.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnPress.Location = new System.Drawing.Point(278, 321);
            this.btnPress.Name = "btnPress";
            this.btnPress.Size = new System.Drawing.Size(96, 37);
            this.btnPress.TabIndex = 1;
            this.btnPress.Tag = "1";
            this.btnPress.Text = "Start";
            this.btnPress.Click += new System.EventHandler(this.btnPress_Click);
            // 
            // txtServer
            // 
            this.txtServer.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.txtServer.Location = new System.Drawing.Point(19, 275);
            this.txtServer.Name = "txtServer";
            this.txtServer.Size = new System.Drawing.Size(115, 22);
            this.txtServer.TabIndex = 3;
            this.txtServer.Text = "192.168.15.145";
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(19, 229);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(144, 46);
            this.label1.TabIndex = 4;
            this.label1.Text = "Enter server name or ip address";
            // 
            // txtMessage
            // 
            this.txtMessage.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.txtMessage.Location = new System.Drawing.Point(10, 377);
            this.txtMessage.Name = "txtMessage";
            this.txtMessage.ReadOnly = true;
            this.txtMessage.Size = new System.Drawing.Size(364, 22);
            this.txtMessage.TabIndex = 5;
            // 
            // txtPort
            // 
            this.txtPort.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.txtPort.Location = new System.Drawing.Point(192, 275);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(58, 22);
            this.txtPort.TabIndex = 6;
            this.txtPort.Text = "1112";
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label2.Location = new System.Drawing.Point(192, 248);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(86, 26);
            this.label2.TabIndex = 7;
            this.label2.Text = "Port Number";
            // 
            // txtFrames
            // 
            this.txtFrames.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.txtFrames.Location = new System.Drawing.Point(298, 275);
            this.txtFrames.Name = "txtFrames";
            this.txtFrames.ReadOnly = true;
            this.txtFrames.Size = new System.Drawing.Size(76, 22);
            this.txtFrames.TabIndex = 8;
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label3.Location = new System.Drawing.Point(298, 248);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(57, 18);
            this.label3.TabIndex = 9;
            this.label3.Text = "Frames";
            // 
            // Form1
            // 
            this.AcceptButton = this.btnPress;
            this.AutoScaleBaseSize = new System.Drawing.Size(6, 15);
            this.CancelButton = this.btnPress;
            this.ClientSize = new System.Drawing.Size(384, 407);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.txtFrames);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.txtMessage);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtServer);
            this.Controls.Add(this.btnPress);
            this.Controls.Add(this.pictureBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "WebClient";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
		#endregion

        private void btnPress_Click(object sender, System.EventArgs e)
        {
            // If the button says 'Start'
            if ((string)btnPress.Tag == "1")
            {
                // Create a new thread to receive the images
                try
                {
                    UdpClient udpSender = new UdpClient();
                    string msg = "req 192.168.15.147 192.168.15.145";
                    byte[] b = Encoding.ASCII.GetBytes(msg);
                    udpSender.Send(b, b.Length, txtServer.Text, Convert.ToInt32(txtPort.Text));
                }
                catch (Exception ex)
                {
                    txtMessage.Text = ex.Message;
                    return;
                }

                // Reset the button tag and description
                btnPress.Tag = "2";
                btnPress.Text = "Stop";
            }
            else
            {
                Stop();
            }
        }

        private void Stop()
        {
            btnPress.Tag = "1";
            btnPress.Text = "Start";

            UdpClient udpSender = new UdpClient();
            string msg = "sto 192.168.15.147 192.168.15.145";
            byte[] b = Encoding.ASCII.GetBytes(msg);
            udpSender.Send(b, b.Length, txtServer.Text, Convert.ToInt32(txtPort.Text));
        }
	}
}
