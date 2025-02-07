namespace Playground;

public partial class Component
{
    public GameObject? GameObject { get; set; }

    public virtual void OnAwake() {}
    
    public virtual void OnStart() {}

    public virtual void OnUpdate()
    {
       
    }
    
    public virtual void OnDestroy() {}
}
