namespace AstroClient;

public sealed class AstroClientException : Exception
{
    public int? HttpStatusCode { get; }

    public AstroClientException(string message, int? httpStatusCode = null) : base(message)
    {
        HttpStatusCode = httpStatusCode;
    }
}
