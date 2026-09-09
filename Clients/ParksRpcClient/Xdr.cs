using System.Buffers.Binary;
using System.Text;

namespace ParksRpcClient;

internal sealed class XdrWriter
{
    private readonly MemoryStream _buf = new();

    public void WriteUInt(uint v)
    {
        Span<byte> b = stackalloc byte[4];
        BinaryPrimitives.WriteUInt32BigEndian(b, v);
        _buf.Write(b);
    }

    public void WriteDouble(double v)
    {
        Span<byte> b = stackalloc byte[8];
        BinaryPrimitives.WriteInt64BigEndian(b, BitConverter.DoubleToInt64Bits(v));
        _buf.Write(b);
    }

    public void WriteString(string s)
    {
        byte[] bytes = Encoding.ASCII.GetBytes(s);
        WriteUInt((uint)bytes.Length);
        _buf.Write(bytes);
        int pad = (4 - bytes.Length % 4) % 4;
        for (int i = 0; i < pad; i++) _buf.WriteByte(0);
    }

    public byte[] ToArray() => _buf.ToArray();
}

internal sealed class XdrReader
{
    private readonly byte[] _data;
    private int _pos;

    public XdrReader(byte[] data) => _data = data;

    public int Position => _pos;

    public uint ReadUInt()
    {
        uint v = BinaryPrimitives.ReadUInt32BigEndian(_data.AsSpan(_pos, 4));
        _pos += 4;
        return v;
    }

    public bool ReadBool() => ReadUInt() != 0;

    public double ReadDouble()
    {
        long bits = BinaryPrimitives.ReadInt64BigEndian(_data.AsSpan(_pos, 8));
        _pos += 8;
        return BitConverter.Int64BitsToDouble(bits);
    }

    public string ReadString()
    {
        int len = (int)ReadUInt();
        string s = Encoding.ASCII.GetString(_data, _pos, len);
        _pos += len;
        _pos += (4 - len % 4) % 4;
        return s;
    }

    public void SkipOpaqueVar()
    {
        int len = (int)ReadUInt();
        _pos += len + (4 - len % 4) % 4;
    }
}
