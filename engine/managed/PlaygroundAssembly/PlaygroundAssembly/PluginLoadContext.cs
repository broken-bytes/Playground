using System.Reflection;
using System.Runtime.Loader;

namespace PlaygroundAssembly;

public class PluginLoadContext(string basePath) : AssemblyLoadContext(isCollectible: true)
{
    protected override System.Reflection.Assembly? Load(AssemblyName assemblyName)
    {
        if (assemblyName.Name is "PlaygroundAssembly")
        {
            return null;
        }
        
        if (assemblyName.Name == "Playground.Core")
        {
            return System.Reflection.Assembly.Load(assemblyName);
        }
        
        string assemblyPath = Path.Combine(
            basePath,
            $"{assemblyName.Name}.dll");
        
        if (File.Exists(assemblyPath))
        {
            return LoadFromAssemblyPath(assemblyPath);
        }

        return null;
    }
}
