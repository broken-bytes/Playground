namespace PlaygroundAssembly.ECS.Core;

public struct EcsFilter
{
    public enum Usage: short
    {
        Read = 3,
        Write = 4,
        ReadWrite = 5
    }

    public enum Operation : short
    {
        And = 0,
        Or = 1,
        Not = 2
    }

    public ulong Id;
    public Usage ParamUsage;
    public Operation ParamOperation;
}
