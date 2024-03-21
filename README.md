# C++ serializer generation

The purpose of this project is to use templates to automate generation of
serialization and deserialisation methods for C++ classes.

The best way to learn how to use the lib is to check the [wiki](https://github.com/drfailer/serializer-cpp/wiki).

## Example

To generate code, the target class must use the `SERIALIZABLE` macro. This is a
variadic macro function that takes the types of the attributes that will be
serialized. Then we use the `SERIALIZER` macro function that takes the
attributes to serialize as argument. This will transfer references to the
serialized attributes. After this two things have been done, we can use,
`serialize` and `deserialize` methods on the objects.

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

    // works with file too
    s.serializeFile("./simple.txt");
}
```

## Warnings

### Genericity

This method relies a lot on SFINAE, which means that a lot of conversion
functions have been written. However, even if this simple library tries to be
very generic, some types may not be handled properly. The library allow to
create custom functions manually if required.

### Pointers

The pointers are supported by the library but they have to be initialized
properly before the deserialisation (either `nullptr` or a valid memory adress).

### Polymorphism

Polymorphism is very problematic in this case so polymophic types will have to
be handled manually. There is a convenient macro for this.

### Containers

Containers are detected with iterators so the majority of the containers of the
standard library. External containers that implements the iterator trait
properly will be supported too. However, the elements inside the containers must
be serializable.

**Static arrays are not supported.**
