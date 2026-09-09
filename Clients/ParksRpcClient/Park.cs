namespace ParksRpcClient;

public sealed record Park(
    uint Id,
    string Name,
    string State,
    double Latitude,
    double Longitude,
    uint EstablishedYear,
    uint AreaAcres);
