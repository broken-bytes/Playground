using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Dock.Avalonia.Controls;
using Dock.Model.Controls;
using ReactiveUI;

namespace PlaygroundEditor;

public class MainWindowViewModel : ReactiveObject
{
    private FileWatchdog _codeWatchdog;
    private FileWatchdog _assetsWatchdog;
    private ProgressBar _backgroundProgressbar;

    public MainWindowViewModel(ProgressBar progressBar, DockControl dock)
    {
        _backgroundProgressbar = progressBar;
    }
    
    internal void OnViewDidAppear()
    {
        _codeWatchdog = new FileWatchdog(EditorEnvironment.ProjectPath + "/code");
        _assetsWatchdog = new FileWatchdog(EditorEnvironment.ProjectPath + "/content");

        BuildManager.Setup(EditorEnvironment.ProjectPath, EditorEnvironment.Project.Name);
        
        _codeWatchdog.FileAdded += CodeWatchdogOnFileAdded;
        _codeWatchdog.FileChanged += CodeWatchdogOnFileChanged;

        _backgroundProgressbar.Value = 0;
        _backgroundProgressbar.Background = new SolidColorBrush(Theme.Colours.Elevated);
        _backgroundProgressbar.Foreground = new SolidColorBrush(Theme.Colours.Actions.Friendly);
        
        RebuildAssembly();
    }

    private void CodeWatchdogOnFileChanged(FileSystemEventArgs args)
    {
        if (!args.FullPath.EndsWith(".swift"))
        {
            return;
        }

        var filePath = args.FullPath;
        File.Create(filePath + ".pmeta").Close();
        
        RebuildAssembly();
    }

    private void CodeWatchdogOnFileAdded(FileSystemEventArgs args)
    {
        if (!args.FullPath.EndsWith(".swift"))
        {
            return;
        }
        
        var filePath = args.FullPath;
        File.Create(filePath + ".pmeta").Close();
        
        RebuildAssembly();
    }

    public void OpenFileCommand()
    {
        
    }

    public void CreateObjectCommand()
    {
        
    }

    private void RebuildAssembly()
    {
        Task.Run(async () =>
        {
            _backgroundProgressbar.Value = 20;
            await BuildManager.UpdateBuildConfig();
            _backgroundProgressbar.Value = 50;
            await BuildManager.RunBuild();
            _backgroundProgressbar.Value = 100;
            await Task.Delay(1000);
            _backgroundProgressbar.Value = 0;
        });
    }
}
