using AstroClient;

namespace AstroCampPlanner;

internal static class AstroRestSilo
{
    private const int Port = 8070;
    private const string BaseUrl = "http://127.0.0.1:8070";
    private static readonly string ServerHome = EnvironmentPaths.Resolve(
        "ASTROCAMPPLANNER_ASTRO_SERVER_HOME",
        Path.Combine(ProjectLocator.Root, "TestServers", "AstroTransitServer"));
    private static readonly string ServerExecutable = Path.Combine(ServerHome, "astro_server");
    private static readonly string ServerWorkingDirectory = ServerHome;

    public static readonly IReadOnlyList<string> Constellations = new[]
    {
        "Orion", "Ursa Major", "Ursa Minor", "Cassiopeia", "Scorpius",
        "Sagittarius", "Leo", "Taurus", "Gemini", "Cygnus", "Lyra",
        "Aquila", "Crux", "Canis Major", "Pegasus",
    };

    public static async Task<TransitResult> GetTransitAsync(double latitude, double longitude, string constellation)
    {
        ProcessSupervisor.EnsureListening(Port, ServerExecutable, ServerWorkingDirectory, Port.ToString());

        using var client = new AstroTransitClient(BaseUrl);
        return await client.GetTransitAsync(latitude, longitude, constellation);
    }
}
