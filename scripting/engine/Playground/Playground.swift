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
    let modelhandle = AssetHandler.loadModel(named: "default.mod")
    let physicsMaterialHandle = AssetHandler.loadPhysicsMaterial(named: "default.pmat")

    let entity = Entity("Sun")
    var sun = SunComponent(direction: Vector3(x: 0.1, y: -0.5, z: 0.2), colour: Colour(r: 1, g: 1, b: 1, a: 1), intensity: 3)
    entity.addComponent(&sun)

    let camEntity = Entity("Camera")
    var cam = CameraComponent(order: 0, fov: 70, nearPlane: 0.01, farPlane: 1000)
    camEntity.addComponent(&cam)
    var scaleValue = Float.random(in: 0.1...1.25)
    var transform = TranslationComponent(position: Vector3(x: 0, y: 0, z: 0))
    camEntity.addComponent(&transform)
    var rotation = RotationComponent(rotation: .identity)
    camEntity.addComponent(&rotation)
    var scale = ScaleComponent(scale: Vector3(x: scaleValue, y: scaleValue, z: scaleValue))
    camEntity.addComponent(&scale)

    for x in 0..<10000 {
        let entity = Entity("Entity\(x)")
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
    let sunId = ECSHandler.registerComponent(SunComponent.self)

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
    ECSHandler.createSystem("CameraMoveSystem", filter: [TranslationComponent.self, RotationComponent.self, CameraComponent.self], multiThreaded: false, delegate: cameraMoveTest)
    ECSHandler.createSystem("HierarchySystem", filter: [TranslationComponent.self, RotationComponent.self, ScaleComponent.self, WorldTranslationComponent.self, WorldRotationComponent.self, WorldScaleComponent.self], multiThreaded: true, delegate: hierarchySystem)
    ECSHandler.createSystem("RenderSystem", filter: [WorldTranslationComponent.self, WorldRotationComponent.self, WorldScaleComponent.self, MeshComponent.self, MaterialComponent.self], multiThreaded: true, delegate: renderSystem)
    ECSHandler.createSystem("SunSystem", filter: [SunComponent.self], multiThreaded: false, delegate: sunSystem)
    ECSHandler.createSystem("CameraSystem", filter: [CameraComponent.self, WorldTranslationComponent.self, WorldRotationComponent.self], multiThreaded: false, delegate: cameraSystem)

    Logger.info("Initialised ECS Systems")
}

func onAddLookupEntry(key: UnsafePointer<CChar>, value: UnsafeRawPointer) {
    NativeLookupTable.addEntry(key: String(cString: key), ptr: value)
}
