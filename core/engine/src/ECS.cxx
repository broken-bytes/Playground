#include "playground/ECS.hxx"
#include "playground/Constants.hxx"
#include <shared/JobSystem.hxx>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <flecs/os_api.h>

namespace playground::ecs {
    flecs::world world;
    std::mutex writeLock;
    std::shared_mutex readLock;

    std::atomic<uint64_t> taskId = { 0 };

    std::map<uint64_t, std::shared_ptr<jobsystem::JobHandle>> jobs;

    ecs_os_thread_t SpawnTask(ecs_os_thread_callback_t callback, void* param) {
        auto taskIdValue = taskId.fetch_add(1);
        auto job = jobsystem::JobHandle::Create("Flecs Task" + taskId, jobsystem::JobPriority::High, [callback]() { callback(nullptr); });
        jobs.insert({ taskIdValue, job });

        jobsystem::Submit(job);

        return taskIdValue;
    };

    void* JoinTask(ecs_os_thread_t thread) {
        auto job = jobs[thread];
        while (!job->IsDone()) {
            // Wait for the job to complete
        }
        return nullptr;
    };

    void Init(int tickRate, bool debugServer) {
        ecs_os_set_api_defaults();
        auto api = ecs_os_get_api();
        api.thread_new_ = SpawnTask;
        api.thread_join_ = JoinTask;
        api.thread_self_ = []() -> ecs_os_thread_id_t {
            return std::this_thread::get_id()._Get_underlying_id();
            };
        ecs_os_set_api(&api);

        if (debugServer) {
            world.import<flecs::stats>();
            world.set<flecs::Rest>({ });
            // Get the number of threads on the system
        }

        world.set_threads(jobsystem::HighPerfWorkers());

        world.set_target_fps(tickRate);

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

            world.script().code(value.c_str()).run();
        }
    }

    void Update(double deltaTime) {
        jobs.clear();
        taskId.store(0);

        world.progress(deltaTime);
    }

    void Shutdown() {
        world.quit();

        while (world.progress(0)) {
        }

        ecs_fini(world);
    }

    uint64_t CreateEntity(const char* name) {
        return world.entity(name);
    }

    void DestroyEntity(uint64_t entityId) {
        world.entity(entityId).destruct();
    }

    void SetParent(uint64_t childId, uint64_t parentId) {
        world.entity(childId).remove(flecs::ChildOf);
        // Make entity a child of the new parent
        world.entity(childId).add(flecs::ChildOf, parentId);
    }

    uint64_t GetParent(uint64_t childId) {
        return world.entity(childId).parent();
    }

    uint64_t GetEntityByName(const char* name) {
        auto entity = world.lookup(name);
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

        desc.entity = ecs_entity_init(world, &entityDesc);

        auto id = ecs_component_init(world, &desc);

        return id;
    }

    void AddComponent(uint64_t entityId, uint64_t componentId) {
        ecs_add_id(world, entityId, componentId);
    }

    void SetComponent(uint64_t entityId, uint64_t componentId, const void* data) {
        world.entity(entityId).set_ptr(componentId, data);
    }

    const void* GetComponent(uint64_t entityId, uint64_t componentId) {
        return world.entity(entityId).get(componentId);
    }

    bool HasComponent(uint64_t entityId, uint64_t componentId) {
        return world.entity(entityId).has(componentId);
    }

    void DestroyComponent(uint64_t entityId, uint64_t componentId) {
        ecs_remove_id(world, entityId, componentId);
    }

    uint64_t CreateSystem(const char* name, uint64_t* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate) {
        ecs_system_desc_t system = {};

        ecs_entity_desc_t entity = {};
        entity.name = name;
        auto depends = ecs_pair(EcsDependsOn, EcsOnUpdate);
        std::vector<ecs_id_t> ids = { depends, 0 };
        entity.add = ids.data();

        ecs_query_desc_t query = {};
        for (int x = 0; x < filterCount; x++) {
            query.terms[x] = ecs_term_t{ .id = filter[x], .inout = EcsInOut, .oper = EcsAnd };
        }

        memset(system.query.terms, 0, sizeof(system.query.terms));
        auto entityId = ecs_entity_init(world, &entity);

        system.query = query;
        system.entity = entityId;
        system.multi_threaded = isParallel;
        system.callback = delegate;

        return ecs_system_init(world, &system);
    }

    void* GetComponentBuffer(ecs_iter_t* iter, uint32_t index, size_t componentSize, size_t* numItems) {
        auto ptr = ecs_field_w_size(iter, componentSize, index);

        *numItems = iter->count;

        return ptr;
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

        ecs_set_hooks_id(world, componentId, &hooks);
    }

    void DeleteAllEntitiesByTag(uint64_t tag) {
        ecs_delete_with(world, tag);
    }

    uint64_t CreateTag(const char* name) {
        auto entity = ecs_new(world);
        ecs_set_name(world, entity, name);

        return entity;
    }

    void AddTag(uint64_t entityId, uint64_t tagId) {
        ecs_add_id(world, entityId, tagId);
    }
}
