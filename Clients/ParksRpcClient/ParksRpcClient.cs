using System.Buffers.Binary;
using System.Net.Sockets;

namespace ParksRpcClient;

public sealed class ParksRpcClient
{
    private const uint Prog = 0x20000001;
    private const uint Vers = 1;
    private const uint ProcGetByName = 1;
    private const uint ProcFindNearest = 2;
    private const uint ProcListAll = 3;

    private const uint MsgTypeCall = 0;
    private const uint MsgTypeReply = 1;
    private const uint ReplyStatAccepted = 0;
    private const uint AcceptStatSuccess = 0;

    private readonly string _host;
    private readonly int _port;
    private static int _nextXid;

    public ParksRpcClient(string host, int port)
    {
        _host = host;
        _port = port;
    }

    public async Task<Park?> GetByNameAsync(string name, CancellationToken ct = default)
    {
        var args = new XdrWriter();
        args.WriteString(name);
        byte[] result = await CallAsync(ProcGetByName, args.ToArray(), ct);

        var r = new XdrReader(result);
        return r.ReadBool() ? ReadPark(r) : null;
    }

    public async Task<(Park Park, double DistanceMiles)> FindNearestAsync(double latitude, double longitude, CancellationToken ct = default)
    {
        var args = new XdrWriter();
        args.WriteDouble(latitude);
        args.WriteDouble(longitude);
        byte[] result = await CallAsync(ProcFindNearest, args.ToArray(), ct);

        var r = new XdrReader(result);
        Park park = ReadPark(r);
        double distance = r.ReadDouble();
        return (park, distance);
    }

    public async Task<IReadOnlyList<Park>> ListAllAsync(CancellationToken ct = default)
    {
        byte[] result = await CallAsync(ProcListAll, Array.Empty<byte>(), ct);

        var r = new XdrReader(result);
        uint count = r.ReadUInt();
        var list = new List<Park>((int)count);
        for (int i = 0; i < count; i++) list.Add(ReadPark(r));
        return list;
    }

    private static Park ReadPark(XdrReader r) => new(
        Id: r.ReadUInt(),
        Name: r.ReadString(),
        State: r.ReadString(),
        Latitude: r.ReadDouble(),
        Longitude: r.ReadDouble(),
        EstablishedYear: r.ReadUInt(),
        AreaAcres: r.ReadUInt());

    private async Task<byte[]> CallAsync(uint proc, byte[] argBytes, CancellationToken ct)
    {
        uint xid = (uint)Interlocked.Increment(ref _nextXid);

        var header = new XdrWriter();
        header.WriteUInt(xid);
        header.WriteUInt(MsgTypeCall);
        header.WriteUInt(2);
        header.WriteUInt(Prog);
        header.WriteUInt(Vers);
        header.WriteUInt(proc);
        header.WriteUInt(0); header.WriteUInt(0);
        header.WriteUInt(0); header.WriteUInt(0);

        byte[] headerBytes = header.ToArray();
        byte[] message = new byte[headerBytes.Length + argBytes.Length];
        Buffer.BlockCopy(headerBytes, 0, message, 0, headerBytes.Length);
        Buffer.BlockCopy(argBytes, 0, message, headerBytes.Length, argBytes.Length);

        using var tcp = new TcpClient();
        await tcp.ConnectAsync(_host, _port, ct);
        using NetworkStream stream = tcp.GetStream();

        byte[] marker = new byte[4];
        BinaryPrimitives.WriteUInt32BigEndian(marker, 0x80000000u | (uint)message.Length);
        await stream.WriteAsync(marker, ct);
        await stream.WriteAsync(message, ct);

        byte[] replyMarker = await ReadExactAsync(stream, 4, ct);
        uint markerValue = BinaryPrimitives.ReadUInt32BigEndian(replyMarker);
        bool isLastFragment = (markerValue & 0x80000000u) != 0;
        int fragmentLen = (int)(markerValue & 0x7FFFFFFFu);
        if (!isLastFragment) throw new ParksRpcException("multi-fragment RPC replies aren't supported");

        byte[] fragment = await ReadExactAsync(stream, fragmentLen, ct);
        var reply = new XdrReader(fragment);

        uint replyXid = reply.ReadUInt();
        if (replyXid != xid) throw new ParksRpcException($"RPC reply xid mismatch: expected {xid}, got {replyXid}");

        uint msgType = reply.ReadUInt();
        if (msgType != MsgTypeReply) throw new ParksRpcException($"expected a REPLY message, got msg_type {msgType}");

        uint replyStat = reply.ReadUInt();
        if (replyStat != ReplyStatAccepted) throw new ParksRpcException($"RPC call denied (reply_stat={replyStat})");

        reply.ReadUInt();
        reply.SkipOpaqueVar();

        uint acceptStat = reply.ReadUInt();
        if (acceptStat != AcceptStatSuccess) throw new ParksRpcException($"RPC call not successful (accept_stat={acceptStat})");

        return fragment[reply.Position..];
    }

    private static async Task<byte[]> ReadExactAsync(NetworkStream stream, int count, CancellationToken ct)
    {
        byte[] buf = new byte[count];
        int offset = 0;
        while (offset < count)
        {
            int read = await stream.ReadAsync(buf.AsMemory(offset, count - offset), ct);
            if (read == 0) throw new ParksRpcException("connection closed while reading an RPC reply");
            offset += read;
        }
        return buf;
    }
}
