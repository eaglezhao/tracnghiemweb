using System;
using System.IO;

namespace WpfWebcamServer.Resources
{
    public interface IResourceView
    {
        FileInfo[] RecordedVideos { set; }
        string BasePath { get; }
    }
}
