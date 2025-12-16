using System.Runtime.CompilerServices;

namespace PlaygroundAssembly.Core;

internal static class LoggerApi
{
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, void> LogInfoPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, void> LogWarningPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, void> LogErrorPtr;

    internal static unsafe void Setup()
    {
        LogInfoPtr =
            (delegate* unmanaged[Cdecl]<byte*, void>)
            NativeLookupTable.GetFunctionPointer("Logger_Info");
        
        LogWarningPtr =
            (delegate* unmanaged[Cdecl]<byte*, void>)
            NativeLookupTable.GetFunctionPointer("Logger_Warn");
        
        LogErrorPtr =
            (delegate* unmanaged[Cdecl]<byte*, void>)
            NativeLookupTable.GetFunctionPointer("Logger_Error");
    }
    
    internal static unsafe void LogInfo(string message)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(message);

        var size = Unsafe.SizeOf<Type>();
        
        fixed (byte* ptr = utf8)
        {
            LogInfoPtr(ptr);
        }
    }
    
    internal static unsafe void LogWarning(string message)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(message);

        var size = Unsafe.SizeOf<Type>();
        
        fixed (byte* ptr = utf8)
        {
            LogWarningPtr(ptr);
        }
    }
    
    internal static unsafe void LogError(string message)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(message);

        var size = Unsafe.SizeOf<Type>();
        
        fixed (byte* ptr = utf8)
        {
            LogErrorPtr(ptr);
        }
    }
}
