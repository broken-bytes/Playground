using Avalonia.Media;

namespace PlaygroundEditor;

public static class Theme
{
    public static class Colours
    {
        public static readonly Color Background = Color.Parse("#16233B");
        public static readonly Color BackgroundLight = Color.Parse("#1E2A47");
        public static readonly Color Elevated = Color.Parse("#777A7A7A");
        
        public static class Actions
        {
            public static readonly Color Friendly = new Color(255, 100, 150, 100);
            public static readonly Color Destructive = new Color(255, 150, 100, 100);
        }
    }
}
