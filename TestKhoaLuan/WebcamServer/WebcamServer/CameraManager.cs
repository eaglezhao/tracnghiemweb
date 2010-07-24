using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WebcamServer
{
    internal class CameraManager
    {
        private List<Camera> cameras = new List<Camera>();

        public void AddCamera(string ip)
        {
            cameras.Add(new Camera(ip));
        }

        public void RemoveCamera(string ip)
        {
            Camera c = FindCamera(ip);
            if (c != null)
                cameras.Remove(c);
        }

        public List<Camera> GetCameras()
        {
            return cameras;
        }

        public Camera GetCameraAt(int index)
        {
            return cameras[index];
        }

        public Camera FindCamera(string ip)
        {
            foreach (Camera c in cameras)
                if (c.IPAddress == ip)
                    return c;

            return null;
        }
    }
}
