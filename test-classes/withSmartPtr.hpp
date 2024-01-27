#ifndef WITHSMARTPTR_HPP
#define WITHSMARTPTR_HPP
#include <memory>
#include <string>
#include "serializer/serializable.hpp"
#include "serializer/serializer.hpp"

class WithSmartPtr {
    SERIALIZABLE(std::shared_ptr<int>, std::unique_ptr<double>, std::string);
public:
    WithSmartPtr(int intVal = 0, double doubleVal = 0, const std::string& strVal = "") :
        SERIALIZER(intPtr, doublePtr, otherType),
        intPtr(std::make_shared<int>(intVal)),
        doublePtr(std::make_unique<double>(doubleVal)), otherType(strVal) {}
    ~WithSmartPtr() = default;

    std::string getOtherType() const { return otherType; }
    double getDoublePtr() const { return *doublePtr; }
    int getIntPtr() const { return *intPtr; }

private:
    std::shared_ptr<int> intPtr;
    std::unique_ptr<double> doublePtr;
    std::string otherType;
};

#endif
