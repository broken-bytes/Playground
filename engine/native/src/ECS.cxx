#include "playground/ECS.hxx"
#include "playground/PhysicsManager.hxx"
#include "playground/Constants.hxx"
#include "playground/systems/RenderSystem.hxx"
#include "playground/systems/HierarchySystem.hxx"
#include "playground/systems/BoxColliderUpdateSystem.hxx"
#include "playground/systems/RigidBodyUpdateSystem.hxx"
#include "playground/systems/StaticBodyUpdateSystem.hxx"
#include "playground/systems/AudioSourceSystem.hxx"
#include "playground/systems/AudioListenerSystem.hxx"
#include "playground/components/TranslationComponent.hxx"
#include "playground/components/WorldTranslationComponent.hxx"
#include "playground/components/RotationComponent.hxx"
#include "playground/components/WorldRotationComponent.hxx"
#include "playground/components/ScaleComponent.hxx"
#include "playground/components/WorldScaleComponent.hxx"
#include "playground/components/MeshComponent.hxx"
#include "playground/components/MaterialComponent.hxx"
#include "playground/components/BoxColliderComponent.hxx"
#include "playground/components/RigidBodyComponent.hxx"
#include "playground/components/StaticBodyComponent.hxx"
#include "playground/components/AudioSourceComponent.hxx"
#include "playground/components/AudioListenerComponent.hxx"
#include <shared/JobSystem.hxx>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <memory>
#include <minmax.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <flecs/os_api.h>
#include <tracy/Tracy.hpp>

namespace playground::ecs {
    std::unique_ptr<flecs::world> world;
    std::mutex writeLock;
    std::shared_mutex readLock;

    std::atomic<uint64_t> taskId = { 0 };

    std::vector<std::shared_ptr<jobsystem::JobHandle>> jobs;

#if EDITOR
    CreateEntityHook createEntityHook = nullptr;
    DestroyEntityHook destroyEntityHook = nullptr;
    SetParentHook setParentHook = nullptr;
#endif

    uint64_t CreateSystem(const char* name, Filter* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate, ecs_entity_t dependsOn);
    void RegisterComponents();
    void RegisterSystems();

    void AttachChildrenCollidersRigid(flecs::entity e, RigidBodyComponent& body) {
        e.children([&](flecs::entity child) {
            if (child.has<BoxColliderComponent>()) {
                auto boxCollider = child.get<BoxColliderComponent>();
                if (boxCollider.handle != UINT64_MAX) {
                    physicsmanager::AttachCollider(body.handle, boxCollider.handle);
                }
            }

            AttachChildrenCollidersRigid(child, body);
        });
    }

    void AttachChildrenCollidersStatic(flecs::entity e, StaticBodyComponent& body) {
        e.children([&](flecs::entity child) {
            if (child.has<BoxColliderComponent>()) {
                auto boxCollider = child.get<BoxColliderComponent>();
                if (boxCollider.handle != UINT64_MAX) {
                    physicsmanager::AttachCollider(body.handle, boxCollider.handle);
                }
            }

            AttachChildrenCollidersStatic(child, body);
        });
    }

    void RegisterComponents() {
        playground::ecs::GetWorld().component<TranslationComponent>("::TranslationComponent")
            .on_add([](flecs::entity e, TranslationComponent) {
                e.add<WorldTranslationComponent>();
            });
        playground::ecs::GetWorld().component<WorldTranslationComponent>("::WorldTranslationComponent");
        playground::ecs::GetWorld().component<RotationComponent>("::RotationComponent")
            .on_add([](flecs::entity e, RotationComponent) {
                e.add<WorldRotationComponent>();
            });
        playground::ecs::GetWorld().component<WorldRotationComponent>("::WorldRotationComponent");
        playground::ecs::GetWorld().component<ScaleComponent>("::ScaleComponent")
            .on_add([](flecs::entity e, ScaleComponent) {
                e.add<WorldScaleComponent>();
            });
        playground::ecs::GetWorld().component<WorldScaleComponent>("::WorldScaleComponent");
        playground::ecs::GetWorld().component<MeshComponent>("::MeshComponent");
        playground::ecs::GetWorld().component<MaterialComponent>("::MaterialComponent");
        playground::ecs::GetWorld().component<BoxColliderComponent>("::BoxColliderComponent");
        playground::ecs::GetWorld().component<RigidBodyComponent>("::RigidBodyComponent");
        playground::ecs::GetWorld().component<StaticBodyComponent>("::StaticBodyComponent");
        playground::ecs::GetWorld().component<AudioSourceComponent>("::AudioSourceComponent");
        playground::ecs::GetWorld().component<AudioListenerComponent>("::AudioListenerComponent");
    }

    void RegisterSystems() {
        playground::ecs::boxcolliderupdatesystem::Init(*world);
        playground::ecs::rigidbodyupdatesystem::Init(*world);
        playground::ecs::staticbodyupdatesystem::Init(*world);
        playground::ecs::hierarchysystem::Init(*world);
        playground::ecs::audiosourcesystem::Init(*world);
        playground::ecs::audiolistenersystem::Init(*world);
        playground::ecs::rendersystem::Init(*world);
    }

    ecs_os_thread_t SpawnTask(ecs_os_thread_callback_t callback, void* param) {
        ZoneScopedNC("FLECS: Spawn Task", tracy::Color::Red);

        auto job = jobsystem::Job{
            .Name = "FLECS_WORKER",
            .Priority = jobsystem::JobPriority::High,
            .Color = tracy::Color::Red,
            .Dependencies = {},
            .Task = [callback, param](uint8_t workerId) { callback(param); }
        };

        std::scoped_lock lock{ writeLock };

        jobs.push_back(jobsystem::Submit(job));

        return jobs.size();
    };

    void* JoinTask(ecs_os_thread_t thread) {
        ZoneScopedNC("FLECS: Join Task", tracy::Color::Red);
        auto job = jobs.at(thread - 1);
        while(!job->IsDone()) {
            std::this_thread::yield();
        }

        return nullptr;
    };

    void Init(bool debugServer) {
        ecs_os_set_api_defaults();
        auto api = ecs_os_get_api();
        api.task_new_ = SpawnTask;
        api.task_join_ = JoinTask;
        api.thread_self_ = []() -> ecs_os_thread_id_t {
            return std::this_thread::get_id()._Get_underlying_id();
            };
        ecs_os_set_api(&api);

        world = std::make_unique<flecs::world>();

        if (debugServer) {
            world->import<flecs::stats>();
            world->set<flecs::Rest>({ });
            // Get the number of threads on the system
        }

        world->set_task_threads(jobsystem::HighPerfWorkers());

        if (!std::filesystem::exists("meta")) {
            std::filesystem::create_directory("meta");
        }

        auto files = std::vector<std::filesystem::directory_entry>();

        for (auto& p : std::filesystem::directory_iterator(std::filesystem::current_path() / "meta")) {
            files.push_back(p);
        }

        for (auto& file : files) {
            std::ifstream fileStream(file.path());
            if (!fileStream.is_open()) {
                std::cerr << "Failed to open: " << file.path() << std::endl;
                continue;
            }

            std::stringstream buffer;
            buffer << fileStream.rdbuf(); // Read entire file into buffer
            std::string value = buffer.str();

            world->script().code(value.c_str()).run();
        }

        jobs = {};

        RegisterComponents();
        RegisterSystems();
    }

    void Update(double deltaTime) {
        jobs.clear();

        world->progress(deltaTime);
    }

    void Clear() {
        world->each([](flecs::entity e) {
            if (e.is_alive()) {
                e.destruct();
            }
        });
    }

    void Shutdown() {
        world->quit();

        ecs_fini(*world);
    }

    flecs::world& GetWorld() {
        return *world;
    }

    uint64_t CreateEntity(const char* name) {
#if EDITOR
        auto entity = world->entity(name);

        if (createEntityHook) {
            createEntityHook(entity.id(), name);
        }

        return entity.id();
#else
        return world->entity(name);
#endif
    }

    void DestroyEntity(uint64_t entityId) {
        world->entity(entityId).destruct();
#if EDITOR
        if (destroyEntityHook) {
            destroyEntityHook(entityId);
        }
#endif
    }

    void SetParent(uint64_t childId, uint64_t parentId) {
        world->entity(childId).remove(flecs::ChildOf);
        // Make entity a child of the new parent
        world->entity(childId).add(flecs::ChildOf, parentId);
#if EDITOR
        if (setParentHook) {
            setParentHook(childId, parentId);
        }
#endif
    }

    uint64_t GetParent(uint64_t childId) {
        return world->entity(childId).parent();
    }

    uint64_t GetEntityByName(const char* name) {
        auto entity = world->lookup(name);
        if (entity) {
            return entity.id();
        }

        return 0;
    }

    uint64_t RegisterComponent(const char* name, size_t size, size_t alignment) {
        auto desc = ecs_component_desc_t{};
        desc.type = {};
        desc.type.size = size;
        desc.type.alignment = alignment;
        desc.type.name = name;

        ecs_entity_desc_t entityDesc = {};
        entityDesc.name = name;

        desc.entity = ecs_entity_init(*world, &entityDesc);

        auto id = ecs_component_init(*world, &desc);

        return id;
    }

    void AddComponent(uint64_t entityId, uint64_t componentId) {
        ecs_add_id(*world, entityId, componentId);
    }

    void SetComponent(uint64_t entityId, uint64_t componentId, const void* data) {
        world->entity(entityId).set_ptr(componentId, data);
    }

    const void* GetComponent(uint64_t entityId, uint64_t componentId) {
        return world->entity(entityId).get(componentId);
    }

    bool HasComponent(uint64_t entityId, uint64_t componentId) {
        return world->entity(entityId).has(componentId);
    }

    void DestroyComponent(uint64_t entityId, uint64_t componentId) {
        ecs_remove_id(*world, entityId, componentId);
    }

    uint64_t CreatePreUpdateSystem(const char* name, Filter* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate) {
        return CreateSystem(name, filter, filterCount, isParallel, delegate, EcsPreUpdate);
    }

    uint64_t CreateUpdateSystem(const char* name, Filter* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate) {
        return CreateSystem(name, filter, filterCount, isParallel, delegate, EcsOnUpdate);
    }

    uint64_t CreatePostUpdateSystem(const char* name, Filter* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate) {
        return CreateSystem(name, filter, filterCount, isParallel, delegate, EcsPostUpdate);
    }

    void* GetComponentBuffer(ecs_iter_t* iter, uint32_t index, size_t componentSize, size_t* numItems) {
        auto ptr = ecs_field_w_size(iter, componentSize, index);

        *numItems = iter->count;

        return ptr;
    }

    uint64_t GetIteratorSystem(ecs_iter_t* iter)
    {
        return iter->system;
    }

    uint64_t GetIteratorSize(ecs_iter_t* iter) {
        return iter->count;
    }

    uint64_t GetIteratorOffset(ecs_iter_t* iter) {
        return iter->offset;
    }

    const uint64_t* GetEntitiesFromIterator(ecs_iter_t* iter, size_t* size) {
        *size = iter->count;

        return iter->entities;
    }

    void CreateHook(uint64_t componentId, ComponentLifetimeDelegate onAdd, ComponentLifetimeDelegate onRemove) {
        ecs_type_hooks_t hooks = {};
        if (onAdd) {
            hooks.on_add = onAdd;
        }
        if (onRemove) {
            hooks.on_remove = onRemove;
        }

        ecs_set_hooks_id(*world, componentId, &hooks);
    }

    void DeleteAllEntitiesByTag(uint64_t tag) {
        ecs_delete_with(*world, tag);
    }

    uint64_t CreateTag(const char* name) {
        auto entity = ecs_new(*world);
        ecs_set_name(*world, entity, name);

        return entity;
    }

    void AddTag(uint64_t entityId, uint64_t tagId) {
        ecs_add_id(*world, entityId, tagId);
    }

    uint64_t CreateSystem(const char* name, Filter* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate, ecs_entity_t dependsOn) {
        ecs_system_desc_t system = {};

        ecs_entity_desc_t entity = {};
        entity.name = name;
        auto depends = ecs_pair(EcsDependsOn, dependsOn);
        std::vector<ecs_id_t> ids = { depends, 0 };
        entity.add = ids.data();

        ecs_query_desc_t query = {};
        for (int x = 0; x < filterCount; x++) {
            query.terms[x] = ecs_term_t{ .id = filter[x].filterComponentId, .inout = filter[x].filterUsage, .oper = filter[x].filterOperation };
        }

        memset(system.query.terms, 0, sizeof(system.query.terms));
        auto entityId = ecs_entity_init(*world, &entity);

        system.query = query;
        system.entity = entityId;
        system.multi_threaded = isParallel;
        system.callback = delegate;

        return ecs_system_init(*world, &system);
    }

#if EDITOR
    void SetEntityCreateHook(CreateEntityHook hook) {
        createEntityHook = hook;
    }

    void SetEntityDestroyHook(DestroyEntityHook hook) {
        destroyEntityHook = hook;
    }

    void SetEntitySetParentHook(SetParentHook hook) {
        setParentHook = hook;
    }
#endif
}
