using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Playground;
using Tmds.DBus.Protocol;

namespace PlaygroundEditor.Controls;

public partial class HierarchyControl : UserControl, EditorWindow
{
    private TreeView _hierarchyTreeView;
    private Dictionary<object, Object> _treeViewObjects = [];
    
    public HierarchyControl()
    {
        InitializeComponent();
        
        _hierarchyTreeView = this.FindControl<TreeView>("Hierarchy")!;
    }

    protected override void OnLoaded(RoutedEventArgs e)
    {
        base.OnLoaded(e);
        
        EditorWindowManager.EditorWindows.Add(this);
        
        EditorEnvironment.OnObjectSelected += OnObjectSelected;
        EditorEnvironment.OnObjectDeselected += OnObjectDeselected;
    }

    private void OnObjectSelected(Object obj) {
    }
    
    private void OnObjectDeselected() {
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
        
        treeView.PointerPressed += TreeViewOnPointerPressed;
        
        item.Items.Add(treeView);
        
        _treeViewObjects.Add(treeView, objc);

        foreach (var child in objc.Children)
        {
            InjectGameObjectTreeView(child, treeView);
        }
    }

    private void TreeViewOnPointerPressed(object? sender, PointerPressedEventArgs e) {
        if (sender is null) {
            return;
        }
        
        EditorEnvironment.SelectedObject = _treeViewObjects[sender];
    }
}

