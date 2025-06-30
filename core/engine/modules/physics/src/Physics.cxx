#include "physics/Physics.hxx"
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
#include <thread>
#include <cstdint>
#include <stdint.h>
#include <iostream>

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


    class PhysxCpuDispatcher : public physx::PxCpuDispatcher {
    public:
        PhysxCpuDispatcher() {
            _isRunning = true;

            _threads[0] = std::thread([&]() {
                while (_isRunning) {
                    physx::PxBaseTask* task;
                    if (_tasks.try_dequeue(task)) {
                        task->addReference();
                        task->run();
                        task->release();
                    }
                }
                });

            _threads[1] = std::thread([&]() {
                while (_isRunning) {
                    physx::PxBaseTask* task;
                    if (_tasks.try_dequeue(task)) {
                        task->run();
                        task->release();
                    }
                }
                });
        }

        void Stop() {
            _isRunning = false;
            _threads[0].join();
            _threads[1].join();
        }

        void submitTask(physx::PxBaseTask& task) override {
            _tasks.enqueue(&task);
        }

        uint32_t getWorkerCount() const override {
            return 2;
        }

    private:
        bool _isRunning;
        std::array<std::thread, 2> _threads;
        moodycamel::ConcurrentQueue<physx::PxBaseTask*> _tasks;
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
    ArenaType arena(32 * 1024 * 1024); // 32MB Physics Objects
    Allocator alloc(&arena, "Physics Allocator");
    eastl::hash_map<uint64_t, physx::PxShape*, eastl::hash<uint64_t>, eastl::equal_to<uint64_t>, Allocator, false> shapes(alloc);
    eastl::hash_map<uint64_t, physx::PxRigidBody*, eastl::hash<uint64_t>, eastl::equal_to<uint64_t>, Allocator, false> bodies(alloc);

    eastl::vector<physx::PxMaterial*, Allocator> materials(alloc);
    eastl::vector<uint32_t, Allocator> freeMaterialIds(alloc);

    void Init() {
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
        desc.cpuDispatcher = new PhysxCpuDispatcher();
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
    }

    void Update(double fixedDelta) {
        scene->simulate(fixedDelta);
    }

    void Shutdown() {
        scene->release();
    }

    physics::PhysicsMaterialHandle CreateMaterial(float staticFriction, float dynamicFriction, float restitution) {
        uint32_t materialId = 0;
        if (freeMaterialIds.size() > 0) {
            materialId = freeMaterialIds.back();
            materials[materialId] = physics->createMaterial(staticFriction, dynamicFriction, restitution);
            freeMaterialIds.pop_back();
        }
        else {
            materials.push_back(physics->createMaterial(staticFriction, dynamicFriction, restitution));
            materialId = materials.size() - 1;
        }

        return materialId;
    }

    void AddBody(uint64_t entityId, float mass, float damping, glm::vec3 position, glm::vec4 rotation) {
        if (bodies.find(entityId) != bodies.end()) {
            std::cerr << "Entity" << +entityId << " already has a body attached" << std::endl;
            return;
        }

        auto transform = physx::PxTransform(position.x, position.y, position.z, physx::PxQuat(rotation.w, rotation.x, rotation.y, rotation.z));
        physx::PxRigidDynamic* rigidDynamic = physics->createRigidDynamic(transform);
        rigidDynamic->setMass(mass);
        rigidDynamic->setLinearDamping(damping);
        scene->addActor(*rigidDynamic);
    }

    void AddBoxCollider(uint64_t entityId, uint32_t materialId, glm::vec4 rotation, glm::vec3 dimensions, glm::vec3 offset) {
        if (shapes.find(entityId) != shapes.end()) {
            std::cerr << "Entity" << +entityId << " already has a shape attached" << std::endl;
            return;
        }

        physx::PxBoxGeometry boxGeom(dimensions.x * 0.5f, dimensions.y * 0.5f, dimensions.z * 0.5f);
        physx::PxShape* shape = physics->createShape(boxGeom, *materials[materialId]);

        physx::PxQuat offsetRot(rotation.w, rotation.x, rotation.y, rotation.z);
        physx::PxVec3 offsetPos(offset.x, offset.y, offset.z);
        physx::PxTransform localPose(offsetPos, offsetRot);

        shape->setLocalPose(localPose);
    }

    void AddShapeToBody(uint64_t body, uint64_t shape) {
        auto collider = shapes.at(shape);
        auto actor = bodies.at(body);

        actor->attachShape(*collider);
    }
}

