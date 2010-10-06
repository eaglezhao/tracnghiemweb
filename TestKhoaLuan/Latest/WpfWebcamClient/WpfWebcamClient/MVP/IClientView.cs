namespace WpfWebcamClient.MVP
{
    public interface IClientView
    {
        string Status { set; }
        string[] Webcams { set; }
        byte[] CurrentFrame { set; }
        bool IsConnected { get; set; }
        bool IsStreaming { set; }
    }
}
