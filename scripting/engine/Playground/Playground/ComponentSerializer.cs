using System.Reflection;

namespace Playground;

internal class ComponentSerializer
{
    internal static Dictionary<string, object> Serialize(object obj)
    {
        var serializedData = new Dictionary<string, object>();

        // Get all fields marked with the [Serialize] attribute
        var fields = obj.GetType().GetFields(BindingFlags.NonPublic | BindingFlags.Instance)
            .Where(f => f.GetCustomAttribute<SerializeAttribute>() != null)
            .ToList();
        
        fields.AddRange(obj.GetType().GetFields(BindingFlags.Public | BindingFlags.Instance));

        // Serialize each field and add it to the dictionary
        foreach (var field in fields)
        {
            var value = field.GetValue(obj);
            serializedData[field.Name] = value;
        }

        return serializedData;
    }

    internal static void Deserialize(object obj, Dictionary<string, object> serializedData)
    {
        var fields = obj.GetType().GetFields(BindingFlags.NonPublic | BindingFlags.Instance)
            .Where(f => f.GetCustomAttribute<SerializeAttribute>() != null)
            .ToList();
        
        fields.AddRange(obj.GetType().GetFields(BindingFlags.Public | BindingFlags.Instance));

        // For each field marked with [Serialize], set its value from the dictionary
        foreach (var field in fields)
        {
            if (serializedData.ContainsKey(field.Name))
            {
                var value = serializedData[field.Name];
                field.SetValue(obj, value);
            }
        }
    }
}
