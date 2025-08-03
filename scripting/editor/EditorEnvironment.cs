using System;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace PlaygroundEditor;

public static class EditorEnvironment
{
    internal static Project Project;
    internal static string ProjectPath;

    internal static Object? SelectedObject {
        get => _selectedObject;
        set {
            if (value == null) {
                OnObjectDeselected?.Invoke();
                _selectedObject = null;
                
                return;
            }
            
            OnObjectSelected?.Invoke(value);
            _selectedObject = value;
        }
    }
    
    private static Object? _selectedObject;
    
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
    internal delegate void OnObjectSelectedDelegate(Object obj);
    internal delegate void OnObjectDeselectedDelegate();
    
    internal static OnEnterPlayModeDelegate? OnEnterPlayMode { get; set; }
    internal static OnExitPlayModeDelegate? OnExitPlayMode { get; set; }
    internal static OnPausePlayModeDelegate? OnPausePlayMode { get; set; }
    internal static OnObjectSelectedDelegate? OnObjectSelected { get; set; }
    internal static OnObjectDeselectedDelegate? OnObjectDeselected { get; set; }

    internal static void Setup() {
        SceneViewManager.Setup();
        SceneManager.Setup();
        EventManager.Setup();
        HierarchyManager.Setup();
    }

    internal static void EnterPlayMode()
    {
        IsPlayMode = true;
        
        // FIXME Debug: test serialization of whole scene
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
