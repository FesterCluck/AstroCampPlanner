using System.Diagnostics;
using System.Net.Sockets;

namespace AstroCampPlanner;

internal static class ProcessSupervisor
{
    public static void EnsureListening(int port, string executablePath, string workingDirectory, string arg)
    {
        if (IsListening(port)) return;

        Process.Start(new ProcessStartInfo(executablePath)
        {
            ArgumentList = { arg },
            WorkingDirectory = workingDirectory,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        });

        for (int i = 0; i < 50 && !IsListening(port); i++)
        {
            Thread.Sleep(100);
        }
    }

    private static bool IsListening(int port)
    {
        try
        {
            using var client = new TcpClient();
            var connectTask = client.ConnectAsync("127.0.0.1", port);
            return connectTask.Wait(200) && client.Connected;
        }
        catch
        {
            return false;
        }
    }
}
