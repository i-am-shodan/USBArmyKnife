using System.Collections.Concurrent;

namespace Agent
{
    internal class TransportStream : Stream, IDisposable
    {
        private readonly CancellationTokenSource cts;
        private readonly MemoryStream DataToStreamOwner = new();
        private readonly ConcurrentQueue<byte[]> blocksToWrite = new();

        private readonly Func<int, CancellationToken, byte[]> ReadBlock;
        private readonly Func<byte[], Task> WriteBlock;

        private readonly Task Writer;
        private bool _Disposed;

        public TransportStream(CancellationToken token, Func<int, CancellationToken, byte[]> readBlock, Func<byte[], Task> writeBlock)
        {
            ReadBlock = readBlock;
            WriteBlock = writeBlock;

            cts = CancellationTokenSource.CreateLinkedTokenSource(token);

            Writer = Task.Run(async () =>
            {
                while (!cts.IsCancellationRequested)
                {
                    try
                    {
                        if (!blocksToWrite.Any())
                        {
                            await Task.Delay(100, cts.Token);
                            continue;
                        }

                        var maxBuffer = blocksToWrite.Sum(x => x.Length);

                        var finalBuffer = new byte[maxBuffer];
                        var totalWritten = 0;
                        while (totalWritten < maxBuffer)
                        {
                            if (blocksToWrite.TryDequeue(out byte[]? buffer))
                            {
                                Buffer.BlockCopy(buffer, 0, finalBuffer, totalWritten, buffer.Length);
                                totalWritten += buffer.Length;
                            }
                        }
                        await WriteBlock(finalBuffer);
                    }
                    catch (Exception ex)
                    {
#if DEBUG
                        Console.WriteLine("Error in transport stream: "+ex.Message);
#endif
                        Console.WriteLine("Error in transport stream: "+ex.Message);
                        throw;
                    }
                }
            }, cts.Token);
        }

        public override bool CanRead => true;

        public override bool CanSeek => throw new NotImplementedException();

        public override bool CanWrite => true;

        public override long Length => DataToStreamOwner.Length;

        public override long Position { get => DataToStreamOwner.Position; set => throw new NotImplementedException(); }

        public override void Flush()
        {
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            var maxRead = DataToStreamOwner.Length - DataToStreamOwner.Position;
            while (count > maxRead)
            {
                var block = ReadBlock((int)(count - maxRead), cts.Token);
                DataToStreamOwner.Write(block);
                DataToStreamOwner.Position -= block.Length;
                maxRead = DataToStreamOwner.Length - DataToStreamOwner.Position;
            }

            return DataToStreamOwner.Read(buffer, offset, count);
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new NotImplementedException();
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException();
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            byte[] toAdd = new byte[count];
            Buffer.BlockCopy(buffer, offset, toAdd, 0, count);

            blocksToWrite.Enqueue(toAdd);
        }

        public override Task WriteAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        {
            return Task.Run(() => Write(buffer, offset, count), cancellationToken);
        }

        ~TransportStream()
        {
            Dispose(false);
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (!_Disposed)
            {
                cts.Cancel();
                cts.Dispose();
                DataToStreamOwner.Dispose();
                _Disposed = true;
            }
        }
    }
}
