# C++ toString generation

The purpose of this is to automatically generate a `toString` function for any
C++ class. Here we are using `toString` as it's a very simple example but in the
end this method should be used for generating serialization functions.

This problem can't be easily solved by only using templates. In this draft, we
use macros and templates together to generate the required code.

## Example

In the following example, we want to generate the `toString` function for the
`Test` class.

If we want to be able to stringify `Test` in another class, we need to specify
that it implements the `Stringifiable` interface.

The second thing we have to do is generating the `toString` function using the
`stringifier` macro. This macro makes two things, first it creates a private
attributes of type `Stringifier`, then it generates the public `toString`
function. This variadic macro takes the types of the elements to stringify as
parameters (`Stringifier` is a template class).

The last thing we want to do is calling the constructor of the hidden attribute.
For this we use the `constructStringifier` macro. This one takes the attributes
that will be in the constructed string. Notice that the constructor takes
references to the attributes so we have to make sure that we don't take
references to the constructor's parameters. Furthermore, the attributes'
identifiers given to the constructor will be the same in the constructed string.
This means that if you give `this->x`, the constructed string will contain
`"this->x"` instead of `"x"`

```cpp
class Test : public Stringifiable {
  public:
    Test(int _x = 0, int _y = 0) : x(_x), y(_y), constructStringifier(x, y) {}
    Test(const Test& other): Test(other.x, other.y) {}
    ~Test() = default;

    // generate the toString function
    stringifier(int, int)

  private:
    int x;
    int y;
};
```
