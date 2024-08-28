#ifndef WITHFUNCTIONS_HPP
#define WITHFUNCTIONS_HPP
#include <serializer/serialize.hpp>
#include <serializer/tools/macros.hpp>

class WithFunctions {
  public:
    explicit WithFunctions(int i = 0) : i_(i) {}

    SERIALIZE(SER_FUN({
        if constexpr (Phase == serializer::tools::Phases::Serialization) {
            context.serializer.serialize_(i_);
        } else {
            context.serializer.deserialize_(i_);
        }
    }));

    [[nodiscard]] int i() const { return i_; }
    void i(int i) { this->i_ = i; }

  private:
    int i_;
};

#endif
