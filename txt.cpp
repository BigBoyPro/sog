Recreating ExpandoObject in C# as a custom class that implements DynamicObject is a complex but feasible task. Below is a detailed implementation that ensures the behavior is as close as possible to ExpandoObject. It includes handling dynamic members, supporting PropertyDescriptor, and managing events to make it compatible with DevExpress controls.

Here’s the code:

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Dynamic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

public class CustomExpando : DynamicObject, INotifyPropertyChanged, ICustomTypeDescriptor
{
    private readonly Dictionary<string, object> _properties = new();
    private readonly PropertyDescriptorCollection _propertyDescriptors;

    public event PropertyChangedEventHandler PropertyChanged;

    public CustomExpando()
    {
        _propertyDescriptors = new PropertyDescriptorCollection(null);
    }

    public override bool TryGetMember(GetMemberBinder binder, out object result)
    {
        if (_properties.TryGetValue(binder.Name, out result))
        {
            return true;
        }

        result = null;
        return false;
    }

    public override bool TrySetMember(SetMemberBinder binder, object value)
    {
        if (!_properties.ContainsKey(binder.Name))
        {
            AddProperty(binder.Name, value);
        }
        else
        {
            _properties[binder.Name] = value;
            OnPropertyChanged(binder.Name);
        }

        return true;
    }

    public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
    {
        if (_properties.TryGetValue(binder.Name, out var member) && member is Delegate del)
        {
            result = del.DynamicInvoke(args);
            return true;
        }

        result = null;
        return false;
    }

    private void AddProperty(string name, object value)
    {
        _properties[name] = value;

        var descriptor = new CustomPropertyDescriptor(name, value?.GetType() ?? typeof(object));
        _propertyDescriptors.Add(descriptor);

        OnPropertyChanged(name);
    }

    private void OnPropertyChanged(string name)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }

    #region ICustomTypeDescriptor Implementation

    public AttributeCollection GetAttributes() => AttributeCollection.Empty;

    public string GetClassName() => nameof(CustomExpando);

    public string GetComponentName() => nameof(CustomExpando);

    public TypeConverter GetConverter() => new TypeConverter();

    public EventDescriptor GetDefaultEvent() => null;

    public PropertyDescriptor GetDefaultProperty() => null;

    public object GetEditor(Type editorBaseType) => null;

    public EventDescriptorCollection GetEvents() => EventDescriptorCollection.Empty;

    public EventDescriptorCollection GetEvents(Attribute[] attributes) => EventDescriptorCollection.Empty;

    public PropertyDescriptorCollection GetProperties() => _propertyDescriptors;

    public PropertyDescriptorCollection GetProperties(Attribute[] attributes) => _propertyDescriptors;

    public object GetPropertyOwner(PropertyDescriptor pd) => this;

    #endregion
}

public class CustomPropertyDescriptor : PropertyDescriptor
{
    private readonly Type _type;

    public CustomPropertyDescriptor(string name, Type type)
        : base(name, null)
    {
        _type = type;
    }

    public override bool CanResetValue(object component) => false;

    public override Type ComponentType => typeof(CustomExpando);

    public override object GetValue(object component)
    {
        if (component is CustomExpando expando && expando.TryGetProperty(Name, out var value))
        {
            return value;
        }

        return null;
    }

    public override bool IsReadOnly => false;

    public override Type PropertyType => _type;

    public override void ResetValue(object component) { }

    public override void SetValue(object component, object value)
    {
        if (component is CustomExpando expando)
        {
            expando.SetProperty(Name, value);
        }
    }

    public override bool ShouldSerializeValue(object component) => true;
}

public static class CustomExpandoExtensions
{
    public static bool TryGetProperty(this CustomExpando expando, string propertyName, out object value)
    {
        if (expando.GetDynamicMemberNames().Contains(propertyName))
        {
            value = ((IDictionary<string, object>)expando)[propertyName];
            return true;
        }

        value = null;
        return false;
    }

    public static void SetProperty(this CustomExpando expando, string propertyName, object value)
    {
        ((IDictionary<string, object>)expando)[propertyName] = value;
    }

    public static IDictionary<string, object> ToDictionary(this CustomExpando expando)
    {
        return expando.GetDynamicMemberNames().ToDictionary(name => name, name => ((IDictionary<string, object>)expando)[name]);
    }
}

Explanation:

1. Dynamic Behavior:

Implemented using DynamicObject methods like TryGetMember, TrySetMember, and TryInvokeMember.



2. PropertyDescriptor Support:

Used ICustomTypeDescriptor and PropertyDescriptor to ensure properties are dynamically described and interact well with tools like DevExpress.



3. INotifyPropertyChanged:

Implemented to allow binding and notify changes when properties are modified.



4. CustomPropertyDescriptor:

Provides metadata about each property dynamically added to the class.



5. Extensions:

Added helper methods to convert properties into a dictionary or perform direct operations.




This implementation should work seamlessly with WPF, data binding, and DevExpress controls. Let me know if you need further refinements!

