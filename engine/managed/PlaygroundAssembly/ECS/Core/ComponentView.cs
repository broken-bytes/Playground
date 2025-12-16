namespace PlaygroundAssembly.ECS.Core;

public interface ComponentView
{
    int Length { get; }
}

public sealed class ComponentView<T> : ComponentView
{
    private readonly T[] _data;

    public int Length => _data.Length;

    public ref T this[int index] => ref _data[index];

    internal ComponentView(T[] data)
    {
        _data = data;
    }
}
