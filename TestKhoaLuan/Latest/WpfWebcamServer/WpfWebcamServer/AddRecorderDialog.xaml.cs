using System;
using System.Windows;
using System.Windows.Input;

namespace WpfWebcamServer
{
    /// <summary>
    /// Interaction logic for AddRecorderDialog.xaml
    /// </summary>
    internal sealed partial class AddRecorderDialog : Window
    {
        public AddRecorderDialog()
        {
            InitializeComponent();
        }

        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private void addButton_Click(object sender, RoutedEventArgs e)
        {
            if (txtAddress.Text == "" || txtPort.Text == "")
                txtError.Text = "Invalid port or address";
            else
            {
                DialogResult = true;
                Close();
            }
        }
    }
}
