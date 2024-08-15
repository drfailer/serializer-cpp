#ifndef WITHFUNCTIONS_HPP
#define WITHFUNCTIONS_HPP
#include <serializer/serialize.hpp>

class WithFunctions {
  public:
    explicit WithFunctions(int i = 0) : i_(i) {}

    SERIALIZE(serializer::tools::mtf::type_list<int, serializer::function_t>(),
              i_, SER_DFUN({
                  if constexpr (!std::is_const_v<
                                    std::remove_pointer_t<decltype(this)>>) {
                      /* i_ *= i_; */
                  }
              }));

    [[nodiscard]] int i() const { return i_; }
    void i(int i) { this->i_ = i; }

  private:
    int i_;
};

#endif
