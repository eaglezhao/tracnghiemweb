using System;
using System.Threading;
using System.Runtime.InteropServices;
using DirectShowLib;

namespace WpfWebcamRecorder.MVP
{
    public sealed class Device : ISampleGrabberCB, IDisposable
    {
        #region Member variables

        // graph builder interface.
        private IFilterGraph2 _filterGraph = null;
        private IMediaControl _mediaCtrl = null;

        // so we can wait for the async job to finish
        private ManualResetEvent _pictureReady = null;

        // Set by async routine when it captures an image
        private volatile bool _gotOne = false;

        // Indicates the status of the graph
        private bool _running = false;

        // Dimensions of the image, calculated once in constructor
        private IntPtr _handle = IntPtr.Zero;
        public int _dropped = 0;

        private bool _disposed = false;

        #endregion

        #region API

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory")]
        private static extern void CopyMemory(IntPtr Destination, IntPtr Source, int Length);

        #endregion

        public Device(int iDeviceNum)
            : this(iDeviceNum, 0, 0, 0)
        {
        }

        public Device(int deviceNum, int frameRate, int width, int height)
        {
            // Get the collection of video devices
            DsDevice[] capDevices = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);

            if (deviceNum + 1 > capDevices.Length)
            {
                throw new Exception("No video capture devices found at that index!");
            }

            try
            {
                // Set up the capture graph
                SetupGraph(capDevices[deviceNum], frameRate, width, height);

                // tell the callback to ignore new images
                _pictureReady = new ManualResetEvent(false);
                _gotOne = true;
                _running = false;
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        ~Device()
        {
            Dispose(false);
        }

        public int Width { get; private set; }
        public int Height { get; private set; }
        public int Stride { get; private set; }

        // capture the next image
        public IntPtr GetBitMap()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            _handle = Marshal.AllocCoTaskMem(Stride * Height);

            try
            {
                // get ready to wait for new image
                _pictureReady.Reset();
                _gotOne = false;
                
                // Start waiting
                if (!_pictureReady.WaitOne(5000, false))
                {
                    throw new Exception("Timeout waiting to get picture");
                }
            }
            catch
            {
                Marshal.FreeCoTaskMem(_handle);
                throw;
            }

            // Got one
            return _handle;
        }

        // Start the capture graph
        public void Start()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);
            
            if (!_running)
            {
                int hr = _mediaCtrl.Run();
                DsError.ThrowExceptionForHR(hr);
                
                _running = true;
            }
        }

        // Pause the capture graph.
        // Running the graph takes up a lot of resources.  Pause it when it isn't needed.
        public void Pause()
        {
            if (_disposed)
                throw new ObjectDisposedException(GetType().FullName);

            if (_running)
            {
                int hr = _mediaCtrl.Pause();
                DsError.ThrowExceptionForHR(hr);

                _running = false;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    CloseInterfaces();
                    if (_pictureReady != null)
                    {
                        _pictureReady.Close();
                        _pictureReady = null;
                    }
                }
                _disposed = true;
            }
        }

        // build the capture graph for grabber
        private void SetupGraph(DsDevice dev, int frameRate, int width, int height)
        {
            int hr;

            ISampleGrabber sampGrabber = null;
            IBaseFilter capFilter = null;
            ICaptureGraphBuilder2 capGraph = null;

            // Get the graphbuilder object
            _filterGraph = (IFilterGraph2)new FilterGraph();
            _mediaCtrl = _filterGraph as IMediaControl;
            try
            {
                // Get the ICaptureGraphBuilder2
                capGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();

                // Get the SampleGrabber interface
                sampGrabber = (ISampleGrabber)new SampleGrabber();

                // Start building the graph
                hr = capGraph.SetFiltergraph(_filterGraph);
                DsError.ThrowExceptionForHR(hr);

                // Add the video device
                hr = _filterGraph.AddSourceFilterForMoniker(dev.Mon, null, "Video input", out capFilter);
                DsError.ThrowExceptionForHR(hr);

                IBaseFilter baseGrabFlt = (IBaseFilter)sampGrabber;
                ConfigureSampleGrabber(sampGrabber);

                // Add the frame grabber to the graph
                hr = _filterGraph.AddFilter(baseGrabFlt, "Ds.NET Grabber");
                DsError.ThrowExceptionForHR(hr);

                // If any of the default config items are set
                if (frameRate + height + width > 0)
                {
                    SetConfigParms(capGraph, capFilter, frameRate, width, height);
                }

                hr = capGraph.RenderStream(PinCategory.Capture, MediaType.Video, capFilter, null, baseGrabFlt);
                DsError.ThrowExceptionForHR(hr);

                SaveSizeInfo(sampGrabber);
            }
            finally
            {
                if (capFilter != null)
                {
                    Marshal.ReleaseComObject(capFilter);
                    capFilter = null;
                }
                if (sampGrabber != null)
                {
                    Marshal.ReleaseComObject(sampGrabber);
                    sampGrabber = null;
                }
                if (capGraph != null)
                {
                    Marshal.ReleaseComObject(capGraph);
                    capGraph = null;
                }
            }
        }

        private void SaveSizeInfo(ISampleGrabber sampGrabber)
        {
            int hr;

            // Get the media type from the SampleGrabber
            AMMediaType media = new AMMediaType();
            hr = sampGrabber.GetConnectedMediaType(media);
            DsError.ThrowExceptionForHR(hr);

            if ((media.formatType != FormatType.VideoInfo) || (media.formatPtr == IntPtr.Zero))
            {
                throw new NotSupportedException("Unknown Grabber Media Format");
            }

            // Grab the size info
            VideoInfoHeader videoInfoHeader = (VideoInfoHeader)Marshal.PtrToStructure(media.formatPtr, typeof(VideoInfoHeader));
            Width = videoInfoHeader.BmiHeader.Width;
            Height = videoInfoHeader.BmiHeader.Height;
            Stride = Width * (videoInfoHeader.BmiHeader.BitCount / 8);

            DsUtils.FreeAMMediaType(media);
            media = null;
        }

        private void ConfigureSampleGrabber(ISampleGrabber sampGrabber)
        {
            AMMediaType media;
            int hr;

            // Set the media type to Video/RBG24
            media = new AMMediaType();
            media.majorType = MediaType.Video;
            media.subType = MediaSubType.RGB24;
            media.formatType = FormatType.VideoInfo;
            hr = sampGrabber.SetMediaType(media);
            DsError.ThrowExceptionForHR(hr);

            DsUtils.FreeAMMediaType(media);
            media = null;

            // Configure the samplegrabber
            hr = sampGrabber.SetCallback(this, 1);
            DsError.ThrowExceptionForHR(hr);
        }

        // Set the Framerate, and video size
        private void SetConfigParms(ICaptureGraphBuilder2 capGraph, IBaseFilter capFilter, int frameRate, int width, int height)
        {
            int hr;
            object o;
            AMMediaType media;

            // Find the stream config interface
            hr = capGraph.FindInterface(
                PinCategory.Capture, MediaType.Video, capFilter, typeof(IAMStreamConfig).GUID, out o);
            /*hr = capGraph.FindInterface(
                null, MediaType.Video, capFilter, typeof(IAMStreamConfig).GUID, out o);*/

            IAMStreamConfig videoStreamConfig = o as IAMStreamConfig;
            if (videoStreamConfig == null)
            {
                throw new Exception("Failed to get IAMStreamConfig");
            }

            // Get the existing format block
            hr = videoStreamConfig.GetFormat(out media);
            DsError.ThrowExceptionForHR(hr);

            // copy out the videoinfoheader
            VideoInfoHeader v = new VideoInfoHeader();
            Marshal.PtrToStructure(media.formatPtr, v);

            // if overriding the framerate, set the frame rate
            if (frameRate > 0)
            {
                v.AvgTimePerFrame = 10000000 / frameRate;
            }

            // if overriding the width, set the width
            if (width > 0)
            {
                v.BmiHeader.Width = width;
            }

            // if overriding the Height, set the Height
            if (height > 0)
            {
                v.BmiHeader.Height = height;
            }

            // Copy the media structure back
            Marshal.StructureToPtr(v, media.formatPtr, false);

            // Set the new format
            hr = videoStreamConfig.SetFormat(media);
            DsError.ThrowExceptionForHR(hr);

            DsUtils.FreeAMMediaType(media);
            media = null;
        }

        // Shut down capture
        private void CloseInterfaces()
        {
            int hr;

            try
            {
                if (_mediaCtrl != null)
                {
                    // Stop the graph
                    hr = _mediaCtrl.Stop();
                    _running = false;
                }
            }
            catch { }

            if (_filterGraph != null)
            {
                Marshal.ReleaseComObject(_filterGraph);
                _filterGraph = null;
            }
        }

        // sample callback, NOT USED
        int ISampleGrabberCB.SampleCB(double SampleTime, IMediaSample pSample)
        {
            if (!_gotOne)
            {
                // Set bGotOne to prevent further calls until we
                // request a new bitmap.
                _gotOne = true;
                IntPtr pBuffer;

                pSample.GetPointer(out pBuffer);
                int iBufferLen = pSample.GetSize();

                if (pSample.GetSize() > Stride * Height)
                {
                    throw new Exception("Buffer is wrong size");
                }

                CopyMemory(_handle, pBuffer, Stride * Height);

                // Picture is ready.
                _pictureReady.Set();
            }

            Marshal.ReleaseComObject(pSample);
            return 0;
        }

        /// <summary> buffer callback, COULD BE FROM FOREIGN THREAD. </summary>
        int ISampleGrabberCB.BufferCB(double SampleTime, IntPtr pBuffer, int BufferLen)
        {
            if (!_gotOne)
            {
                // The buffer should be long enought
                if (BufferLen <= Stride * Height)
                {
                    // Copy the frame to the buffer
                    CopyMemory(_handle, pBuffer, Stride * Height);
                }
                else
                {
                    throw new Exception("Buffer is wrong size");
                }

                // Set bGotOne to prevent further calls until we
                // request a new bitmap.
                _gotOne = true;

                // Picture is ready.
                _pictureReady.Set();
            }
            else
            {
                _dropped++;
            }
            return 0;
        }
    }
}
