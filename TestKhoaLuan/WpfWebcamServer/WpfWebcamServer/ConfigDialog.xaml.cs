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
using System.Windows.Shapes;

namespace WpfWebcamServer
{
    /// <summary>
    /// Interaction logic for ConfigDialog.xaml
    /// </summary>
    public partial class ConfigDialog : Window
    {
        public ConfigDialog(string basePath, Window owner)
        {
            InitializeComponent();

            this.Owner = owner;
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            filePathTextBox.Text = basePath;
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            Properties.Settings.Default.basePath = filePathTextBox.Text;
            Properties.Settings.Default.Save();
            Close();
        }

        private void browseButton_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FolderBrowserDialog fbDialog = new System.Windows.Forms.FolderBrowserDialog();
            if (fbDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                filePathTextBox.Text = fbDialog.SelectedPath;
        }
    }
}
