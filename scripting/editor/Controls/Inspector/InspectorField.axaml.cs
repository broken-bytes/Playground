using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace PlaygroundEditor.Controls;

public partial class InspectorField : UserControl {
    public InspectorField(string title, UserControl content) {
        InitializeComponent();
        
        this.FindControl<TextBlock>("FieldTitle").Text = title;
        this.FindControl<StackPanel>("FieldContent").Children.Add(content);
    }
}
