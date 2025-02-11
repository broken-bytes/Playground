using System;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Threading;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Platform;
using Avalonia.Threading;
using Playground;

namespace PlaygroundEditor;

class TimerComponent : Component
{
    public override void OnUpdate()
    {
        Console.WriteLine(Time.DeltaTime);
    }
}

public partial class MainWindow : Window
{
    private bool _mouseDownForWindowMoving = false;
    private PointerPoint _originalPoint;
    private ProgressBar _backgroundProgressBar;
    private bool _isDragging = false;
    private Thread _engineThread;
    private Thread _editorThread;
    private TreeView _hierarchyTreeView;
    
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
        var assembly = Assembly.Load("Playground");

        var types = assembly.GetTypes();
        var mainHandler = types.FirstOrDefault(type => type.Name.Contains("PlaygroundMainHandler"));
        
        var onShutdownMethod = mainHandler?.GetMethod("OnDestroy", BindingFlags.Static | BindingFlags.NonPublic);
        onShutdownMethod?.Invoke(null, new object[] { });
        
        System.Environment.Exit(0);
    }

    private void NativeEmbed_OnLoaded(object? sender, RoutedEventArgs e)
    {
        var embed = (NativeEngineEmbedHost)sender!;
        
        var assembly = Assembly.Load("Playground");

        var types = assembly.GetTypes();
        var mainHandler = types.FirstOrDefault(type => type.Name.Contains("PlaygroundMainHandler"));
        var renderer = types.FirstOrDefault(type => type.Name.Contains("Renderer"));
        
        var readBackBufferMethod = renderer?.GetMethod("ReadBackBuffer", BindingFlags.Static | BindingFlags.NonPublic);
        
        Environment.ReadBackBufferMethod = readBackBufferMethod;

        // Get the static method OnStart
        var onStartMethod = mainHandler?.GetMethod("OnStart", BindingFlags.Static | BindingFlags.NonPublic);
        var onUpdate = mainHandler?.GetMethod("OnUpdate", BindingFlags.Static | BindingFlags.NonPublic);
        
        if (onStartMethod != null)
        {
            // Get the HWND from the window implementation
            if (PlatformImpl is { } windowImpl)
            {
                IntPtr hwnd = windowImpl.Handle?.Handle ?? IntPtr.Zero;

                // Invoke the static OnStart method with the HWND
                onStartMethod.Invoke(
                    null, 
                    new object[]
                    {
                        embed._handle, 
                        (uint)embed.Bounds.Width, 
                        (uint)embed.Bounds.Height, 
                        false,
                        EditorEnvironment.ProjectPath + @"/.cache/artifacts/bin/GameAssembly/debug/GameAssembly.dll",
                    });

                Environment.UpdateMethod = onUpdate;
            }
        }
        
        _engineThread = new Thread(() =>
        {
            while (true)
            {
                EditorLoop.OnUpdate();
            }
        });
        _engineThread.Start();
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
