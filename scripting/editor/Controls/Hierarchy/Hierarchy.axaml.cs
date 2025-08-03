using System;
using System.Collections.Generic;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Tmds.DBus.Protocol;

namespace PlaygroundEditor.Controls;

public partial class Hierarchy : UserControl, EditorWindow
{
    private TreeView _hierarchyTreeView;
    private Dictionary<object, Entity> _treeViewObjects = [];
    
    public Hierarchy()
    {
        InitializeComponent();
        
        _hierarchyTreeView = this.FindControl<TreeView>("HierarchyTree")!;
    }

    protected override void OnLoaded(RoutedEventArgs e)
    {
        base.OnLoaded(e);
        
        EditorWindowManager.EditorWindows.Add(this);
        
        EditorEnvironment.OnObjectSelected += OnObjectSelected;
        EditorEnvironment.OnObjectDeselected += OnObjectDeselected;
        
        HierarchyManager.OnAddEntity += OnEntityCreated;
        HierarchyManager.OnRemoveEntity += OnEntityRemoved;
        HierarchyManager.OnSetParent += OnEntitySetParent;
    }

    private void OnObjectSelected(Object obj) {
    }
    
    private void OnObjectDeselected() {
    }

    private void OnEntityCreated(Entity ent) {
        Dispatcher.UIThread.Post(() =>
        {
            InjectEntityTreeView(ent, _hierarchyTreeView);
        });
    }

    private void OnEntityRemoved(Entity ent) {
        Dispatcher.UIThread.Post(() =>
        {
            object? key = null;
            foreach (var treeViewObject in _treeViewObjects) {
                if (treeViewObject.Value == ent) {
                    key = treeViewObject.Key;
                    break;
                }
            }

            if (key == null) { return; }
            
            _treeViewObjects.Remove(key);
            _hierarchyTreeView.Items.Remove(key);
        });
    }

    private void OnEntitySetParent(Entity child, Entity parent) {
        Dispatcher.UIThread.Post(() => {
                object? key = null;
                foreach (var treeViewObject in _treeViewObjects) {
                    if (treeViewObject.Value == parent) {
                        key = treeViewObject.Key;
                        break;
                    }
                }

                object? childKey = null;
                foreach (var treeViewObject in _treeViewObjects) {
                    if (treeViewObject.Value == child) {
                        childKey = treeViewObject.Key;
                        break;
                    }
                }

                if (key == null || childKey == null) {
                    return;
                }

                _hierarchyTreeView.Items.Remove(childKey);
                ((TreeViewItem)key).Items.Add(childKey);
            }
        );
    }

    public void OnEditorUpdate()
    {
        UpdateHierarchy();
    }

    private void UpdateHierarchy()
    {
        Dispatcher.UIThread.Post(() =>
        {
            //_hierarchyTreeView.Items.Clear();
            // TODO: Get all entities in scene
        });
    }

    private void InjectEntityTreeView(Entity ent, ItemsControl item)
    {
        TreeViewItem treeView = new();
        treeView.Header = ent.Name;
        
        treeView.PointerPressed += TreeViewOnPointerPressed;
        
        item.Items.Add(treeView);
        
        _treeViewObjects.Add(treeView, ent);
    }

    private void TreeViewOnPointerPressed(object? sender, PointerPressedEventArgs e) {
        if (sender is null) {
            return;
        }
        
        EditorEnvironment.SelectedObject = _treeViewObjects[sender];
    }
}

