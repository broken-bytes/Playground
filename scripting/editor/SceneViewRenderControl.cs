using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;

namespace PlaygroundEditor;

public class SceneViewRenderControl: Control
{
    public override void Render(DrawingContext context)
    {
        // Custom drawing here
        context.FillRectangle(Brushes.Transparent, new Rect(0, 0, this.Width, this.Height));
    }
}
