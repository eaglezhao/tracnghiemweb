using System;
using WpfWebcamServer.Views;
using System.IO;
using System.Threading;

namespace WpfWebcamServer
{
    internal sealed class ResourcePresenter
    {
        private IResourceView _view;
        private volatile bool _executing = false;

        internal ResourcePresenter(IResourceView view)
        {
            _view = view;
        }

        internal void PlayVideo(string fileName)
        {
            System.Diagnostics.Process.Start("wmplayer.exe", "\"" + fileName + "\"");
        }

        internal void LoadVideoList()
        {
            if (!_executing)
            {
                _executing = true;
                System.Threading.Tasks.Task.Factory.StartNew(() =>
                    {
                        try
                        {
                            DirectoryInfo dir = new DirectoryInfo(_view.BasePath);
                            if (dir.Exists)
                                _view.RecordedVideos = dir.GetFiles("*.avi", SearchOption.AllDirectories);
                            else
                                _view.RecordedVideos = null;
                        }
                        catch (UnauthorizedAccessException) { }
                    });
                _executing = false;
            }
        }
    }
}
