using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WpfWebcamServer.Views
{
    public interface IWebcamView : IMessageView
    {
        string AddedWebcam { set; }
        string RemovedWebcam { set; }
        string BasePath { get; }

        int BroadcastPort { get; }
        int WebcamPort { get; }

        bool AllowVideoRecord { get; }
        bool AllowSoundAlarm { get; }

        void UpdateViewer(string name, System.IO.MemoryStream stream);
    }
}
