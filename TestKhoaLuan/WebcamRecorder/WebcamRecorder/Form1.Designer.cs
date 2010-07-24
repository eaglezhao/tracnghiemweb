namespace WebcamRecorder
{
    partial class RecorderForm
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
            this.msgListBox = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            // 
            // msgListBox
            // 
            this.msgListBox.FormattingEnabled = true;
            this.msgListBox.ItemHeight = 16;
            this.msgListBox.Location = new System.Drawing.Point(12, 12);
            this.msgListBox.Name = "msgListBox";
            this.msgListBox.Size = new System.Drawing.Size(427, 260);
            this.msgListBox.TabIndex = 0;
            // 
            // RecorderForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(451, 291);
            this.Controls.Add(this.msgListBox);
            this.Name = "RecorderForm";
            this.Text = "Webcam Recorder";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.RecorderForm_FormClosed);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox msgListBox;
    }
}

