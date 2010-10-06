using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WpfWebcamServer
{
    public interface IFrameHandler
    {
        void HandleFrame(string sender, byte[] buffer);
    }
}
