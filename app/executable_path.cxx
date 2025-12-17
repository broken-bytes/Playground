#include "executable_path.hxx"

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <limits.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#endif

std::filesystem::path executable_path()
{
#if defined(_WIN32)
    std::wstring buffer(32768, L'\0');
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), buffer.size());
    buffer.resize(size);
    return std::filesystem::path(buffer);

#elif defined(__linux__)
    char buffer[PATH_MAX];
    ssize_t size = readlink("/proc/self/exe", buffer, PATH_MAX);
    return std::filesystem::path(std::string(buffer, size));

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    _NSGetExecutablePath(buffer.data(), &size);
    return std::filesystem::canonical(buffer);
#endif
}

std::filesystem::path executable_dir()
{
    return executable_path().parent_path();
}
