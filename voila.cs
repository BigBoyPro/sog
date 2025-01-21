Apologies for the earlier oversight. To replicate the full functionality of ExpandoObject, we need to implement the following interfaces:

IDynamicMetaObjectProvider

IDictionary<string, object>

ICollection<KeyValuePair<string, object>>

IEnumerable<KeyValuePair<string, object>>

INotifyPropertyChanged


Below is a comprehensive implementation of a CustomExpando class that mirrors ExpandoObject's behavior:

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Dynamic;
using System.Linq;
using System.Linq.Expressions;

public sealed class CustomExpando : DynamicObject, IDictionary<string, object>, INotifyPropertyChanged
{
    private readonly Dictionary<string, object> _properties = new();

    public event PropertyChangedEventHandler PropertyChanged;

    // DynamicObject overrides
    public override bool TryGetMember(GetMemberBinder binder, out object result)
    {
        return _properties.TryGetValue(binder.Name, out result);
    }

    public override bool TrySetMember(SetMemberBinder binder, object value)
    {
        this[binder.Name] = value;
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

    // IDictionary<string, object> implementation
    public object this[string key]
    {
        get => _properties.TryGetValue(key, out var value) ? value : throw new KeyNotFoundException($"Key '{key}' not found.");
        set
        {
            if (_properties.ContainsKey(key))
            {
                _properties[key] = value;
                OnPropertyChanged(key);
            }
            else
            {
                _properties.Add(key, value);
                OnPropertyChanged(key);
            }
        }
    }

    public ICollection<string> Keys => _properties.Keys;

    public ICollection<object> Values => _properties.Values;

    public int Count => _properties.Count;

    public bool IsReadOnly => false;

    public void Add(string key, object value)
    {
        _properties.Add(key, value);
        OnPropertyChanged(key);
    }

    public bool ContainsKey(string key) => _properties.ContainsKey(key);

    public bool Remove(string key)
    {
        if (_properties.Remove(key))
        {
            OnPropertyChanged(key);
            return true;
        }
        return false;
    }

    public bool TryGetValue(string key, out object value) => _properties.TryGetValue(key, out value);

    public void Add(KeyValuePair<string, object> item) => Add(item.Key, item.Value);

    public void Clear()
    {
        var keys = _properties.Keys.ToList();
        _properties.Clear();
        foreach (var key in keys)
        {
            OnPropertyChanged(key);
        }
    }

    public bool Contains(KeyValuePair<string, object> item) => _properties.ContainsKey(item.Key) && _properties[item.Key] == item.Value;

    public void CopyTo(KeyValuePair<string, object>[] array, int arrayIndex)
    {
        foreach (var kvp in _properties)
        {
            array[arrayIndex++] = kvp;
        }
    }

    public bool Remove(KeyValuePair<string, object> item) => Remove(item.Key);

    public IEnumerator<KeyValuePair<string, object>> GetEnumerator() => _properties.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => _properties.GetEnumerator();

    // INotifyPropertyChanged implementation
    private void OnPropertyChanged(string propertyName)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

Key Features:

1. Dynamic Member Handling:

Overrides TryGetMember, TrySetMember, and TryInvokeMember to manage dynamic members.



2. Dictionary Functionality:

Implements IDictionary<string, object> to allow key-value pair management.



3. Property Change Notification:

Implements INotifyPropertyChanged to notify subscribers of property changes, facilitating data binding in WPF.



4. Event Invocation:

Supports dynamic invocation of delegates stored as properties.




This implementation ensures that CustomExpando behaves similarly to ExpandoObject, including dynamic member addition/removal, dictionary operations, and property change notifications.

