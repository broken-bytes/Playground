using System;

namespace PlaygroundEditor;

[Serializable]
public struct Project
{
    public string Name;
    public string Version;
    public DateTime CreationDate;
    public string GUID;
}
