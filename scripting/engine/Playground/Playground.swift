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

    Physics.setup()
    Logger.info("Started Physics")

    NativeMath.setup()
    Time.setup()

    initComponents()
    initTags()
    initSystems()

    let materialhandle = AssetHandler.loadMaterial(named: "default.mat")
    let groundMaterialhandle = AssetHandler.loadMaterial(named: "ground.mat")
    let modelhandle = AssetHandler.loadModel(named: "default.mod")
    let physicsMaterialHandle = AssetHandler.loadPhysicsMaterial(named: "default.pmat")

    let entity = Entity("Sun")
    var sun = SunComponent(direction: Vector3(x: 0.1, y: -0.5, z: 0.2), colour: Colour(r: 1, g: 1, b: 1, a: 1), intensity: 1)
    entity.addComponent(&sun)

    let camEntity = Entity("Camera")
    var cam = CameraComponent(order: 0, fov: 70, nearPlane: 0.01, farPlane: 250)
    camEntity.addComponent(&cam)
    var transform = TranslationComponent(position: Vector3(x: 0, y: 0, z: 0))
    camEntity.addComponent(&transform)
    var rotation = RotationComponent(rotation: .identity)
    camEntity.addComponent(&rotation)
    var scale = ScaleComponent(scale: Vector3(x: 1, y: 1, z: 1))
    camEntity.addComponent(&scale)
    var listener = AudioListenerComponent(index: 0)
    camEntity.addComponent(&listener)

    let floor = Entity("Floor")
    var floorMaterial = MaterialComponent(handle: groundMaterialhandle)
    floor.addComponent(&floorMaterial)
    var floorMesh = MeshComponent(handle: modelhandle, meshId: 0)
    floor.addComponent(&floorMesh)

    var floorTransform = TranslationComponent(position: Vector3(x: 0, y: -10, z: 0))
    floor.addComponent(&floorTransform)
    var floorRotation = RotationComponent(rotation: .identity)
    floor.addComponent(&floorRotation)
    var floorScale = ScaleComponent(scale: Vector3(x: 50, y: 0.5, z: 50))
    floor.addComponent(&floorScale)

    var staticBody = StaticBodyComponent()
    floor.addComponent(&staticBody)

    var boxCollider = BoxColliderComponent(isTrigger: false, dimensions: Vector3(x: 100, y: 0.5, z: 100), offset: .zero, rotation: .identity, material: physicsMaterialHandle)
    floor.addComponent(&boxCollider)

    for x in 0..<1 {
        let entity = Entity("Entity\(x)")
        var material = MaterialComponent(handle: materialhandle)
        entity.addComponent(&material)
        var mesh = MeshComponent(handle: modelhandle, meshId: 0)
        entity.addComponent(&mesh)

        var transform = TranslationComponent(position: Vector3(x: Float.random(in: -100...100), y: Float.random(in: 10...20), z: Float.random(in: -100...100)))
        entity.addComponent(&transform)
        var rotation = RotationComponent(rotation: .identity)
        entity.addComponent(&rotation)
        var scale = ScaleComponent(scale: Vector3(x: 1, y: 1, z: 1))
        entity.addComponent(&scale)

        var rigidbody = RigidBodyComponent(mass: 5, damping: 0.1)
        entity.addComponent(&rigidbody)

        var boxCollider = BoxColliderComponent(isTrigger: false, dimensions: .one, offset: .zero, rotation: .identity, material: physicsMaterialHandle)
        entity.addComponent(&boxCollider)

        var audioSource = AudioSourceComponent(with: "event:/Ambient/motor_loop_1")
        entity.addComponent(&audioSource)
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
