using System;
using System.IO;
using System.Xml.Serialization;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;

namespace PlaygroundEditor;

public partial class App : Application
{
    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            desktop.MainWindow = new MainWindow();
        }                                        

        base.OnFrameworkInitializationCompleted();

        var args = System.Environment.GetCommandLineArgs();
        
        // Second arg is the project path, check for project.pproj
        if (args.Length <= 1)
        {
            throw new ArgumentException("Invalid command line arguments, expected project path");
        }
        
        var projectPath = args[1] + "/project.pproj";

        if (File.Exists(projectPath))
        {
            // Parse the file via xml reader
            var deserializer = new XmlSerializer(typeof(Project));
            using Stream reader = new FileStream(projectPath, FileMode.Open);
            // Call the Deserialize method to restore the object's state.
            var project = (Project?)deserializer.Deserialize(reader);

            if (project == null)
            {
                throw new ArgumentException("Invalid project file");
            }
                
            EditorEnvironment.Project = project.Value;
            EditorEnvironment.ProjectPath = args[1];
            
            return;
        }
        
        if (args.Length <= 2)
        {
            throw new ArgumentException(
                "Invalid project path, expected either valid project path or create flag and project name");
        }
            
        EditorEnvironment.Project = CreateProject(args[1], args[2]);
        EditorEnvironment.ProjectPath = args[1];
    }

    private Project CreateProject(string projectPath, string projectName)
    {
        var project = new Project();
        project.Name = projectName;
        project.Version = "1.0.0";
        project.CreationDate = DateTime.Now;
        project.GUID = Guid.NewGuid().ToString();
        
        var serializer = new XmlSerializer(typeof(Project));
        serializer.Serialize(File.OpenWrite(projectPath + "/project.pproj"), project);
        
        // Create a new package file
        string projectContent = 
"""
// swift-tools-version: 6.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "Assembly",
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "Assembly",
            type: .dynamic,
            targets: ["Assembly"]
        ),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .target(
            name: "Assembly",
            path: "./",
            swiftSettings: [
                .unsafeFlags([
                "-I", "H:/build",
                "-L", "H:/build",
                "-lPlayground",
                "-lH:/build/Playground.lib"
                ])
            ]
        ),
    ]
)
""";
        
        // Create code folder
        var codeDir = Directory.CreateDirectory(projectPath + "/code");
        // Create content folder
        var contentDir = Directory.CreateDirectory(projectPath + "/content");
        // Create cache folder
        Directory.CreateDirectory(projectPath + "/.cache");
        
        File.WriteAllText(codeDir.FullName + "/Package.swift", projectContent);

        var buildProps = @"<Project>
  <PropertyGroup>
    <ArtifactsPath>./.cache/artifacts</ArtifactsPath>
  </PropertyGroup>
</Project>
";
        File.WriteAllText($"{projectPath}/Directory.Build.props", buildProps);
        
        Directory.CreateDirectory(contentDir + "/" + "scenes");
        
        File.WriteAllText(projectPath + "/content/scenes/SampleScene.pscn", "{}");
        
        return project;
    }
}
