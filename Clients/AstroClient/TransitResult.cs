namespace AstroClient;

public sealed record TransitResult(
    string Constellation,
    double Latitude,
    double Longitude,
    DateTimeOffset TransitTimeUtc,
    double MaxAltitudeDeg,
    string Visibility);
