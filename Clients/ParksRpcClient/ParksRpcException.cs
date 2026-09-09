namespace ParksRpcClient;

public sealed class ParksRpcException : Exception
{
    public ParksRpcException(string message) : base(message) { }
}
