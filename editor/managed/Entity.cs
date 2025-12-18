using System;

namespace PlaygroundEditor;

public class Entity {
    public UInt64 Id { get; set; }
    public string Name { get; set; }
    public Entity? Parent { get; set; }

    public Entity(UInt64 id, string name, Entity? parent) {
        Id = id;
        Name = name;
        Parent = parent;
    }
}
