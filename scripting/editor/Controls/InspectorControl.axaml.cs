using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Markup.Xaml;
using Avalonia.Media;

namespace PlaygroundEditor.Controls;

public partial class InspectorControl : UserControl, EditorWindow {
    private StackPanel _content;
    
    public InspectorControl() {
        InitializeComponent();
        _content = this.FindControl<StackPanel>("Content")!;
        
        EditorWindowManager.EditorWindows.Add(this);
        
        EditorEnvironment.OnObjectSelected += OnObjectSelected;
        EditorEnvironment.OnObjectDeselected += OnObjectDeselected;
        
    }

    private void OnObjectDeselected() {
        _content.Children.Clear();
    }

    private void OnObjectSelected(Object objc) {
        _content.Children.Clear();
    }

    public void OnEditorUpdate() {
        _content.Children.Clear();
    }
}

