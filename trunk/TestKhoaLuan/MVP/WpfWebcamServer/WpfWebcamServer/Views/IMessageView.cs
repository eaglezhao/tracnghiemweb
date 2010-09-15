using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WpfWebcamServer.Views
{
    public interface IMessageView
    {
        string Status { set; }
        string Notification { set; }
    }
}
