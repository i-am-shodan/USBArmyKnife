#if DEBUG
using Agent.TLV;
using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;

namespace Agent.LocalTesting
{
    internal class TcpServer
    {
        public static async Task Start(int port)
        {
            Executer executer = new();

            TcpListener tcpListener = new TcpListener(IPAddress.Loopback, port);
            tcpListener.Start();

            CancellationTokenSource? currentCts = null;

            while (true)
            {
                var client = await tcpListener.AcceptTcpClientAsync();

                _ = Task.Run(async () =>
                {
                    if (currentCts != null)
                    {
                        currentCts.Cancel();
                        currentCts = null;
                    }

                    BlockingCollection<byte[]> tlvEncodedBlocks = new();
                    using (var tlvBuffer = new MemoryStream())
                    using (var cts = new CancellationTokenSource())
                    {
                        currentCts = cts;

                        try
                        {
                            await TLV.TLVHandling.WriteTLVToStream((byte)Executer.Command.WSCONNECT, new byte[1], 0, 1, tlvBuffer, cts.Token);
                            tlvEncodedBlocks.Add(tlvBuffer.ToArray());
                            tlvBuffer.SetLength(0);

                            using (var transportStream = new TransportStream(cts.Token,
                                (count, token) =>
                                {
                                    using (var cts2 = CancellationTokenSource.CreateLinkedTokenSource(cts.Token, token))
                                    {
                                        return tlvEncodedBlocks.Take(cts2.Token);
                                    }
                                },
                                async (buffer) =>
                                {
                                    // this data is TLV encoded, we need to undo this
                                    if (buffer[0] == (byte)Executer.Command.WSDATARECV)
                                    {
                                        await client.GetStream().WriteAsync(buffer, 4 + 1, buffer.Length - (4 + 1), cts.Token);
                                        Console.WriteLine($"Write {buffer.Length - (4 + 1)} bytes to TCP client");
                                    }
                                }))
                            {
                                executer.ParseAndExecute(transportStream, cts);

                                while (client.Connected && !cts.IsCancellationRequested)
                                {
                                    if (client.Available > 0)
                                    {
                                        var buffer = new byte[client.Available];
                                        var read = await client.GetStream().ReadAsync(buffer);

                                        if (read > 0)
                                        {
                                            tlvBuffer.SetLength(0);
                                            await TLVHandling.WriteTLVToStream((byte)Executer.Command.WSDATA, buffer, 0, read, tlvBuffer, cts.Token);

                                            tlvEncodedBlocks.Add(tlvBuffer.ToArray());
                                            executer.ParseAndExecute(transportStream, cts);
                                        }
                                    }
                                    else
                                    {
                                        await Task.Delay(100);
                                    }
                                }

                                cts.Cancel();
                            }
                        }
                        catch (Exception)
                        {
                            cts.Cancel();
                        }
                    }

                    client.Close();
                    currentCts = null;
                });
            }
        }
    }
}
#endif