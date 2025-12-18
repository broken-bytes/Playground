namespace PlaygroundEditor;

using System;
using System.IO;

internal class FileWatchdog
{
    private FileSystemWatcher _watcher;
    
    internal delegate void OnFileAddedDelegate(FileSystemEventArgs args);
    internal delegate void OnFileChangedDelegate(FileSystemEventArgs args);
    internal delegate void OnFileRemovedDelegate(FileSystemEventArgs args);
    
    internal event OnFileAddedDelegate FileAdded;
    internal event OnFileChangedDelegate FileChanged;
    internal event OnFileRemovedDelegate FileRemoved;
    
    internal FileWatchdog(string directoryPath)
    {
        _watcher = new FileSystemWatcher(directoryPath)
        {
            NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite | NotifyFilters.Attributes,
            Filter = "*.*", // Watch all file types
            EnableRaisingEvents = true // Start the watcher
        };

        // Event handlers for different file system changes
        _watcher.Created += OnFileCreated;
        _watcher.Changed += OnFileChanged;
        _watcher.Deleted += OnFileDeleted;
        _watcher.Renamed += OnFileRenamed;
    }

    // Event handler for file creation
    private void OnFileCreated(object sender, FileSystemEventArgs e)
    {
        FileAdded?.Invoke(e);
    }

    // Event handler for file changes
    private void OnFileChanged(object sender, FileSystemEventArgs e)
    {
        FileChanged?.Invoke(e);
    }

    // Event handler for file deletion
    private void OnFileDeleted(object sender, FileSystemEventArgs e)
    {
        FileRemoved?.Invoke(e);
    }

    // Event handler for file renaming
    private void OnFileRenamed(object sender, RenamedEventArgs e)
    {
        FileChanged?.Invoke(e);
    }

    // Stops the file watcher
    public void StopWatching()
    {
        _watcher.EnableRaisingEvents = false;
        _watcher.Dispose();
    }
}
