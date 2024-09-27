using HPPH;
using RemoteViewing.Vnc;
using ScreenCapture.NET;

namespace Agent.VNC
{
    internal class VNCFramebufferSource : IVncFramebufferSource, IDisposable
    {
        public bool SupportsResizing => false;

        public int Width { get; set; } = 200;

        public int Height { get; set; } = 400;

        private readonly IScreenCaptureService screenCaptureService;
        private readonly IEnumerable<GraphicsCard> graphicsCards;
        private readonly IEnumerable<Display> displays;
        private readonly IScreenCapture screenCapture;
        private readonly ICaptureZone fullscreen;
        private readonly int downscale = 2;
        private VncFramebuffer? framebuffer = null;
        private bool disposedValue;
        private DateTime lastUpdateTime = DateTime.MinValue;
        private Action onNewFrame;

        public VNCFramebufferSource(Action onNewFrame, int downscale = 2)
        {
            this.onNewFrame = onNewFrame;

            // Create a screen-capture service
            screenCaptureService = new DX11ScreenCaptureService();

            // Get all available graphics cards
            graphicsCards = screenCaptureService.GetGraphicsCards();

            // Get the displays from the graphics card(s) you are interested in
            displays = screenCaptureService.GetDisplays(graphicsCards.First());

            // Create a screen-capture for all screens you want to capture
            screenCapture = screenCaptureService.GetScreenCapture(displays.First());

            this.downscale = downscale;

            // Register the regions you want to capture on the screen
            // Capture the whole screen
            fullscreen = screenCapture.RegisterCaptureZone(0, 0, screenCapture.Display.Width, screenCapture.Display.Height, downscaleLevel: downscale);

            Width = fullscreen.Width;
            Height = fullscreen.Height;
        }

        public VncFramebuffer Capture()
        {
            onNewFrame();

            // Capture the screen
            // This should be done in a loop on a separate thread as CaptureScreen blocks if the screen is not updated (still image).
            screenCapture.CaptureScreen();

            using (fullscreen.Lock())
            {
                IImage image = fullscreen.Image;

                if (this.framebuffer == null || this.framebuffer.Width != image.Width || this.framebuffer.Height != image.Height)
                {
                    this.framebuffer = new VncFramebuffer("VNC", image.Width, image.Height, new VncPixelFormat());
                }

                RefImage<ColorBGRA> refImage = image.AsRefImage<ColorBGRA>();

                lock (this.framebuffer.SyncRoot)
                {
                    for (int row = 0; row < image.Height; row++)
                    {
                        for (int col = 0; col < image.Width; col++)
                        { 
                            this.framebuffer.SetPixel(col, row, new byte[] { refImage[col, row].B, refImage[col, row].G, refImage[col, row].R, refImage[col, row].A, });
                        }
                    }
                }

                lastUpdateTime = DateTime.UtcNow;

                return this.framebuffer;
            }
        }

        public ExtendedDesktopSizeStatus SetDesktopSize(int width, int height)
        {
            return ExtendedDesktopSizeStatus.Prohibited;
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    screenCapture.Dispose();
                }

                disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }

        internal void Reset()
        {
            lastUpdateTime = DateTime.UtcNow;
        }
    }
}
