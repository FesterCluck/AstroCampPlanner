namespace AstroCampPlanner;

internal static class ProjectLocator
{
    private static readonly Lazy<string> RootLazy = new(FindRoot);

    public static string Root => RootLazy.Value;

    private static string FindRoot()
    {
        DirectoryInfo? dir = new DirectoryInfo(AppContext.BaseDirectory);
        while (dir is not null)
        {
            if (File.Exists(Path.Combine(dir.FullName, "AstroCampPlanner.csproj")))
            {
                return dir.FullName;
            }
            dir = dir.Parent;
        }
        throw new InvalidOperationException(
            "Could not locate the AstroCampPlanner project root by walking up from " + AppContext.BaseDirectory +
            ". Set the ASTROCAMPPLANNER_*_HOME environment variables explicitly instead.");
    }
}
