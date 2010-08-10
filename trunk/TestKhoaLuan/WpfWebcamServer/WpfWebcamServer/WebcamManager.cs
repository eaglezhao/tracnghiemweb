using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WpfWebcamServer
{
    internal class WebcamManager
    {
        private List<Webcam> webcams = new List<Webcam>();

        public void AddWebcam(Webcam webcam)
        {
            lock (this)
            {
                webcams.Add(webcam);
            }
        }

        public bool RemoveWebcam(Webcam webcam)
        {
            lock (this)
            {
                if (webcams.Contains(webcam))
                {
                    webcams.Remove(webcam);
                    return true;
                }
                else
                    return false;
            }
        }

        public List<Webcam> GetWebcams()
        {
            return webcams;
        }

        public Webcam GetWebcamAt(int index)
        {
            return webcams[index];
        }

        public Webcam FindWebcam(string name)
        {
            lock (this)
            {
                foreach (Webcam w in webcams)
                    if (w.Name == name)
                        return w;

                return null;
            }
        }
    }
}
