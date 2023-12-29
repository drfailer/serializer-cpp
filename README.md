# C++ serializer generation

The purpose of this project is to use templates to automate generation of
serialization and deserialisation methods for C++ classes.

## How to use

To generate code, the target class must use the `SERIALIZABLE` macro. This is a
variadic macro function that takes the types of the attributes that will be
serialized. Then we use the `SERIALIZER` macro function that takes the
attributes to serialize as argument. This will transfer references to the
serialized attributes. After this two things have been done, we can use,
`serialize` and `deserialize` methods on the objects.

The `serialize` method returns a `std::string` and the `deserialize` method
takes a `std::string` as argument and update the attributes of the object.

### Example for a simple class

```cpp
class Simple {
    SERIALIZABLE(int, int); // this class can be serialized
  public:
    // we have to call the SERIALIZER in the constructor parameters list
    Simple(int _x = 0, int _y = 0) : SERIALIZER(x, y), x(_x), y(_y) {}
    Simple(const Simple &other) : Simple(other.x, other.y) {}
    ~Simple() = default;

  private:
    int x;
    int y;
};

void main(int argc, char** argv) {
    Simple s1(1, 2);
    Simple s2;
    std::string result;

    result = s.serialize();
    s2.deserialize(result);

    // s1 == s2
}
```

## Limits

### Genericity

This method relies a lot on SFINAE, which means that a lot of conversion
functions have been written. However, even if this simple library tries to be
very generic, some types may not be handled properly. The library allow to
create custom functions manually if required.

### Polymorphism

Polymorphism is very problematic in this case so polymophic types will have to
be handled manually.

### Containers

Here containers are detected with iterators. If a collection of elements has to
be serialized, it must be iterable and it has to store only non-pointer types or
pointers on fundamental types. This is due to the fact that polymophic types
can not be handled.

For now, **only sequential access containers like `std::vector` are handled**.
The library should be completed to be able to handle maps, ...
