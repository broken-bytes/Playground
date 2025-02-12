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
    }

    public void OnEditorUpdate() {
        _content.Children.Clear();

        var posvector = new VectorField();
        var rotationVector = new VectorField();
        var scaleVector = new VectorField();
        var posField = new InspectorField("Position", posvector);
        var rotation = new InspectorField("Rotation", rotationVector);
        var scaleField = new InspectorField("Scale", scaleVector);
        
        var transform = new InspectorComponentControl("Transform", [posField, rotation, scaleField]);
        
        _content.Children.Add(transform);
    }
}

