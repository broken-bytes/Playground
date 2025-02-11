using System.Diagnostics;

namespace Playground;

internal static class ResourceManager
{
    internal static string _path;
    
    static ResourceManager()
    {
        var processPath = Environment.ProcessPath;

        if (processPath == null)
        {
            throw new NullReferenceException("The process path cannot be null.");
        }
        
        _path = Path.Combine(processPath, "Resources");
    }
}
