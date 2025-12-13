@attached(member, names: named(offset))
public macro component() = #externalMacro(
    module: "PlaygroundScriptingMacros",
    type: "ComponentMacro"
)
