namespace AstroCampPlanner;

internal static class EnvironmentPaths
{
    public static string Resolve(string variableName, string defaultValue)
    {
        string? value = Environment.GetEnvironmentVariable(variableName);
        return string.IsNullOrWhiteSpace(value) ? defaultValue : value;
    }
}
