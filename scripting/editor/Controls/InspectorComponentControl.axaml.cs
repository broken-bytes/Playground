using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace PlaygroundEditor.Controls;

public partial class InspectorComponentControl : UserControl {
    public InspectorComponentControl(string title, List<Control> content) {
        InitializeComponent();
        
        var expander =  this.FindControl<Expander>("Expander");
        
        expander.Header = title;
        expander.IsExpanded = true;
        
        var componentContent = this.FindControl<StackPanel>("ComponentContent");
        componentContent.Children.AddRange(content);
    }
}

