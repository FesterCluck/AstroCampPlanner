using System.Diagnostics;

namespace AstroCampPlanner;

internal enum AuthOutcome
{
    Approved,
    Declined,
    RejectedLocally,
    NoCarrier,
    Error,
}

internal readonly struct AuthResult
{
    public AuthResult(AuthOutcome outcome, string transcript, int exitCode)
    {
        Outcome = outcome;
        Transcript = transcript;
        ExitCode = exitCode;
    }

    public AuthOutcome Outcome { get; }
    public string Transcript { get; }
    public int ExitCode { get; }
}

internal static class DialupPaymentSilo
{
    private static readonly string ServerHome = EnvironmentPaths.Resolve(
        "ASTROCAMPPLANNER_DIALUP_SERVER_HOME",
        Path.Combine(ProjectLocator.Root, "TestServers", "DialupServer"));
    private static readonly string ClientHome = EnvironmentPaths.Resolve(
        "ASTROCAMPPLANNER_DIALUP_CLIENT_HOME",
        Path.Combine(ProjectLocator.Root, "Clients", "DialupClient"));
    private static readonly string ServerExecutable = Path.Combine(ServerHome, "server");
    private static readonly string ClientExecutable = Path.Combine(ClientHome, "client");
    private static readonly string ModemPortPath = Path.Combine(ServerHome, "campplanner_modem_port");

    public static AuthResult ChargeCard(string card, string expiry, string cvv, string amount)
    {
        EnsureModemBankIsRunning();

        var startInfo = new ProcessStartInfo(ClientExecutable)
        {
            WorkingDirectory = ClientHome,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        startInfo.ArgumentList.Add("--port");
        startInfo.ArgumentList.Add(ModemPortPath);
        startInfo.ArgumentList.Add("--amount");
        startInfo.ArgumentList.Add(amount);
        startInfo.ArgumentList.Add(card);
        startInfo.ArgumentList.Add(expiry);
        startInfo.ArgumentList.Add(cvv);

        using Process dialer = Process.Start(startInfo)!;
        string transcript = dialer.StandardOutput.ReadToEnd();
        dialer.WaitForExit();

        AuthOutcome outcome = dialer.ExitCode switch
        {
            0 => AuthOutcome.Approved,
            1 => AuthOutcome.Declined,
            2 => AuthOutcome.RejectedLocally,
            3 => AuthOutcome.NoCarrier,
            _ => AuthOutcome.Error,
        };
        return new AuthResult(outcome, transcript, dialer.ExitCode);
    }

    private static void EnsureModemBankIsRunning()
    {
        if (File.Exists(ModemPortPath)) return;

        var startInfo = new ProcessStartInfo(ServerExecutable)
        {
            WorkingDirectory = ServerHome,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        startInfo.ArgumentList.Add("--symlink");
        startInfo.ArgumentList.Add(ModemPortPath);
        Process.Start(startInfo);

        for (int i = 0; i < 50 && !File.Exists(ModemPortPath); i++)
        {
            Thread.Sleep(100);
        }
    }
}
