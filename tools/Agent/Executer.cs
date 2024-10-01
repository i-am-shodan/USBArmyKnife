using Agent.TLV;
using Agent.VNC;
using System;
using System.Collections.Concurrent;
using System.Data;
using System.Diagnostics;
using System.Text;

namespace Agent
{
    internal class Executer
    {
        private CancellationTokenSource? currentCts;

        private BlockingCollection<byte[]> vncDataBlocks = new();
        private readonly VNCServer vnc;
        private TransportStream? vncStream = null;

        public enum Command
        {
            Execute = 1,
            DebugMsg = 2,
            WSCONNECT = 3,
            WSDATA = 4,
            WSDISCONNECT = 5,
            WSDATARECV = 6,
            RequestAgentStatus = 7,
            AgentStatus = 8,
            ExecuteResult = 9
        }

        public Executer()
        {
            vnc = new VNCServer(() =>
            {
                if (currentCts != null)
                {
                    try
                    {
                        currentCts.Cancel();
                    }
                    catch (ObjectDisposedException)
                    {

                    }
                }
            });
        }

        public void ParseAndExecute(Stream stream, CancellationTokenSource cts)
        {
            var cmd = (Command)stream.ReadByte();

            byte[] lengthBytes = new byte[4];
            int offset = 0;
            int read = 0;

            while (read + offset < lengthBytes.Length)
            {
                read = stream.Read(lengthBytes, offset, lengthBytes.Length);
            }
            var length = BitConverter.ToUInt32(lengthBytes, 0);
            var data = new byte[length];

            if (length != 0)
            {
                offset = 0;
                read = 0;

                while (read + offset < data.Length)
                {
                    read = stream.Read(data, offset, data.Length);
                }
            }

            HandleTLV(cmd, data, stream, cts);
        }

        private static async Task<string> Run(string command, string arguments, CancellationToken token = default)
        {
            try
            {
                var startInfo = new ProcessStartInfo()
                {
                    FileName = command,
                    Arguments = arguments,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                using (var process = new Process() { StartInfo = startInfo })
                {
                    // Capture the output and error streams
                    StringBuilder output = new StringBuilder();
                    StringBuilder error = new StringBuilder();

                    process.OutputDataReceived += (sender, e) => { if (e.Data != null) output.AppendLine(e.Data); };
                    process.ErrorDataReceived += (sender, e) => { if (e.Data != null) error.AppendLine(e.Data); };

                    process.Start();
                    process.BeginOutputReadLine();
                    process.BeginErrorReadLine();

                    await process.WaitForExitAsync(token);

                    return output.ToString() + error.ToString();
                }
            }
            catch (Exception ex)
            {
                return ex.Message;
            }
        }

        private void RunCommand(string command, Stream stream, CancellationToken token = default)
        {
            _ = Task.Run(async () =>
            {
#pragma warning disable CS0168 // Variable is declared but never used
                try
                {
                    var output = await Run("cmd.exe", "/c "+command, token);
                    var buffer = Encoding.UTF8.GetBytes(output);

                    const int MAX_BUFFER_LENGTH = 8192;

                    // To avoid running the device out of memory we cap the returned data at 8k
                    buffer = buffer.Take(MAX_BUFFER_LENGTH).ToArray();

                    int dataSent = 0;
                    while (dataSent < buffer.Length)
                    {
                        var amountOfDataToSend = buffer.Length - dataSent > 2048 ? 2048 : buffer.Length - dataSent;
                        await TLVHandling.WriteTLVToStream((byte)Command.ExecuteResult, buffer, dataSent, amountOfDataToSend, stream, token);
                        await Task.Delay(50); // processing time
                        dataSent += amountOfDataToSend;
#if DEBUG
                        Console.WriteLine("OUT ExecuteResult)" + amountOfDataToSend);
#endif
                    }
                }
                catch (Exception ex)
                {
#if DEBUG
                    Console.WriteLine("OUT ExecuteResult) " + ex.Message);
#endif
                }
#pragma warning restore CS0168 // Variable is declared but never used
            }, token);
        }

        private void HandleTLV(Command cmd, byte[] data, Stream stream, CancellationTokenSource cts)
        {
            switch (cmd)
            {
                case Command.Execute:
                    var cmdLine = Encoding.UTF8.GetString(data);
#if DEBUG
                    Console.WriteLine("IN Execute) "+cmdLine);
#endif
                    RunCommand(cmdLine, stream, cts.Token);
                    break;
                case Command.WSCONNECT:
#if DEBUG
                    Console.WriteLine("IN WSCONNECT");
#endif

                    if (vncStream != null)
                    {
                        vncStream.Dispose();
                        vncStream = null;
                    }

                    vncDataBlocks = new BlockingCollection<byte[]>();
                    vncStream = new TransportStream(cts.Token, (count, token) =>
                    {
                        try
                        {
                            using (var cts2 = CancellationTokenSource.CreateLinkedTokenSource(cts.Token, token))
                            {
                                while (!cts2.IsCancellationRequested)
                                {
                                    return vncDataBlocks.Take(cts2.Token);
                                }
                            }

                            throw new Exception("Timed out");
                        }
                        catch
                        {
                            try
                            {
                                cts.Cancel();
                            }
                            catch (ObjectDisposedException)
                            {

                            }
                            throw;
                        }
                    },
                    async (buffer) =>
                    {
#pragma warning disable CS0168 // Variable is declared but never used
                        try
                        {
                            int dataSent = 0;
                            while (dataSent < buffer.Length)
                            {
                                var amountOfDataToSend = buffer.Length - dataSent > 2048 ? 2048 : buffer.Length - dataSent;
                                await TLVHandling.WriteTLVToStream((byte)Command.WSDATARECV, buffer, dataSent, amountOfDataToSend, stream, cts.Token);
                                await Task.Delay(100); // processing time to ensure we can push the buffer out over WiFi before we get another one
                                dataSent += amountOfDataToSend;
#if DEBUG
                                Console.WriteLine("OUT WSDATARECV)" + amountOfDataToSend);
#endif
                            }

#if DEBUG
                            Console.WriteLine("OUT WSDATARECV) all sent");
#endif
                        }
                        catch (Exception ex)
                        {
#if DEBUG
                            Console.WriteLine("OUT WSDATARECV) " + ex.Message);
#endif
                            cts.Cancel();
                            throw;
                        }
#pragma warning restore CS0168 // Variable is declared but never used
                    });
                    currentCts = cts;
                    vnc.Start(vncStream);
                    break;
                case Command.WSDISCONNECT:
                    if (vncStream != null)
                    {
                        vncStream.Dispose();
                        vncStream = null;
                    }
#if DEBUG
                    Console.WriteLine("IN WSDISCONNECT)");
#endif
                    break;
                case Command.WSDATA:
#if DEBUG
                    Console.WriteLine("IN WSDATA) "+data.Length);
#endif
                    vncDataBlocks.Add(data);
                    break;
                case Command.DebugMsg:
#if DEBUG
                    Console.WriteLine("IN DebugMsg) "+ Encoding.UTF8.GetString(data));
#endif
                    break;
                case Command.RequestAgentStatus:
                    using (var ms = new MemoryStream())
                    {
                        var machineName = Encoding.UTF8.GetBytes(Environment.MachineName);
                        TLVHandling.WriteTLVToStream((byte)Command.AgentStatus, machineName, 0, machineName.Length, stream, cts.Token).Wait();
                    }
                    break;
                default:
                    throw new InvalidDataException("unknown command");
            }
        }
    }
}
