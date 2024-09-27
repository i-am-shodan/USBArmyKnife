#if DEBUG
using Agent;
using Agent.TLV;
using System.Collections.Concurrent;
using System.Text;
using WebSocketSharp;
using WebSocketSharp.Net;
using WebSocketSharp.Server;

namespace Agent.LocalTesting
{
    public class NoVNCWebSocket : WebSocketBehavior
    {
        private static Executer executer = new();
        private static NoVNCWebSocket? self = null;

        private BlockingCollection<byte[]> tlvEncodedBlocks = new();
        private MemoryStream tlvBuffer = new MemoryStream();
        private CancellationTokenSource cts = new CancellationTokenSource();
        private TransportStream transportStream;

        public NoVNCWebSocket()
        {
            self = this;
            transportStream = new TransportStream(cts.Token,
            (count, token) =>
            {
                var block = tlvEncodedBlocks.Take(cts.Token);
                Console.WriteLine("Read block");
                return block;
            },
            (buffer) =>
            {
                // this data is TLV encoded, we need to undo this
                if (buffer[0] == (byte)Executer.Command.WSDATARECV)
                {
                    byte[] nonTlvData = new byte[buffer.Length - (4 + 1)];
                    Buffer.BlockCopy(buffer, 4 + 1, nonTlvData, 0, buffer.Length - (4 + 1));

                    self?.Send(nonTlvData);
                    Console.WriteLine($"Wrote {nonTlvData.Length} bytes to web socket");
                }
                return Task.CompletedTask;
            });
        }


        protected override void OnClose(CloseEventArgs e)
        {
            Console.WriteLine("Web socket close");
            cts.Cancel();
        }

        protected override void OnOpen()
        {
            Console.WriteLine("Web socket open");

            TLVHandling.WriteTLVToStream((byte)Executer.Command.WSCONNECT, new byte[1], 0, 1, tlvBuffer, cts.Token).Wait();
            tlvEncodedBlocks.Add(tlvBuffer.ToArray());
            tlvBuffer.SetLength(0);

            _ = Task.Run(async () =>
            {
                while (true)
                {
                    executer.ParseAndExecute(transportStream, cts);
                    await Task.Delay(100);
                }
            });
        }

        protected override void OnMessage(MessageEventArgs e)
        {
            Console.WriteLine($"Web socket data {e.RawData.Length} - {e.IsBinary}");

            TLVHandling.WriteTLVToStream((byte)Executer.Command.WSDATA, e.RawData, 0, e.RawData.Length, tlvBuffer, cts.Token).Wait();
            tlvEncodedBlocks.Add(tlvBuffer.ToArray());
            tlvBuffer.SetLength(0);

            tlvEncodedBlocks.Add(tlvBuffer.ToArray());
        }
    }


    public class WebServer
    {
        public static string GetMimeType(string file)
        {
            if (file.EndsWith(".html"))
            {
                return "text/html";
            }
            else if (file.EndsWith(".svg"))
            {
                return "image/svg+xml";
            }
            else if (file.EndsWith(".png"))
            {
                return "image/png";
            }
            else if (file.EndsWith(".gif"))
            {
                return "image/gif";
            }
            else if (file.EndsWith(".mp3"))
            {
                return "image/png";
            }
            else if (file.EndsWith(".ogg"))
            {
                return "video/ogg";
            }
            else if (file.EndsWith(".ico"))
            {
                return "image/x-icon";
            }
            else if (file.EndsWith(".js"))
            {
                return "text/javascript";
            }
            else if (file.EndsWith(".css"))
            {
                return "text/css";
            }
            else if (file.EndsWith(".ttf"))
            {
                return "application/octet-stream";
            }
            else
            {
                return "text/plain";
            }
        }

        public static void Start(int port, string docBase)
        {
            var httpsv = new HttpServer(80);
            // Set the document root path.
            httpsv.DocumentRootPath = "D:\\Repos\\GitHub\\MyUSBTest\\USBArmyKnife\\ui\\vnc";

            // Set the HTTP GET request event.
            httpsv.OnGet += (sender, e) =>
            {
                var req = e.Request;
                var res = e.Response;

                var path = req.RawUrl;

                if (path == "/")
                    path += "index.html";

                byte[] contents;

                if (!e.TryReadFile(path, out contents))
                {
                    res.StatusCode = (int)HttpStatusCode.NotFound;

                    return;
                }

                res.ContentType = GetMimeType(path);
                res.ContentEncoding = Encoding.UTF8;

                res.ContentLength64 = contents.LongLength;

                res.Close(contents, true);
            };

            // Add the WebSocket services.
            httpsv.AddWebSocketService<NoVNCWebSocket>("/websockify");

            httpsv.Start();

            if (httpsv.IsListening)
            {
                Console.WriteLine("Listening on port {0}, and providing WebSocket services:", httpsv.Port);

                foreach (var path in httpsv.WebSocketServices.Paths)
                    Console.WriteLine("- {0}", path);
            }

            Console.WriteLine("\nPress Enter key to stop the server...");
            Console.ReadLine();

            httpsv.Stop();
        }
    }
}
#endif