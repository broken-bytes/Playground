namespace PlaygroundAssembly.Assembly;

internal static class GameAssemblyLoader
{
    internal static void Load(string path)
    {
        var assembly = System.Reflection.Assembly.LoadFrom(path);
    }
}
