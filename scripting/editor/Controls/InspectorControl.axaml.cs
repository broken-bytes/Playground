using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Playground;
using Object = Playground.Object;

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
        
        if (objc is GameObject go) {
            var posvector = new Vector3Field();
            var rotationVector = new Vector3Field();
            var scaleVector = new Vector3Field();

            posvector.SetValue(go.Transform.Position);
            rotationVector.SetValue(go.Transform.EulerAngles);
            scaleVector.SetValue(go.Transform.Scale);
            
            var posField = new InspectorField("Position", posvector);
            var rotation = new InspectorField("Rotation", rotationVector);
            var scaleField = new InspectorField("Scale", scaleVector);

            var transform = new InspectorComponentControl("Transform", [posField, rotation, scaleField]);

            _content.Children.Add(transform);
        }
    }

    public void OnEditorUpdate() {
        _content.Children.Clear();
    }
}

