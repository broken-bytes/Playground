using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace PlaygroundEditor;

internal static class BuildManager
{
    private static string _projectPath;
    private static string _projectName;
    private static string _playgroundAssemblyPath => $"{_projectPath}\\GameAssembly.csproj";
    private static string _solutionPath => $"{_projectPath}\\{_projectName}.sln";
    
    private static List<string> _filesToBuild = new();

    internal static void Setup(string projectPath, string projectName)
    {
        _projectPath = projectPath;
        _projectName = projectName;
        
        UpdateBuildConfig();
    }
    
    internal static async Task<bool> RunBuild()
    {
        // Create a new process to run the `dotnet build` command
        Process process = new Process();
        
        // Set the start info for the process
        process.StartInfo = new ProcessStartInfo
        {
            FileName = "dotnet",
            Arguments = $"build \"{_solutionPath}\"",
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false, 
            CreateNoWindow = false 
        };

        // Event handlers to capture the output and errors
        process.OutputDataReceived += (sender, e) => 
        {
            if (e.Data != null)
            {
                Console.WriteLine("Output: " + e.Data);
            }
        };

        process.ErrorDataReceived += (sender, e) => 
        {
            if (e.Data != null)
            {
                Console.WriteLine("Error: " + e.Data);
            }
        };
        
        await Task.Run(() =>
        {
            try
            {
                // Start the process
                process.Start();

                // Start reading the output and error asynchronously
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                // Wait for the process to exit
                process.WaitForExit();

                if (process.ExitCode == 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            catch (Exception ex)
            {
                return false;
            }
        });

        return false;
    }

    internal static async Task UpdateBuildConfig()
    {
        await Task.Run(() =>
        {
            // Check all .cs files in folder and subfolders
            foreach (var entry in Directory.EnumerateFiles(_projectPath + "\\code"))
            {
                if (entry.EndsWith(".cs"))
                {
                    _filesToBuild.Add(entry);
                }
            }
        
            var projectFileContent = ProjectHeader;
            projectFileContent += "     <ItemGroup>\n";
            foreach (var file in _filesToBuild)
            {
                projectFileContent += $"        <Compile Include=\"{file}\" />\n";
            }
        
            projectFileContent += "        <Reference Include=\"Playground\">\n";
            projectFileContent += $"             <HintPath>H:\\\\Projects\\\\Playground\\\\scripting\\\\editor\\\\bin\\\\Debug\\\\net9.0\\\\Playground.dll</HintPath>\n";
            projectFileContent += "        </Reference>\n";
            projectFileContent += "     </ItemGroup>\n";
            projectFileContent += "</Project>";
        
            File.WriteAllText(_playgroundAssemblyPath, projectFileContent);
        });
    }

    private static string ProjectHeader = $@"<?xml version=""1.0"" encoding=""utf-8""?>
<Project Sdk=""Microsoft.NET.Sdk"">
  <PropertyGroup>
    <Configuration Condition="" '$(Configuration)' == '' "">Debug</Configuration>
    <Platform Condition="" '$(Platform)' == '' "">x64</Platform>
    <UseArtifactsOutput>true</UseArtifactsOutput>
    <RootNamespace>GameAssembly</RootNamespace>
    <OutputType>Library</OutputType>
    <TargetFramework>net9.0</TargetFramework>
    <LangVersion>13</LangVersion>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
    <FileAlignment>512</FileAlignment>
    <BaseDirectory>./code</BaseDirectory>
    <IntermediateOutputPath>./.cache\bin\$(Configuration)\</IntermediateOutputPath>
  </PropertyGroup>
  <PropertyGroup Condition="" '$(Configuration)|$(Platform)' == 'Debug|x64' "">
    <DebugSymbols>true</DebugSymbols>
    <DebugType>full</DebugType>
    <Optimize>false</Optimize>
    <DefineConstants></DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
    <TreatWarningsAsErrors>False</TreatWarningsAsErrors>
  </PropertyGroup>
  <PropertyGroup>
    <NoConfig>true</NoConfig>
    <NoStdLib>true</NoStdLib>
    <AddAdditionalExplicitAssemblyReferences>false</AddAdditionalExplicitAssemblyReferences>
    <ImplicitlyExpandNETStandardFacades>false</ImplicitlyExpandNETStandardFacades>
    <ImplicitlyExpandDesignTimeFacades>false</ImplicitlyExpandDesignTimeFacades>
  </PropertyGroup>
";
}
