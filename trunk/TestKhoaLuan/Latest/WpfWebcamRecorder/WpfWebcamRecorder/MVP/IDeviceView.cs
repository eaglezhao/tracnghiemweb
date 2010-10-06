using System.Collections.Generic;
using Sensor;

namespace WpfWebcamRecorder.MVP
{
    public interface IDeviceView
    {
        string Status { set; }
        string Notification { set; }
        string SoundPath { get; }
        int Port { get; }
        string[] Devices { set; }
        List<ISensor> Sensors { set; }
    }
}
