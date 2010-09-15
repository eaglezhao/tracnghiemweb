using System;
using System.IO;

namespace WpfWebcamServer.Views
{
    public interface IResourceView
    {
        FileInfo[] RecordedVideos { set; }
        string BasePath { get; }
    }
}
