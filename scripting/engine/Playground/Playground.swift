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
    Rendering.Renderer.setup()
    Logger.info("Started Renderer")

    Physics.setup()
    Logger.info("Started Physics")

    NativeMath.setup()
    Time.setup()

    initComponents()
    initTags()
    initSystems()
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
    let rigidId = ECSHandler.registerComponent(RigidBodyComponent.self)
    let staticId = ECSHandler.registerComponent(StaticBodyComponent.self)
    let boxId = ECSHandler.registerComponent(BoxColliderComponent.self)

    Logger.info("Initialised ECS Components")
}

func initTags() {
}

func initSystems() {
    ECSHandler.createSystem(
        "CameraMoveSystem", 
        filter: [
            ECSFilter(component: TranslationComponent.self, usage: .write, operation: .and), 
            ECSFilter(component: RotationComponent.self, usage: .write, operation: .and), 
            ECSFilter(component: CameraComponent.self, usage: .write, operation: .and)
        ],
        multiThreaded: false, 
        delegate: cameraMoveTest
    )
    ECSHandler.createSystem(
        "CameraSystem", 
        filter: [
            ECSFilter(component: CameraComponent.self, usage: .write, operation: .and), 
            ECSFilter(component: WorldTranslationComponent.self, usage: .write, operation: .and), 
            ECSFilter(component: WorldRotationComponent.self, usage: .write, operation: .and)
        ],
            multiThreaded: false, 
            delegate: cameraSystem
        )
    ECSHandler.createSystem("SunSystem", filter: [ECSFilter(component: SunComponent.self, usage: .write, operation: .and)], multiThreaded: false, delegate: sunSystem)

    Logger.info("Initialised ECS Systems")
}

func onAddLookupEntry(key: UnsafePointer<CChar>, value: UnsafeRawPointer) {
    NativeLookupTable.addEntry(key: String(cString: key), ptr: value)
}
