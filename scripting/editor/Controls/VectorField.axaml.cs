using System.Globalization;
using System.Text.RegularExpressions;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Playground;

namespace PlaygroundEditor.Controls;

public partial class Vector3Field : UserControl {
    private MaskedTextBox _xTextBox;
    private MaskedTextBox _yTextBox;
    private MaskedTextBox _zTextBox;
    
    public Vector3Field() {
        InitializeComponent();
        
        _xTextBox = this.FindControl<MaskedTextBox>("X");
        _yTextBox = this.FindControl<MaskedTextBox>("Y");
        _zTextBox = this.FindControl<MaskedTextBox>("Z");
    }

    private void TextBox_OnTextChanged(object? sender, TextChangedEventArgs e) {
        var maskBox = (e.Source as MaskedTextBox)!;
    
        // Get the text entered in the MaskedTextBox
        
        if (!float.TryParse(maskBox.Text, out _)) {
            e.Handled = true;
            maskBox.Text = maskBox.Text.Substring(0, maskBox.Text.Length - 1);
        }
    }

    internal void SetValue(Vector3 value) {
        _xTextBox.Text = value.X.ToString(CultureInfo.InvariantCulture);
        _yTextBox.Text = value.Y.ToString(CultureInfo.InvariantCulture);
        _zTextBox.Text = value.Z.ToString(CultureInfo.InvariantCulture);
    }
}

