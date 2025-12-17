namespace Playground.Core.Editor;

[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public sealed class ReadonlyAttribute()
    : Attribute
{
}
