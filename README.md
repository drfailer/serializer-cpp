# Serializer-cpp

`serializer-cpp` is a C++20 binary serialization library. The library povides
very generic functions that can handle most of the types of the standard library
and more. It also provides some convenient macros that generate some code
automatically.

The best way to learn how to use the lib is to check the [wiki](https://github.com/drfailer/serializer-cpp/wiki).

## Features

- Easy serialization by writting minimal code.
- Automatic serialization for most of the types of the standard library and
  more.
- Support for polymorphic types.
- Identifiers for deserializing and managing objects with a type not known at
  compile time.
- Lite binary output format.

## Example

```cpp
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>

class MyClass {
  public:
    /* ... */
    SERIALIZE(x_, y_, str_)

  private:
    int x_;
    int y_;
    std::string str_;
};

void main(int argc, char** argv) {
    MyClass s1(1, 2, "three");
    MyClass s2;
    serializer::Bytes result;

    s1.serialize(result);
    s2.deserialize(result);
    /* s1 == s2 */
}
```

## Alternatives

- [zpp_bits](https://github.com/eyalz800/zpp_bits)
- [cereal](https://uscilab.github.io/cereal/)
- [cista](https://github.com/felixguendling/cista)
