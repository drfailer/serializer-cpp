#ifndef WITHFUNCTIONS_HPP
#define WITHFUNCTIONS_HPP
#include <serializer/serializable.hpp>

class WithFunctions {
    SERIALIZABLE(int, serializer::function_t);

  public:
    explicit WithFunctions(int i = 0)
        : SERIALIZER(
              i_,
              [this](serializer::Phases phase, std::string_view const &) {
                  if (phase == serializer::Phases::Serialization) {
                      i_ += i_;
                  } else {
                      i_ *= i_;
                  }
              }),
          i_(i) {}

    [[nodiscard]] int i() const { return i_; }
    void i(int i) { this->i_ = i; }

  private:
    int i_;
};

#endif
