namespace Playground.Core.Logging;

public static class Logger
{
    public static void Info(string message)
    {
        LoggerApi.LogInfo(message);
    }
    
    public static void Warn(string message)
    {
        LoggerApi.LogWarning(message);
    }
    
    public static void Error(string message)
    {
        LoggerApi.LogError(message);
    }
}
