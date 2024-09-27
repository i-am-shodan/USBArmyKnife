namespace Agent.TLV
{
    internal class TLVHandling
    {
        public static async Task<int> WriteTLVToStream(byte tag, byte[] data, int offset, int count, Stream output, CancellationToken token = default)
        {
            var bufferToSend = new byte[count + 4 + 1];
            bufferToSend[0] = tag; // tag
            var lenBytes = BitConverter.GetBytes(count);
            Buffer.BlockCopy(lenBytes, 0, bufferToSend, 1, lenBytes.Length);
            Buffer.BlockCopy(data, offset, bufferToSend, 1 + lenBytes.Length, count);

            await output.WriteAsync(bufferToSend, 0, bufferToSend.Length, token);
            return bufferToSend.Length;
        }
    }
}
