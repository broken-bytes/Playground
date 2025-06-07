@attached(extension, conformances: Component.OnCreate, Component.OnStart, Component.OnEnable, Component.OnUpdate, Component.OnDisable, Component.OnDestroy, names: arbitrary)
@attached(member, names: named(setField), named(getField))
public macro component(_ phases: Component.Lifecycle...) = #externalMacro(
    module: "PlaygroundScriptingMacros",
    type: "ComponentMacro"
)
