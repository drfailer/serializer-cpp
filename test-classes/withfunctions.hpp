#ifndef WITHFUNCTIONS_HPP
#define WITHFUNCTIONS_HPP
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>

class WithFunctions {
  public:
    explicit WithFunctions(int i = 0) : i_(i) {}

    SERIALIZE(SER_FUN({
        if constexpr (Phase == serializer::tools::Phases::Serialization) {
            context.serializer.serializeAll(i_);
        } else {
            context.serializer.deserializeAll(i_);
        }
    }));

    [[nodiscard]] int i() const { return i_; }
    void i(int i) { this->i_ = i; }

  private:
    int i_;
};

#endif
