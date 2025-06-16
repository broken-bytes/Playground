public typealias StartDelegate = @convention(c) (@convention(c) (UnsafePointer<CChar>, UnsafeMutableRawPointer) -> Void, @convention(c) () -> Void) -> Void

@_cdecl("PlaygroundMain")
public func playgroundMain(delegate: StartDelegate) {
    delegate(
        { NativeLookupTable.addEntry(key: String(cString: $0), ptr: $1) },
        startUp
    )
}

func startUp() {
    Logger.start()
    Logger.info("Playground Scripting Layer Startup")
    Logger.info("Started Logger")
    Profiler.start()
    Logger.info("Started Profiler")
    Input.start()
    Logger.info("Started Input")
    EventHandler.start()
    Logger.info("Started EventHandler")
    ECSHandler.start()
    Logger.info("Started ECSHandler")
    AssetHandler.setup()
    Logger.info("Started Assethandler")
    Renderer.setup()
    Logger.info("Started Renderer")

    NativeMath.setup()

    initComponents()
    initTags()
    initHooks()
    initSystems()

    let materialhandle = AssetHandler.loadMaterial(named: "default.mat")
    let modelhandle = AssetHandler.loadModel(named: "cube.mod")

    for x in 0..<50000 {
        let entity = Entity("MainCamera_\(x)")
        var cam = CameraComponent(id: 0)
        entity.addComponent(&cam)
            if x % 500 == 0 {
            var material = MaterialComponent(handle: materialhandle)
            entity.addComponent(&material)
            var mesh = MeshComponent(handle: modelhandle, meshId: 0)
            entity.addComponent(&mesh)
        }
        var transform = TransformComponent(position: .zero, rotation: .identity, scale: .one)
        entity.addComponent(&transform)
    }
}

func initComponents() {
    let componentId = ECSHandler.registerComponent(CameraComponent.self)
    let transformId = ECSHandler.registerComponent(TransformComponent.self)
    let worldTransformId = ECSHandler.registerComponent(WorldTransformComponent.self)
    let meshId = ECSHandler.registerComponent(MeshComponent.self)
    let materialId = ECSHandler.registerComponent(MaterialComponent.self)

    Logger.info("Initialised ECS Components")
}

func initHooks() {
    ECSHandler.addHook(
        TransformComponent.self,
        onAdd: { iter in
            let entity = ECSHandler.entitiesFor(iter: iter)
            var worldComponent = WorldTransformComponent(position: .zero, rotation: .identity, scale: .one)
            ECSHandler.addComponent(entity[0], type: WorldTransformComponent.self)
            ECSHandler.setComponent(entity[0], data: &worldComponent)
        },
        onRemove: { _ in 

        }
    )

    Logger.info("Initialised ECS Hooks")
}

func initTags() {
}

func initSystems() {
    ECSHandler.createSystem("CameraSystem", filter: [CameraComponent.self], multiThreaded: false, delegate: cameraSystem)
    ECSHandler.createSystem("HierarchySystem", filter: [TransformComponent.self, WorldTransformComponent.self], multiThreaded: true, delegate: hierarchySystem)
    ECSHandler.createSystem("RenderSystem", filter: [WorldTransformComponent.self, MeshComponent.self, MaterialComponent.self], multiThreaded: true, delegate: renderSystem)
    Logger.info("Initialised ECS Systems")
}

func onAddLookupEntry(key: UnsafePointer<CChar>, value: UnsafeRawPointer) {
    NativeLookupTable.addEntry(key: String(cString: key), ptr: value)
}
