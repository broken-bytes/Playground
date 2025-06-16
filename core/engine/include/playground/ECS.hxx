#pragma once

#define FLECS_CORE
#define FLECS_SYSTEM
#define FLECS_PIPELINE
#define FLECS_META
#define FLECS_META_C
#define FLECS_PIPELINE
#define FLECS_HTTP
#define FLECS_TIMER
#define FLECS_REST
#define FLECS_STATS
#define FLECS_MONITOR
#define FLECS_CPP
#include <flecs.h>
#include <flecs/addons/rest.h>

namespace playground::ecs {
    typedef void (*SystemTickDelegate)(ecs_iter_t*);
    typedef void (*ComponentLifetimeDelegate)(ecs_iter_t*);
    void Init(int tickRate, bool debugServer);
    void Update(double deltaTime);
    void Shutdown();

    uint64_t CreateEntity(const char* name);
    void DestroyEntity(uint64_t entityId);
    void SetParent(uint64_t childId, uint64_t parentId);
    uint64_t GetParent(uint64_t childId);
    uint64_t GetEntityByName(const char* name);

    uint64_t RegisterComponent(const char* name, size_t size, size_t alignment);
    void AddComponent(uint64_t entityId, uint64_t componentId);
    void SetComponent(uint64_t entityId, uint64_t componentId, const void* data);
    const void* GetComponent(uint64_t entityId, uint64_t componentId);
    bool HasComponent(uint64_t entityId, uint64_t componentId);
    void DestroyComponent(uint64_t entityId, uint64_t componentId);

    uint64_t CreateSystem(const char* name, uint64_t* filter, size_t filterCount, bool isParallel, SystemTickDelegate delegate);
    void* GetComponentBuffer(ecs_iter_t* iter, uint32_t index, size_t componentSize, size_t* numItems);
    uint64_t GetIteratorSize(ecs_iter_t* iter);
    uint64_t GetIteratorOffset(ecs_iter_t* iter);
    const uint64_t* GetEntitiesFromIterator(ecs_iter_t* iter, size_t* size);
    void CreateHook(uint64_t component, ComponentLifetimeDelegate onAdd, ComponentLifetimeDelegate onRemove);
    void DeleteAllEntitiesByTag(uint64_t tag);

    uint64_t CreateTag(const char* name);
    void AddTag(uint64_t entityId, uint64_t tagId);
}
