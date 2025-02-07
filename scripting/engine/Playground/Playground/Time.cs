namespace Playground;

public static class Time
{
    public static double DeltaTime { get; internal set; }
    public static double ElapsedTime { get; internal set; }

    private static DateTime _lastTick;
    
    internal static void OnTick()
    {
        var nextTick = DateTime.Now;
        DeltaTime = (nextTick - _lastTick).TotalMilliseconds;
        _lastTick = nextTick;
    }
}
