using System;
using System.IO;
using System.Xml;
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
        
        // Create a csproj file
        // Create a new .csproj file
        string projectFile = "GameAssembly.csproj";
        string projectContent = "";
        
        File.WriteAllText($"{projectPath}/{projectFile}", projectContent);
        
        // Create a sln file
        string solutionFile = $"{projectName}.sln";
        string solutionContent = $@"
Microsoft Visual Studio Solution File, Format Version 12.00
Project(""{{{Guid.NewGuid().ToString()}}}"") = ""GameAssembly"", ""GameAssembly.csproj"", ""{{{project.GUID}}}""
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{{{project.GUID}}}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		{{{project.GUID}}}.Debug|Any CPU.Build.0 = Debug|Any CPU
		{{{project.GUID}}}.Release|Any CPU.ActiveCfg = Release|Any CPU
		{{{project.GUID}}}.Release|Any CPU.Build.0 = Release|Any CPU
	EndGlobalSection
EndGlobal
        ";
        File.WriteAllText($"{projectPath}/{solutionFile}", solutionContent);
        
        // Create code folder
        Directory.CreateDirectory(projectPath + "/code");
        // Create content folder
        Directory.CreateDirectory(projectPath + "/content");
        // Create cache folder
        Directory.CreateDirectory(projectPath + "/.cache");

        var buildProps = @"<Project>
  <PropertyGroup>
    <ArtifactsPath>./.cache/artifacts</ArtifactsPath>
  </PropertyGroup>
</Project>
";
        
        File.WriteAllText($"{projectPath}/Directory.Build.props", buildProps);
        
        return project;
    }
}
