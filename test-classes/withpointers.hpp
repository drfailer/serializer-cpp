#ifndef WITH_POINTERS_HPP
#define WITH_POINTERS_HPP
#include "serializer/serializer.hpp"
#include "test-classes/simple.hpp"

class WithPointers {
    SERIALIZABLE(int *, double *, Simple *);
  public:
    explicit WithPointers(Simple *classPointer)
        : SERIALIZER(nullPointer_, fundamentalPointer_, classPointer_),
          classPointer_(classPointer) {
        fundamentalPointer_ = new double;
        *fundamentalPointer_ = 1.9;
    }
    ~WithPointers() {
        delete fundamentalPointer_;
        delete classPointer_;
    }

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
    [[nodiscard]] double *fundamentalPointer() const { return fundamentalPointer_; }
    [[nodiscard]] int *nullPointer() const { return nullPointer_; }

  private:
    int *nullPointer_ = nullptr;
    double *fundamentalPointer_ = nullptr;
    Simple *classPointer_ = nullptr;
};

#endif
