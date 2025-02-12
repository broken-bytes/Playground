using System.Text.RegularExpressions;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace PlaygroundEditor.Controls;

public partial class VectorField : UserControl {
    public VectorField() {
        InitializeComponent();
    }

    private void TextBox_OnTextChanged(object? sender, TextChangedEventArgs e) {
        var maskBox = (e.Source as MaskedTextBox)!;
    
        // Get the text entered in the MaskedTextBox
        
        if (!float.TryParse(maskBox.Text, out _)) {
            e.Handled = true;
            maskBox.Text = maskBox.Text.Substring(0, maskBox.Text.Length - 1);
        }
    }
}

