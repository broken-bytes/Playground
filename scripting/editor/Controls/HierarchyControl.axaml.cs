using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Playground;

namespace PlaygroundEditor.Controls;

public partial class HierarchyControl : UserControl, EditorWindow
{
    private TreeView _hierarchyTreeView;
    
    public HierarchyControl()
    {
        InitializeComponent();
        
        _hierarchyTreeView = this.FindControl<TreeView>("Hierarchy")!;
    }

    protected override void OnLoaded(RoutedEventArgs e)
    {
        base.OnLoaded(e);
        
        EditorWindowManager.EditorWindows.Add(this);
    }

    public void OnEditorUpdate()
    {
        UpdateHierarchy();
    }

    private void UpdateHierarchy()
    {
        Dispatcher.UIThread.Post(() =>
        {
            _hierarchyTreeView.Items.Clear();
            foreach (var objc in Playground.SceneManager.SceneObjects)
            {
                InjectGameObjectTreeView(objc, _hierarchyTreeView);
            } 
        });
    }

    private void InjectGameObjectTreeView(GameObject objc, ItemsControl item)
    {
        TreeViewItem treeView = new();
        treeView.Header = objc.Name;
        
        item.Items.Add(treeView);

        foreach (var child in objc.Children)
        {
            InjectGameObjectTreeView(child, treeView);
        }
    }
}

