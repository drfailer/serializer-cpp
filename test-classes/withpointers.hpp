#ifndef WITHPOINTERS_HPP
#define WITHPOINTERS_HPP
#include "serializer/serializable.hpp"
#include "test-classes/simple.hpp"

class WithPointers : public Serializable<int *, double *, Simple *> {
  public:
    WithPointers(Simple *_classPointer)
        : serializable(nullPointer, fundamentalPointer, classPointer),
          classPointer(_classPointer) {
        fundamentalPointer = new double;
        *fundamentalPointer = 1.9;
    }
    ~WithPointers() {
        delete fundamentalPointer;
        delete classPointer;
    }

    /* accessors **************************************************************/
    void setClassPointer(Simple *classPointer) {
        delete this->classPointer;
        this->classPointer = classPointer;
    }
    void setFundamentalPointer(double *fundamentalPointer) {
        delete this->fundamentalPointer;
        this->fundamentalPointer = fundamentalPointer;
    }
    Simple *getClassPointer() const { return classPointer; }
    double *getFundamentalPointer() const { return fundamentalPointer; }
    int *getNullPointer() const { return nullPointer; }

  private:
    int *nullPointer = nullptr;
    double *fundamentalPointer = nullptr;
    Simple *classPointer = nullptr;
};

#endif
