﻿namespace Playground;

using System;

[AttributeUsage(AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
public class SerializeAttribute : Attribute
{
}
