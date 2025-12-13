import Foundation
import SwiftSyntax
import SwiftSyntaxBuilder
import SwiftSyntaxMacros
import SwiftParser

public struct ComponentMacro: MemberMacro  {
    public static func expansion(
        of node: AttributeSyntax,
        providingMembersOf declaration: some DeclGroupSyntax,
        in context: some MacroExpansionContext
    ) throws -> [DeclSyntax] {
        guard let structDecl = declaration.as(StructDeclSyntax.self) else { return [] }

        let fields = structDecl.memberBlock.members.compactMap { member -> (name: String, type: String)? in
            guard
                let varDecl = member.decl.as(VariableDeclSyntax.self),
                let binding = varDecl.bindings.first,
                let identifier = binding.pattern.as(IdentifierPatternSyntax.self),
                let type = binding.typeAnnotation?.type.description
            else { return nil }

            return (identifier.identifier.text, type.trimmingCharacters(in: .whitespacesAndNewlines))
        }

        let jsonFields: [[String: String]] = fields.map { field in
            return [
                "name": String(describing: field.name),
                "type": String(describing: field.type)
            ]
        }

        let jsonObject: [String: Any] = [
            "name": structDecl.identifier.text,
            "fields": jsonFields
        ]

        let isEngineBuild = ProcessInfo.processInfo.environment["ENGINE_BUILD"] == "1"
        let projectName = ProcessInfo.processInfo.environment["PROJECT_NAME"] ?? "Unknown"

        let base = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!

        let outputDir: URL
        if isEngineBuild {
            outputDir = base.appendingPathComponent("BrokenBytes/BlitzEngine/CoreComponents/Reflection")
        } else {
            outputDir = base
                .appendingPathComponent("BrokenBytes/BlitzEngine/Projects")
                .appendingPathComponent(projectName)
                .appendingPathComponent("Reflection")
        }

        try FileManager.default.createDirectory(at: outputDir, withIntermediateDirectories: true)
        let jsonData = try JSONSerialization.data(withJSONObject: jsonObject, options: [.prettyPrinted])

        // Pick a write path — this must be somewhere writable by your build env
        let outputPath = outputDir.appendingPathComponent("\(structDecl.identifier.text).json")

        try jsonData.write(to: outputPath)

         // Generate switch cases for each field
        let switchCases = fields.map { field in
            let name = field.name
            return """
            case "\(name)":
                return base.distance(to: UnsafeRawPointer(&instance.\(name)))
            """
        }.joined(separator: "\n")

        let offsetFunc = try DeclSyntax(stringLiteral:
            """
            static func offset(of field: String) -> Int {
                var raw = [UInt8](repeating: 0, count: MemoryLayout<Self>.size)
                return raw.withUnsafeMutableBytes { rawBuffer in
                    let ptr = rawBuffer.baseAddress!.assumingMemoryBound(to: Self.self)
                    let base = UnsafeRawPointer(ptr)
                    var instance = ptr.pointee

                    switch field {
                        \(switchCases)
                        default:
                            return -1
                    }
                }
            }
            """
        )

        return [offsetFunc]
    }
}

func uppercasedFirstLetter(_ input: String) -> String {
    guard let first = input.first else { return input }
    return first.uppercased() + input.dropFirst()
}
