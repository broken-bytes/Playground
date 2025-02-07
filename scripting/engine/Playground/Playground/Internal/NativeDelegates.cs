using System.Runtime.InteropServices;

namespace Playground.Internal;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void LookupTableDelegate(string name, IntPtr address);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void EmptyDelegate();

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void UpdateDelegate(double deltaTime);
