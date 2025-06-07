import SwiftCompilerPlugin
import SwiftSyntax
import SwiftSyntaxBuilder
import SwiftSyntaxMacros
import Foundation

@main
struct ScriptingPlugin: CompilerPlugin {
    let providingMacros: [Macro.Type] = [
        ComponentMacro.self
    ]
}
