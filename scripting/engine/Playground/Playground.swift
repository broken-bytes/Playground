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
    Time.setup()

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
        var material = MaterialComponent(handle: materialhandle)
        entity.addComponent(&material)
        var mesh = MeshComponent(handle: modelhandle, meshId: 0)
        entity.addComponent(&mesh)

        var scaleValue = Float.random(in: 0.1...1.25)
        var transform = TranslationComponent(position: Vector3(x: Float.random(in: -10...10), y: Float.random(in: -10...10), z: Float.random(in: 10...50)))
        entity.addComponent(&transform)
        var rotation = RotationComponent(rotation: .identity)
        entity.addComponent(&rotation)
        var scale = ScaleComponent(scale: Vector3(x: scaleValue, y: scaleValue, z: scaleValue))
        entity.addComponent(&scale)
    }
}

func initComponents() {
    let componentId = ECSHandler.registerComponent(CameraComponent.self)
    let translationId = ECSHandler.registerComponent(TranslationComponent.self)
    let rotationId = ECSHandler.registerComponent(RotationComponent.self)
    let scaleId = ECSHandler.registerComponent(ScaleComponent.self)
    let worldTranslationId = ECSHandler.registerComponent(WorldTranslationComponent.self)
    let worldRotationId = ECSHandler.registerComponent(WorldRotationComponent.self)
    let worldScaleId = ECSHandler.registerComponent(WorldScaleComponent.self)
    let meshId = ECSHandler.registerComponent(MeshComponent.self)
    let materialId = ECSHandler.registerComponent(MaterialComponent.self)

    Logger.info("Initialised ECS Components")
}

func initHooks() {
    ECSHandler.addHook(
        TranslationComponent.self,
        onAdd: { iter in
            let entity = ECSHandler.entitiesFor(iter: iter)
            var worldComponent = WorldTranslationComponent(position: .zero)
            ECSHandler.addComponent(entity[0], type: WorldTranslationComponent.self)
            ECSHandler.setComponent(entity[0], data: &worldComponent)
        },
        onRemove: { _ in 

        }
    )

    ECSHandler.addHook(
        RotationComponent.self,
        onAdd: { iter in
            let entity = ECSHandler.entitiesFor(iter: iter)
            var worldComponent = WorldRotationComponent(rotation: .identity)
            ECSHandler.addComponent(entity[0], type: WorldRotationComponent.self)
            ECSHandler.setComponent(entity[0], data: &worldComponent)
        },
        onRemove: { _ in 

        }
    )

    ECSHandler.addHook(
        ScaleComponent.self,
        onAdd: { iter in
            let entity = ECSHandler.entitiesFor(iter: iter)
            var worldComponent = WorldScaleComponent(scale: .one)
            ECSHandler.addComponent(entity[0], type: WorldScaleComponent.self)
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
    ECSHandler.createSystem("TransformSystem", filter: [TranslationComponent.self], multiThreaded: true, delegate: transformSystem)
    ECSHandler.createSystem("HierarchySystem", filter: [TranslationComponent.self, RotationComponent.self, ScaleComponent.self, WorldTranslationComponent.self, WorldRotationComponent.self, WorldScaleComponent.self], multiThreaded: true, delegate: hierarchySystem)
    ECSHandler.createSystem("RenderSystem", filter: [WorldTranslationComponent.self, WorldRotationComponent.self, WorldScaleComponent.self, MeshComponent.self, MaterialComponent.self], multiThreaded: true, delegate: renderSystem)
    Logger.info("Initialised ECS Systems")
}

func onAddLookupEntry(key: UnsafePointer<CChar>, value: UnsafeRawPointer) {
    NativeLookupTable.addEntry(key: String(cString: key), ptr: value)
}
