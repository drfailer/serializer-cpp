# C++ serializer generation

The purpose of this project is to use templates and concepts (C++20) to
automate the generation of serialization and deserialization methods for C++
classes.

The best way to learn how to use the lib is to check the [wiki](https://github.com/drfailer/serializer-cpp/wiki).

## Example

To generate the code, the target class must use the `SERIALIZABLE` macro. This is a
variadic macro function that takes the types of the members that will be
serialized. Then we use the `SERIALIZER` macro function that takes references to the
members to serialize as argument. Then, we can use, `serialize` and `deserialize`
methods on the objects.

```cpp
#include <serializer/serializable.hpp>

class Simple {
    SERIALIZABLE(int, std::string, int); // making the class serializable

  public:
    explicit Simple(int x, int y, std::string str):
        SERIALIZER(x_, str_, y_), // initializing the serializer
        x_(x), y_(y), str_(std::move(str)) {}
    /* ... */

  private:
    int x_;
    int y_;
    std::string str_;
};

void main(int argc, char** argv) {
    Simple s1(1, 2, "three");
    Simple s2;
    std::string result;

    result = s1.serialize();
    s2.deserialize(result);
    /* s1 == s2 */

    // works with file too
    s1.serializeFile("./simple.txt");
}
```

## Warnings

### Genericity

This method relies a lot on concepts, which means that a lot of conversion
functions have been written. However, even if this simple library tries to be
very generic, some types may not be handled properly. The library allow to
create custom functions manually if required.

### Pointers

The pointers are supported by the library, but they have to be initialized
properly before the deserialization (either `nullptr` or a valid memory address).

### Polymorphism

Polymorphism is very problematic in this case so polymorphic types will have to
be handled manually. There is a convenient macro for this.

### Containers

Containers are detected with iterators so the majority of the containers of the
standard library are supported. External containers that are iterable will be
supported too (`QList` from Qt for instance). However, the elements inside the
containers must be serializable.

**Static arrays are not supported.**
