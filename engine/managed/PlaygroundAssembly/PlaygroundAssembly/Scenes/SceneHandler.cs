using System.Reflection;
using System.Text.Json;
using Playground.Core.Assets;
using Playground.Core.Logging;
using PlaygroundAssembly.ECS;

namespace PlaygroundAssembly.Scenes;

public static class SceneHandler
{
    public static void LoadScene(string name)
    {
        var sceneInfo = AssetApi.LoadScene(name);
            
        Logger.Info($"Loaded scene: {sceneInfo}");
        
        var sceneData = SceneParser.Parse(sceneInfo);
        Logger.Info("Entities:");
        foreach (var entity in sceneData.Entities)
        {
            Logger.Info($"- {entity.Name}");
        }
        
        ProcessScene(sceneData);
    }

    private static void ProcessScene(Scene scene)
    {
        foreach (var entity in scene.Entities)
        {
            SpawnEntity(entity);
        }
    }
    
    private static void SpawnEntity(SceneEntity entity)
    {
        var ecsEntity = new Entity(entity.Name);
        
        foreach (var comp in entity.Components)
        {
            Logger.Info($"Required assembly: {comp.Type.Assembly}");
            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                Logger.Info($"Available assembly: {assembly.GetName().Name}");
            }
            
            var asm = AppDomain.CurrentDomain.GetAssemblies()
                .First(a => a.GetName().Name == comp.Type.Assembly);

            var type = asm.GetType(
                comp.Type.Name,
                throwOnError: true
            );    
            
            Logger.Info($"Spawned component {type.Name}");
            
            object boxed = Activator.CreateInstance(type)!;
            
            foreach (var (name, jsonValue) in comp.Properties)
            {
                var field = type.GetField(
                    name,
                    BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance
                );

                if (field == null)
                {
                    continue;
                }

                object? value = JsonSerializer.Deserialize(
                    jsonValue.GetRawText(),
                    field.FieldType
                );

                field.SetValue(boxed, value);
            }

            ecsEntity.With(type, boxed);
        }
    }
}
