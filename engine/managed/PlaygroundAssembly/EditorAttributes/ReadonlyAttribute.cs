namespace PlaygroundAssembly.EditorAttributes;

[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public sealed class ReadonlyAttribute()
    : Attribute
{
}
