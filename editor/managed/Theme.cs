using Avalonia.Media;

namespace PlaygroundEditor;

public static class Theme
{
    public static class Colours
    {
        public static readonly Color Background = Color.Parse("#121212");
        public static readonly Color BackgroundLight = Color.Parse("#282828");
        public static readonly Color Elevated = Color.Parse("#3f3f3f");
        
        public static class Actions
        {
            public static readonly Color Friendly = new Color(255, 100, 150, 100);
            public static readonly Color Destructive = new Color(255, 150, 100, 100);
        }
    }
}
