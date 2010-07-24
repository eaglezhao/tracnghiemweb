namespace WebcamServer
{
    partial class WebcamServer
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.addrComboBox = new System.Windows.Forms.ComboBox();
            this.msgListBox = new System.Windows.Forms.ListBox();
            this.stopButton = new System.Windows.Forms.Button();
            this.statusTextBox = new System.Windows.Forms.TextBox();
            this.deviceComboBox = new System.Windows.Forms.ComboBox();
            this.detectButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox
            // 
            this.pictureBox.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.pictureBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pictureBox.Location = new System.Drawing.Point(12, 12);
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.Size = new System.Drawing.Size(640, 480);
            this.pictureBox.TabIndex = 1;
            this.pictureBox.TabStop = false;
            // 
            // addrComboBox
            // 
            this.addrComboBox.FormattingEnabled = true;
            this.addrComboBox.Location = new System.Drawing.Point(672, 302);
            this.addrComboBox.Name = "addrComboBox";
            this.addrComboBox.Size = new System.Drawing.Size(250, 24);
            this.addrComboBox.TabIndex = 2;
            // 
            // msgListBox
            // 
            this.msgListBox.FormattingEnabled = true;
            this.msgListBox.ItemHeight = 16;
            this.msgListBox.Location = new System.Drawing.Point(672, 12);
            this.msgListBox.Name = "msgListBox";
            this.msgListBox.Size = new System.Drawing.Size(250, 228);
            this.msgListBox.TabIndex = 3;
            // 
            // stopButton
            // 
            this.stopButton.Location = new System.Drawing.Point(672, 349);
            this.stopButton.Name = "stopButton";
            this.stopButton.Size = new System.Drawing.Size(111, 23);
            this.stopButton.TabIndex = 4;
            this.stopButton.Text = "Stop";
            this.stopButton.UseVisualStyleBackColor = true;
            this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // statusTextBox
            // 
            this.statusTextBox.Enabled = false;
            this.statusTextBox.Location = new System.Drawing.Point(12, 507);
            this.statusTextBox.Name = "statusTextBox";
            this.statusTextBox.Size = new System.Drawing.Size(910, 22);
            this.statusTextBox.TabIndex = 5;
            // 
            // deviceComboBox
            // 
            this.deviceComboBox.FormattingEnabled = true;
            this.deviceComboBox.Location = new System.Drawing.Point(672, 261);
            this.deviceComboBox.Name = "deviceComboBox";
            this.deviceComboBox.Size = new System.Drawing.Size(250, 24);
            this.deviceComboBox.TabIndex = 6;
            this.deviceComboBox.SelectedIndexChanged += new System.EventHandler(this.deviceComboBox_SelectedIndexChanged);
            // 
            // detectButton
            // 
            this.detectButton.Location = new System.Drawing.Point(811, 349);
            this.detectButton.Name = "detectButton";
            this.detectButton.Size = new System.Drawing.Size(111, 23);
            this.detectButton.TabIndex = 7;
            this.detectButton.Text = "Detect Devices";
            this.detectButton.UseVisualStyleBackColor = true;
            this.detectButton.Click += new System.EventHandler(this.detectButton_Click);
            // 
            // WebcamServer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(931, 542);
            this.Controls.Add(this.detectButton);
            this.Controls.Add(this.deviceComboBox);
            this.Controls.Add(this.statusTextBox);
            this.Controls.Add(this.stopButton);
            this.Controls.Add(this.msgListBox);
            this.Controls.Add(this.addrComboBox);
            this.Controls.Add(this.pictureBox);
            this.Name = "WebcamServer";
            this.Text = "WebcamServer";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox;
        private System.Windows.Forms.ComboBox addrComboBox;
        private System.Windows.Forms.ListBox msgListBox;
        private System.Windows.Forms.Button stopButton;
        private System.Windows.Forms.TextBox statusTextBox;
        private System.Windows.Forms.ComboBox deviceComboBox;
        private System.Windows.Forms.Button detectButton;
    }
}