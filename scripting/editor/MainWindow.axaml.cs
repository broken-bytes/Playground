using System;
using System.Linq;
using System.Reflection;
using System.Threading;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Platform;
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
    
    private bool _isDragging = false;
    
    public MainWindow()
    {
        InitializeComponent();
        
        var assembly = Assembly.Load("Playground");

        var types = assembly.GetTypes();
        var mainHandler = types.FirstOrDefault(type => type.Name.Contains("PlaygroundMainHandler"));

        // Get the static method OnStart
        var onStartMethod = mainHandler.GetMethod("OnStart", BindingFlags.Static | BindingFlags.NonPublic);

        if (onStartMethod != null)
        {
            // Get the HWND from the window implementation
            if (PlatformImpl is IWindowImpl windowImpl)
            {
                IntPtr hwnd = windowImpl.Handle?.Handle ?? IntPtr.Zero;

                // Invoke the static OnStart method with the HWND
                onStartMethod.Invoke(null, new object[] { hwnd });
            }
        }
        
        Thread.Sleep(250);
        
        var dummy = new GameObject();
        dummy.Attach<TimerComponent>();
    }
    
    private void InputElement_OnPointerMoved(object? sender, PointerEventArgs e)
    {
        if (!_mouseDownForWindowMoving) return;

        PointerPoint currentPoint = e.GetCurrentPoint(this);
        Position = new PixelPoint(Position.X + (int)(currentPoint.Position.X - _originalPoint.Position.X),
            Position.Y + (int)(currentPoint.Position.Y - _originalPoint.Position.Y));
    }

    private void InputElement_OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        if (WindowState == WindowState.Maximized || WindowState == WindowState.FullScreen) return;

        _mouseDownForWindowMoving = true;
        _originalPoint = e.GetCurrentPoint(this);
    }

    private void InputElement_OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        _mouseDownForWindowMoving = false;
    }
    
    private void LeftSplitter_OnPointerMoved(object? sender, PointerEventArgs e)
    {
        if (!_isDragging) return;

        PointerPoint currentPoint = e.GetCurrentPoint(this);
        Position = new PixelPoint(Position.X + (int)(currentPoint.Position.X - _originalPoint.Position.X),
            Position.Y + (int)(currentPoint.Position.Y - _originalPoint.Position.Y));
    }

    private void LeftSplitter_OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        if (WindowState == WindowState.Maximized || WindowState == WindowState.FullScreen) return;

        _isDragging = true;
        _originalPoint = e.GetCurrentPoint(this);
    }

    private void LeftSplitter_OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        _isDragging = false;
    }
}
