#include "physics/Physics.hxx"
#include <shared/Hardware.hxx>
#include <shared/JobSystem.hxx>
#include <PxPhysics.h>
#include <PxConfig.h>
#include <PxActor.h>
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxMaterial.h>
#include <PxScene.h>
#include <PxSceneDesc.h>
#include <pvd/PxPvd.h>
#include <pvd/PxPvdTransport.h>
#include <pvd/PxPvdSceneClient.h>
#include <task/PxTask.h>
#include <foundation/PxPhysicsVersion.h>
#include <foundation/PxFoundation.h>
#include <foundation/PxErrorCallback.h>
#include <common/PxTolerancesScale.h>
#include <shared/Arena.hxx>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>
#include <concurrentqueue.h>
#include <array>
#include <mutex>
#include <thread>
#include <cstdint>
#include <stdint.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <functional>
#include <vector>
#include <shared_mutex>
#include <tracy/Tracy.hpp>

namespace playground::physics {
    class PhysXAllocator : public physx::PxAllocatorCallback {
    public:
        void* allocate(size_t size, const char*, const char*, int) override {
            return _aligned_malloc(size, 16);
        }
        void deallocate(void* ptr) override {
            _aligned_free(ptr);
        }
    };

    class PhysxErrorCallback : public physx::PxErrorCallback {
    public:
        void reportError(physx::PxErrorCode::Enum code, const char* msg, const char* file, int line) override {
            printf("PhysX Error [%d]: %s (%s:%d)\n", code, msg, file, line);
        }
    };

    class PhysxWorker {
    public:
        PhysxWorker(uint8_t id, std::function<physx::PxBaseTask*()> pullTask) {
            _id = id;
            _isRunning = true;

            _thread = std::thread([&, pullTask]() {
                while (_isRunning) {
                    if (_isFinished.load()) {
                        continue;
                    }

                    physx::PxBaseTask* task;
                    task = pullTask();

                    while (task != nullptr) {
                        task->run();
                        task->release();

                        task = pullTask();
                    }

                    _isFinished.store(true);
                }
            });
        }

        bool IsFinished() {
            return _jobCounter.load() == 0;
        }

        void Reset() {
            _jobCounter.load() == 0;
        }

        void Stop() {
            while (!_isFinished) {
                // Wait until all scheduled jobs have run
            }

            _isRunning = false;
            _thread.join();
        }

    private:
        std::atomic<uint16_t> _jobCounter{ 0 };
        std::atomic<bool> _isFinished{ false };
        uint8_t _id;
        bool _isRunning;
        std::thread _thread;
    };

    class PhysxCpuDispatcher : public physx::PxCpuDispatcher {
    public:
        PhysxCpuDispatcher() {
        }

        void Stop() {
            while (!IsFinished()) {
                // Wait until all tasks have finished
            }
        }

        void submitTask(physx::PxBaseTask& task) override {
            auto index = _jobCounter.fetch_add(1);
            std::stringstream ss;
            ss << "PHYSICS_JOB" << +index;
            auto job = jobsystem::JobHandle::Create(ss.str(), jobsystem::JobPriority::High, tracy::Color::Pink1, [&task, this]() {
                task.run();
                task.release();
                _jobCounter.fetch_sub(1);
            });

            jobsystem::Submit(job);
        }

        void Reset() {

        }

        bool IsFinished() {
            return _jobCounter.load() == 0;
        }

        uint32_t getWorkerCount() const override {
            return jobsystem::HighPerfWorkers();
        }

    private:
        std::atomic<uint16_t> _jobCounter{ 0 };
    };

    physx::PxFilterFlags DefaultFilter(
        physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags, const void*, physx::PxU32)
    {
        // Let everything collide by default
        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

        // Optionally, enable triggers
        if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
            pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;

        return physx::PxFilterFlag::eDEFAULT;
    }

    PhysXAllocator allocator;
    PhysxErrorCallback errorCallback;
    physx::PxFoundation* foundation;
    physx::PxPhysics* physics = nullptr;
    physx::PxScene* scene = nullptr;

    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;
    ArenaType arena(128 * 1024 * 1024); // 128MB Physics Objects
    Allocator alloc(&arena, "Physics Allocator");
    eastl::vector<physx::PxShape*, Allocator> shapes(alloc);
    moodycamel::ConcurrentQueue<uint64_t> freeShapeIdsQueue;
    eastl::vector<physx::PxRigidActor*, Allocator> bodies(alloc);
    moodycamel::ConcurrentQueue<uint64_t> freeBodyIdsQueue;
    eastl::vector<physx::PxMaterial*, Allocator> materials(alloc);
    moodycamel::ConcurrentQueue<uint32_t> freeMaterialIdsQueue;

    std::unique_ptr<PhysxCpuDispatcher> dispatcher;
    std::shared_mutex physxMutex;

    bool isRunning = false;

    void Init() {
        isRunning = true;
        foundation = PxCreateFoundation(
            PX_PHYSICS_VERSION,
            allocator,
            errorCallback
        );
        if (!foundation) {
            exit(2);
        }

        auto tolerance = physx::PxTolerancesScale();

        physx::PxPvd* pvd = physx::PxCreatePvd(*foundation);
        physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

        physics = PxCreatePhysics(
            PX_PHYSICS_VERSION,
            *foundation,
            tolerance,
            false,
            pvd,
            nullptr
        );

        if (!physics) {
            std::cerr << "Failed to create PhysX instance." << std::endl;
            exit(2);
        }

        auto desc = physx::PxSceneDesc(physics->getTolerancesScale());
        desc.gravity = physx::PxVec3{ 0 , -9, 0 };

        dispatcher = std::make_unique<PhysxCpuDispatcher>();

        desc.cpuDispatcher = dispatcher.get();
        desc.filterShader = DefaultFilter;
        desc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
        desc.flags |= physx::PxSceneFlag::eENABLE_CCD;
        desc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;

        scene = physics->createScene(desc);

        if (!scene) {
            std::cerr << "Failed to create PhysX scene." << std::endl;
            physics->release();
            foundation->release();
            exit(2);
        }

        scene->getScenePvdClient()->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        scene->getScenePvdClient()->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        scene->getScenePvdClient()->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    void Update(double fixedDelta) {
        scene->simulate(fixedDelta, nullptr);
        uint32_t error;

        while (!dispatcher->IsFinished()) {
            std::this_thread::yield();
        }

        dispatcher->Reset();

        scene->fetchResults(true, &error);

        if (error != 0) {
            std::cerr << "PhysX scene fetch results error: " << error << std::endl;
        }
    }

    void Shutdown() {
        isRunning = false;
    }

    physics::PhysicsMaterialHandle CreateMaterial(float staticFriction, float dynamicFriction, float restitution) {
        uint64_t materialId = 0;
        if (freeMaterialIdsQueue.try_dequeue(materialId)) {
            materials[materialId] = physics->createMaterial(staticFriction, dynamicFriction, restitution);
        }
        else {
            materials.push_back(physics->createMaterial(staticFriction, dynamicFriction, restitution));
            materialId = materials.size() - 1;
        }

        return materialId;
    }

    uint64_t CreateRigidBody(float mass, float damping, math::Vector3 position, math::Quaternion rotation) {
        std::unique_lock lock(physxMutex);

        auto transform = physx::PxTransform(position.X, position.Y, position.Z, physx::PxQuat(rotation.X, rotation.Y, rotation.Z, rotation.W));
        physx::PxRigidDynamic* rigidDynamic = physics->createRigidDynamic(transform);
        rigidDynamic->setMass(mass);
        rigidDynamic->setLinearDamping(damping);

        scene->addActor(*rigidDynamic);

        uint64_t bodyId = 0;
        if (freeBodyIdsQueue.try_dequeue(bodyId)) {
            bodies[bodyId] = rigidDynamic;
        }
        else {
            bodies.push_back(rigidDynamic);
            bodyId = bodies.size() - 1;
        }

        return bodyId;
    }

    uint64_t CreateStaticBody(math::Vector3 position, math::Quaternion rotation) {
        std::unique_lock lock(physxMutex);

        auto transform = physx::PxTransform(position.X, position.Y, position.Z, physx::PxQuat(rotation.X, rotation.Y, rotation.Z, rotation.W));
        physx::PxRigidStatic* rigidStatic = physics->createRigidStatic(transform);

        scene->addActor(*rigidStatic);

        uint64_t bodyId = 0;
        if (freeBodyIdsQueue.try_dequeue(bodyId)) {
            bodies[bodyId] = rigidStatic;
        }
        else {
            bodies.push_back(rigidStatic);
            bodyId = bodies.size() - 1;
        }

        return bodyId;
    }

    uint64_t CreateBoxCollider(uint32_t materialId, math::Quaternion rotation, math::Vector3 dimensions, math::Vector3 offset) {
        std::unique_lock lock(physxMutex);

        physx::PxBoxGeometry boxGeom(dimensions.X, dimensions.Y, dimensions.Z);
        physx::PxShape* shape = physics->createShape(boxGeom, *materials[materialId]);

        physx::PxQuat offsetRot(rotation.X, rotation.Y, rotation.Z, rotation.W);
        physx::PxVec3 offsetPos(offset.X, offset.Y, offset.Z);
        physx::PxTransform localPose(offsetPos, offsetRot);

        shape->setLocalPose(localPose);

        uint64_t shapeId = 0;
        if (freeShapeIdsQueue.try_dequeue(shapeId)) {
            shapes[shapeId] = shape;
        }
        else {
            shapes.push_back(shape);
            shapeId = shapes.size() - 1;
        }

        return shapeId;
    }

    void AddShapeToBody(uint64_t body, uint64_t shape) {
        std::unique_lock lock(physxMutex);

        auto collider = shapes.at(shape);
        auto actor = bodies.at(body);
        actor->attachShape(*collider);
    }

    void RemoveBody(uint64_t body) {
        std::unique_lock lock(physxMutex);

        auto actor = bodies.at(body);
        if (actor) {
            scene->removeActor(*actor);
            actor->release();
            bodies[body] = nullptr;
            freeBodyIdsQueue.enqueue(body);
        }
    }

    void RemoveShape(uint64_t shape) {
        std::unique_lock lock(physxMutex);

        auto collider = shapes.at(shape);
        if (collider) {
            collider->release();
            shapes[shape] = nullptr;
            freeShapeIdsQueue.enqueue(shape);
        }
    }

    void GetBodyPosition(uint64_t id, math::Vector3* position) {
        std::shared_lock lock(physxMutex);
        auto pos = bodies[id]->getGlobalPose().p;
        position->X = pos.x;
        position->Y = pos.y;
        position->Z = pos.z;
    }

    void GetBodyRotation(uint64_t id, math::Quaternion* rotation) {
        std::shared_lock lock(physxMutex);
        auto rot = bodies[id]->getGlobalPose().q;
        rotation->X = rot.x;
        rotation->Y = rot.y;
        rotation->Z = rot.z;
        rotation->W = rot.w;
    }
}

