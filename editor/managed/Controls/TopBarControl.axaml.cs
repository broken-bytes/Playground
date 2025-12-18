using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;

namespace PlaygroundEditor;

public partial class TopBarControl : UserControl
{
    private Button _playButton;
    private Button _pauseButton;
    public TopBarControl()
    {
        InitializeComponent();

        _playButton = this.FindControl<Button>("PlayButton");
        _pauseButton = this.Find<Button>("PauseButton");
    }

    private void PlayButton_OnPointerPressed(object sender, RoutedEventArgs e)
    {
        if (!EditorEnvironment.IsPlayMode)
        {
            _playButton.Background = new SolidColorBrush(PlaygroundEditor.Theme.Colours.Actions.Friendly);
            EditorEnvironment.EnterPlayMode();
        }
        else
        {
            _pauseButton.Background = new SolidColorBrush(PlaygroundEditor.Theme.Colours.Elevated);
            _playButton.Background = new SolidColorBrush(PlaygroundEditor.Theme.Colours.Elevated);
            EditorEnvironment.ExitPlayMode();
        }
    }

    private void PauseButton_OnClick(object? sender, RoutedEventArgs e)
    {
        if (!EditorEnvironment.IsPlayMode)
        {
            return;
        }

        if (EditorEnvironment.IsPlayMode)
        {
            if (!EditorEnvironment.IsPaused)
            {
                _pauseButton.Background = new SolidColorBrush(PlaygroundEditor.Theme.Colours.Actions.Destructive);
                EditorEnvironment.PausePlayMode();
            }
            else
            {
                _pauseButton.Background = new SolidColorBrush(PlaygroundEditor.Theme.Colours.Elevated);
                EditorEnvironment.PausePlayMode();
            }
        }
    }
}

