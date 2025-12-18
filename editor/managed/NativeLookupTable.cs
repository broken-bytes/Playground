using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace PlaygroundEditor;

public static class NativeLookupTable {
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void AddEntryDelegate(IntPtr key, IntPtr value);
    
    private static Dictionary<string, IntPtr> lookupTable = new();
    
    public static void AddEntry(IntPtr key, IntPtr value) {
        string keyStr = Marshal.PtrToStringAnsi(key)!;
        lookupTable.Add(keyStr, value);
    }

    public static IntPtr Lookup(string key) {
        return lookupTable[key];
    }
}
