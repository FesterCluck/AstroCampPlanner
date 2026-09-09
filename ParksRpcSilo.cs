using ParksRpcClient;

namespace AstroCampPlanner;

internal static class ParksRpcSilo
{
    private const string Host = "127.0.0.1";
    private const int Port = 9090;
    private static readonly string ServerHome = EnvironmentPaths.Resolve(
        "ASTROCAMPPLANNER_PARKS_SERVER_HOME",
        Path.Combine(ProjectLocator.Root, "TestServers", "ParksRpcServer"));
    private static readonly string ServerExecutable = Path.Combine(ServerHome, "rpc_server");
    private static readonly string ServerWorkingDirectory = ServerHome;

    public static IReadOnlyList<Park> ListAllParks()
    {
        EnsureRpcHostUp();
        var clnt = new ParksRpcClient.ParksRpcClient(Host, Port);
        return clnt.ListAllAsync().GetAwaiter().GetResult();
    }

    public static Park GetParkByName(string name)
    {
        EnsureRpcHostUp();
        var clnt = new ParksRpcClient.ParksRpcClient(Host, Port);
        Park? park = clnt.GetByNameAsync(name).GetAwaiter().GetResult();
        if (park is null)
        {
            throw new InvalidOperationException($"RPC: no such park \"{name}\" on remote host {Host}:{Port}");
        }
        return park;
    }

    private static void EnsureRpcHostUp()
    {
        ProcessSupervisor.EnsureListening(Port, ServerExecutable, ServerWorkingDirectory, Port.ToString());
    }
}
