using System;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using WpfWebcamServer.Devices.Presenter;
using Network;

namespace WpfWebcamServer.Devices
{
    public class Client : Device, IFrameHandler
    {
        private DevicePresenter _dvPresenter;

        public Client(Messenger messenger, DevicePresenter presenter, string name)
            : base(messenger)
        {
            _dvPresenter = presenter;
            Name = name;
        }

        public string EventArg { get; private set; }

        public void HandleFrame(string sender, byte[] buffer)
        {
            SendMessage(buffer);
        }

        protected override void ProcessMessage(byte[] buffer, Network.MessageType type)
        {
            string[] message = Encoding.ASCII.GetString(buffer).Split(' ');

            if (message[0] == "start")
            {
                Webcam w = (Webcam)_dvPresenter.FindDevice(message[1]);

                if (w != null)
                {
                    w.AddClient(this);

                    EventArg = "Client " + Name + " requested webcam " + w.Name;
                    RaiseWebcamChangeEvent();
                }
            }
            else if (message[0] == "stop")
            {
                Webcam w = (Webcam)_dvPresenter.FindDevice(message[1]);

                if (w != null)
                {
                    w.RemoveClient(this);

                    EventArg = "Client " + Name + " stopped viewing webcam " + w.Name;
                    RaiseWebcamChangeEvent();
                }
            }
            else if (message[0] == "get-list")
            {
                string wcList = "webcam-list";
                foreach (string s in _dvPresenter.GetWebcamNames())
                    wcList += " " + s;
                SendMessage(wcList);
            }
        }

        private void RaiseWebcamChangeEvent()
        {
            if (WebcamChanged != null)
                WebcamChanged(this);
        }

        public event DeviceEventHandler WebcamChanged;
    }
}
