using System.IO;

namespace WpfWebcamServer.Devices.View
{
    public interface IDeviceView
    {
        string Status { set; }
        string Notification { set; }
        string AddedClient { set; }
        string RemovedClient { set; }
        string AddedWebcam { set; }
        string RemovedWebcam { set; }
        string BasePath { get; }

        int BroadcastPort { get; }
        int DevicePort { get; }

        bool AllowVideoRecord { get; }
        bool AllowSoundAlarm { get; }

        void UpdateViewer(string name, MemoryStream stream);
    }
}
