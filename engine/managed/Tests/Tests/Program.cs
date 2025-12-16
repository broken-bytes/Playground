// See https://aka.ms/new-console-template for more information

unsafe
{
    delegate* unmanaged<int> fn = &PlaygroundAssembly.PlaygroundAssembly.Startup;

    fn();
}
