#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>
#include <playground/Engine.hxx>

#include "executable_path.hxx"

#ifdef _WIN32
#include <Windows.h>
typedef wchar_t char_t;
typedef std::wstring string_t;

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#define string_compare wcscmp

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

typedef char_t char;
typedef string_t std::string

#endif

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#include <thread>

typedef void(__cdecl* Startup)(LookupTableDelegate, ScriptStartupCallback);
typedef void(__cdecl* CoreLayerStartUp)(PlaygroundConfig&);
typedef void(__cdecl* ScriptingLayerStartUp)(Startup);
typedef void(*register_function_fn)(const char* name, size_t length, void* ptr);
typedef int(*startup_fn)(const char* path, size_t len);

CoreLayerStartUp coreStartup = nullptr;
uint16_t windowWidth = 1280;
uint16_t windowHeight = 720;
void* windowPtr = nullptr;

extern "C" uint8_t PlaygroundCoreMain(const PlaygroundConfig&);
extern "C" void PlaygroundMain(Startup start);
extern "C" void AssemblyMain();

// Forward declarations
hostfxr_handle fx_handle = nullptr;
hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
hostfxr_get_runtime_delegate_fn get_delegate_fptr;
hostfxr_run_app_fn run_app_fptr;
hostfxr_close_fn close_fptr;
load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
string_t get_full_path(int argc, char_t** argv);
int run_engine_dot_net_assembly(const string_t& root_path);
bool load_hostfxr(const char_t *assembly_path);
void *load_library(const char_t *);
void *get_export(void *, const char *);
bool LostHostFxr();
load_assembly_and_get_function_pointer_fn LoadDotNetAssembly(const char_t *config_path);
void load_startup_method(const char_t *config_path);
void load_function_lookup_method(const char_t *config_path);
load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *config_path);
register_function_fn registerFunctionPtr = nullptr;
startup_fn startupPtr = nullptr;

void StartUpEngine(LookupTableDelegate lookup, ScriptStartupCallback startup) {
    auto workDir = std::filesystem::current_path().string();
    auto config = PlaygroundConfig{
        lookup,
#if EDITOR
        nullptr,
#endif
        startup,
        windowWidth,
        windowHeight,
        true,
        "Test",
        workDir.c_str(),
        nullptr
    };

    PlaygroundCoreMain(config);
}

void StartupDelegate(
    LookupTableDelegate lookupPtr,
    ScriptStartupCallback startupPtr
) {
    StartUpEngine(lookupPtr, startupPtr);
}

#ifdef _WIN32
int wmain(int argc, char_t** argv)
#else
int main(int argc, char_t** argv)
#endif
{
    auto coreClr = run_engine_dot_net_assembly(get_full_path(argc, argv));
    if (coreClr != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    StartUpEngine([](auto name, auto ptr)
    {
        registerFunctionPtr(name, strlen(name), ptr);
    }, [] ()
    {
        auto path = executable_path().parent_path().string();
        std::cout << "Scripting Assembly startup: "<< startupPtr(path.c_str(), path.size()) << std::endl;
    });

	return 0;
}

string_t get_full_path(int argc, char_t** argv)
{
    char_t host_path[MAX_PATH];
#if _WIN32
    auto size = ::GetFullPathNameW(argv[0], sizeof(host_path) / sizeof(char_t), host_path, nullptr);
    assert(size != 0);
#else
    auto resolved = realpath(argv[0], host_path);
    assert(resolved != nullptr);
#endif

    string_t root_path = host_path;
    auto pos = root_path.find_last_of(DIR_SEPARATOR);
    assert(pos != string_t::npos);
    root_path = root_path.substr(0, pos + 1);

    return root_path;
}

int run_engine_dot_net_assembly(const string_t& root_path)
{
    const string_t app_path = root_path + STR("PlaygroundAssembly.dll");
    const string_t configPath = root_path + STR("PlaygroundAssembly.runtimeconfig.json");

    if (!load_hostfxr(app_path.c_str()))
    {
        assert(false && "Failure: load_hostfxr()");
        return EXIT_FAILURE;
    }

    // Load .NET Core
    load_assembly_and_get_function_pointer = get_dotnet_load_assembly(configPath.c_str());

    load_startup_method(configPath.c_str());
    load_function_lookup_method(configPath.c_str());

    return 0;
}

bool load_hostfxr(const char_t *assembly_path)
{
    get_hostfxr_parameters params { sizeof(get_hostfxr_parameters), assembly_path, nullptr };
    // Pre-allocate a large buffer for the path to hostfxr
    char_t buffer[MAX_PATH];
    size_t buffer_size = sizeof(buffer) / sizeof(char_t);
    int rc = get_hostfxr_path(buffer, &buffer_size, &params);
    if (rc != 0)
    {
        return false;
    }

    // Load hostfxr and get desired exports
    // NOTE: The .NET Runtime does not support unloading any of its native libraries. Running
    // dlclose/FreeLibrary on any .NET libraries produces undefined behavior.
    void *lib = load_library(buffer);
    init_for_config_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
    get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
    run_app_fptr = (hostfxr_run_app_fn)get_export(lib, "hostfxr_run_app");
    close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

    return (init_for_config_fptr && get_delegate_fptr && close_fptr);
}

load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *config_path)
{
    std::wcout << L"runtimeconfig path = " << config_path << std::endl;
    std::wcout << L"exists = " << std::filesystem::exists(config_path) << std::endl;

    // Load .NET Core
    void *load_assembly_and_get_function_pointer = nullptr;
    hostfxr_handle cxt = nullptr;
    int rc = init_for_config_fptr(config_path, nullptr, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        close_fptr(cxt);
        return nullptr;
    }

    // Get the load assembly function pointer
    rc = get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
    {
        std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
    }

    fx_handle = cxt;
    //close_fptr(cxt);
    return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
}

void load_startup_method(const char_t *config_path)
{
    startupPtr = nullptr;

    int rc = load_assembly_and_get_function_pointer(
        config_path,
        STR("PlaygroundAssembly.PlaygroundAssembly, PlaygroundAssembly"),
        STR("Startup"),
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        (void**)&startupPtr
    );

    std::cout << std::hex << rc << std::endl;

    assert(rc == 0 && startupPtr);
}

void load_function_lookup_method(const char_t *config_path)
{
    registerFunctionPtr = nullptr;

    int rc = load_assembly_and_get_function_pointer(
        config_path,
        STR("PlaygroundAssembly.PlaygroundAssembly, PlaygroundAssembly"),
        STR("RegisterFunction"),
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        (void**)&registerFunctionPtr
    );

    std::cout << std::hex << rc << std::endl;

    assert(rc == 0 && registerFunctionPtr);
}

#ifdef _WIN32
void *load_library(const char_t *path)
{
    HMODULE h = ::LoadLibraryW(path);
    assert(h != nullptr);
    return (void*)h;
}
void *get_export(void *h, const char *name)
{
    void *f = ::GetProcAddress((HMODULE)h, name);
    assert(f != nullptr);
    return f;
}
#else
void *load_library(const char_t *path)
{
    void *h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    assert(h != nullptr);
    return h;
}
void *get_export(void *h, const char *name)
{
    void *f = dlsym(h, name);
    assert(f != nullptr);
    return f;
}
#endif
