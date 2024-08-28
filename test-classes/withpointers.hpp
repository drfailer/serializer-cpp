#ifndef WITH_POINTERS_HPP
#define WITH_POINTERS_HPP
#include "test-classes/simple.hpp"
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>

class WithPointers {
  public:
    explicit WithPointers(Simple *classPointer) : classPointer_(classPointer) {
        fundamentalPointer_ = new double;
        *fundamentalPointer_ = 1.9;
    }
    ~WithPointers() {
        delete fundamentalPointer_;
        delete classPointer_;
    }

    SERIALIZE(nullPointer_, fundamentalPointer_, classPointer_)

    /* accessors **************************************************************/
    void classPointer(Simple *classPointer) {
        delete this->classPointer_;
        this->classPointer_ = classPointer;
    }
    void fundamentalPointer(double *fundamentalPointer) {
        delete this->fundamentalPointer_;
        this->fundamentalPointer_ = fundamentalPointer;
    }
    [[nodiscard]] Simple *classPointer() const { return classPointer_; }
    [[nodiscard]] double *fundamentalPointer() const {
        return fundamentalPointer_;
    }
    [[nodiscard]] int *nullPointer() const { return nullPointer_; }

  private:
    int *nullPointer_ = nullptr;
    double *fundamentalPointer_ = nullptr;
    Simple *classPointer_ = nullptr;
};

#endif
