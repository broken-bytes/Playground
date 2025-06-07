import SwiftSyntax
import SwiftSyntaxBuilder
import SwiftSyntaxMacros
import SwiftParser

public struct ComponentMacro: MemberMacro, ExtensionMacro  {
    public static func expansion(
        of node: AttributeSyntax,
        providingMembersOf declaration: some DeclGroupSyntax,
        in context: some MacroExpansionContext
    ) throws -> [DeclSyntax] {
        guard let classDecl = declaration.as(ClassDeclSyntax.self) else { return [] }

        let fields = classDecl.memberBlock.members.compactMap { member -> (name: String, type: String)? in
            guard
                let varDecl = member.decl.as(VariableDeclSyntax.self),
                let binding = varDecl.bindings.first,
                let identifier = binding.pattern.as(IdentifierPatternSyntax.self),
                let type = binding.typeAnnotation?.type.description
            else { return nil }

            return (identifier.identifier.text, type.trimmingCharacters(in: .whitespacesAndNewlines))
        }

        let setCases = fields.map { name, type in
            "case \"\(name)\": self.\(name) = value as! \(type)"
        }.joined(separator: "\n        ")

        let getCases = fields.map { name, _ in
            "case \"\(name)\": return \(name)"
        }.joined(separator: "\n        ")

        let setField = """
        func setField(_ name: String, _ value: Any) {
            switch name {
        \(setCases)
            default: fatalError("Unknown field: \\(name)")
            }
        }
        """

        let getField = """
        func getField(_ name: String) -> Any? {
            switch name {
        \(getCases)
            default: return nil
            }
        }
        """

        return [
            DeclSyntax(stringLiteral: setField),
            DeclSyntax(stringLiteral: getField)
        ]
    }

    public static func expansion(
        of node: AttributeSyntax,
        attachedTo declaration: some DeclGroupSyntax,
        providingExtensionsOf type: some TypeSyntaxProtocol,
        conformingTo protocols: [TypeSyntax],
        in context: some MacroExpansionContext
    ) throws -> [ExtensionDeclSyntax] {
        guard let classDecl = declaration.as(ClassDeclSyntax.self) else { return [] }

        var inheritedTypes = ""

        // Parse macro arguments: @Component(OnUpdate, OnStart)
        if let arguments = node.argument?.as(TupleExprElementListSyntax.self) {
            for arg in arguments {
                if let memberAccess = arg.expression.as(MemberAccessExprSyntax.self) {
                    let caseName = memberAccess.declName.baseName.text
                    let protocolName = "Component.\(uppercasedFirstLetter(caseName))"
                    inheritedTypes += protocolName + ","
                }
            }
        }

        inheritedTypes = String(inheritedTypes.dropLast())

        return [ExtensionDeclSyntax(
            extendedType: type,
            memberBlock: MemberBlockSyntax(": \(raw: inheritedTypes) {}"))
        ]
    }
}

func uppercasedFirstLetter(_ input: String) -> String {
    guard let first = input.first else { return input }
    return first.uppercased() + input.dropFirst()
}
