using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Platform;
using Avalonia.Threading;

namespace PlaygroundEditor;

public partial class MainWindow : Window
{
    private bool _mouseDownForWindowMoving = false;
    private PointerPoint _originalPoint;
    private ProgressBar _backgroundProgressBar;
    private bool _isDragging = false;
    private Thread _engineThread;
    private Thread _editorThread;
    private TreeView _hierarchyTreeView;
    IntPtr _lookup;
    IntPtr _startup;
    
    public MainWindow()
    {
        InitializeComponent();
        
        DataContext = new MainWindowViewModel(
            this.FindControl<ProgressBar>("BackgroundProgressBar")!
        );
        
        EditorEnvironment.OnEnterPlayMode += () =>
        {
            KeyboardNavigation.SetTabNavigation(this, KeyboardNavigationMode.None);
        };
        EditorEnvironment.OnExitPlayMode += () =>
        {
            KeyboardNavigation.SetTabNavigation(this, KeyboardNavigationMode.Cycle);
        };

        _editorThread = new Thread(OnEditorUpdate);
        _editorThread.Start();
    }

    private void OnEditorUpdate()
    {
        Thread.Sleep(60);
        EditorWindowManager.OnUpdate();
    }

    protected override void OnOpened(EventArgs e)
    {
        base.OnOpened(e);
        this.FindControl<TextBlock>("TitleText")!.Text = $"{EditorEnvironment.Project.Name}";
    }

    private void InputElement_OnPointerMoved(object? sender, PointerEventArgs e)
    {
        if (!_mouseDownForWindowMoving) return;

        var currentPoint = e.GetCurrentPoint(this);
        Position = new PixelPoint(Position.X + (int)(currentPoint.Position.X - _originalPoint.Position.X),
            Position.Y + (int)(currentPoint.Position.Y - _originalPoint.Position.Y));
    }

    private void InputElement_OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        if (WindowState is WindowState.Maximized or WindowState.FullScreen) return;

        _mouseDownForWindowMoving = true;
        _originalPoint = e.GetCurrentPoint(this);
    }

    private void InputElement_OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        _mouseDownForWindowMoving = false;
    }

    protected override void OnClosed(EventArgs e)
    {

    }

    private void NativeEmbed_OnLoaded(object? sender, RoutedEventArgs e)
    {
        var embed = (NativeEngineEmbedHost)sender!;

        var engineThread = new Thread(
            () => {
                NativeEngineEmbedHost.ScriptingStartup startupCallback = (lookupFn, startupFn) =>
                {
                    _lookup = lookupFn;
                    _startup = startupFn;
                };
        
                // Call Swift to set the pointers
                NativeEngineEmbedHost.PlaygroundMain(startupCallback);
                
                var editorDel = new NativeLookupTable.AddEntryDelegate(NativeLookupTable.AddEntry);

                GCHandle.Alloc(editorDel);
                
                var config = new NativeEngineEmbedHost.PlaygroundConfig {
                    Delegate = _lookup!,
                    EditorDelegate = Marshal.GetFunctionPointerForDelegate(editorDel),
                    StartupCallback = _startup!,
                    Width = 1280,
                    Height = 720,
                    Fullscreen = false,
                    WindowName = "Editor",
                    Path = Directory.GetCurrentDirectory(),
                    WindowHandle = embed._handle
                };
                
                NativeEngineEmbedHost.PlaygroundCoreMain(ref config);
                //EditorEnvironment.Setup();
            });
            
        engineThread.Start();
    }
    
    private void NativeEmbed_OnGotFocus(object? sender, PointerEventArgs e) {
        SceneViewManager.OnGotFocus();
    }
    
    private void NativeEmbed_OnLostFocus(object? sender, PointerEventArgs e) {
        SceneViewManager.OnLostFocus();
    }

    private void InputElement_OnKeyDown(object? sender, KeyEventArgs e)
    {
        Console.WriteLine(e.Key + " " + e.KeyModifiers);
    }

    private void Window_OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        Console.WriteLine(e.GetCurrentPoint(this));
    }

    private void FileMenu_OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        Console.WriteLine(e.GetCurrentPoint(this));
    }

    private void MainWindow_OnLoaded(object? sender, RoutedEventArgs e)
    {
        ((MainWindowViewModel)DataContext!)?.OnViewDidAppear();
    }
}
