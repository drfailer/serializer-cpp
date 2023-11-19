# C++ toString generation

The purpose of this is to automatically generate a `toString` function for any
C++ class. Here we are using `toString` as it's a very simple example but in the
end this method should be used for generating serialization functions.

## Example

```cpp
#include "stringifiable.hpp"

class Test : public Stringifiable<int, int> {
public:
    Test(int _x = 0, int _y = 0) : stringifiable(x, y), x(_x), y(_y) { }
    Test(const Test& other): Test(other.x, other.y) { }
    ~Test() = default;

private:
    int x;
    int y;
};
```
