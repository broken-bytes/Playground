using Playground.Core.Assets;
using Playground.Core.Logging;

namespace PlaygroundAssembly.Scenes;

public static class SceneHandler
{
    public static async Task LoadScene(string name)
    {
        var sceneInfo = AssetApi.LoadScene(name);
            
        Logger.Info($"Loaded scene: {sceneInfo}");
        
        var sceneData = SceneParser.Parse(sceneInfo);
        Logger.Info(sceneData.ToString());
        
        await Task.CompletedTask;
    }
}
