using RemoteViewing.Vnc.Server;
using RemoteViewing.Vnc;
using Microsoft.Extensions.Logging;

namespace Agent.VNC
{
    internal class VNCServer : IVncPasswordChallenge, IDisposable
    {
        private readonly Action OnError;
        private readonly VncServerSession session;
        private readonly VNCFramebufferSource framebuffer;
        private bool disposedValue;

        public VNCServer(Action onerr)
        {
            this.OnError = onerr;

            // When run over serial we are only able to transmit around 1kb/s 
            framebuffer = new VNCFramebufferSource(OnFrameUpdate, 1);

            var loggerFactory = new LoggerFactory();
            var log = loggerFactory.CreateLogger("vnc");

            // Create a session.
            session = new VncServerSession(this, log);
            session.SetFramebufferSource(framebuffer);
#pragma warning disable CS8622 // Nullability of reference types in type of parameter doesn't match the target delegate (possibly because of nullability attributes).
            session.ConnectionFailed += HandleConnectionFailed;
            session.Closed += HandleClosed;
#pragma warning restore CS8622 // Nullability of reference types in type of parameter doesn't match the target delegate (possibly because of nullability attributes).

            session.Encoder = new TightEncoder(session)
            {
                Compression = TightCompression.Jpeg,
            };
        }

        private void OnFrameUpdate()
        {
            // There seems to be a bug in either the RemoteViewing.Vnc library or NoVNC
            // Basically the pixel format that is passed from NoVNC results in JPEGs that are black
            // Apart from some colour shift values being different the format is the same as the default
            // So to mitigate I force the default here every time we want to make a frame
            session.ClientPixelFormat = new VncPixelFormat();
            session.MaxUpdateRate = 0.3;
        }

        public void Start(Stream stream)
        {
            // Set up a framebuffer and options.
            var options = new VncServerSessionOptions
            {
                AuthenticationMethod = AuthenticationMethod.None
            };

            framebuffer.Reset();

            // wait for client and server to set up connection
            session.Connect(stream, options);
        }

        private void HandleConnectionFailed(object sender, EventArgs e)
        {
            OnError();
        }

        private void HandleClosed(object sender, EventArgs e)
        {
            OnError();
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    OnError();
                    framebuffer.Dispose();
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

        public byte[] GenerateChallenge()
        {
            throw new NotImplementedException();
        }

        public void GetChallengeResponse(byte[] challenge, char[] password, byte[] response)
        {
            throw new NotImplementedException();
        }

        public void GetChallengeResponse(byte[] challenge, byte[] password, byte[] response)
        {
            throw new NotImplementedException();
        }
    }
}
