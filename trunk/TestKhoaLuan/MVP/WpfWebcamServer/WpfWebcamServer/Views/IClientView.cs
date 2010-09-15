using System;

namespace WpfWebcamServer.Views
{
    public interface IClientView : IMessageView
    {
        string AddedClient { set; }
        string RemovedClient { set; }
        int ClientPort { get; }
        Webcams.WebcamPresenter WCPresenter { get; }
    }
}
