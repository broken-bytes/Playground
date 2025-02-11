namespace PlaygroundEditor;

public static class EditorEnvironment
{
    internal static Project Project;
    internal static string ProjectPath;
    
    internal static bool IsPlayMode
    {
        get => _isPlayMode;
        set
        {
            _isPlayMode = value;
            if (_isPlayMode)
            {
                OnEnterPlayMode?.Invoke();
            }
            else
            {
                IsPaused = false;
                OnExitPlayMode?.Invoke();
            }
        }
    }

    internal static bool IsPaused
    {
        get => _isPaused;
        set =>  _isPaused = value;
    }
    
    private static bool _isPlayMode;
    private static bool _isPaused;
    
    internal delegate void OnEnterPlayModeDelegate();
    internal delegate void OnExitPlayModeDelegate();
    internal delegate void OnPausePlayModeDelegate();
    
    internal static OnEnterPlayModeDelegate? OnEnterPlayMode { get; set; }
    internal static OnExitPlayModeDelegate? OnExitPlayMode { get; set; }
    internal static OnPausePlayModeDelegate? OnPausePlayMode { get; set; }

    internal static void EnterPlayMode()
    {
        IsPlayMode = true;
    }

    internal static void ExitPlayMode()
    {
        IsPlayMode = false;
    }

    internal static void PausePlayMode()
    {
        IsPaused = true;
    }
}
