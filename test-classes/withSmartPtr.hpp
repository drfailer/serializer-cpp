#ifndef WITH_SMART_PTR_HPP
#define WITH_SMART_PTR_HPP
#include "serializer/serializable.hpp"
#include "serializer/serializer.hpp"
#include <memory>
#include <string>

class WithSmartPtr {
    SERIALIZABLE(std::shared_ptr<int>, std::unique_ptr<double>, std::string);

  public:
    explicit WithSmartPtr(int intVal = 0, double doubleVal = 0,
                          std::string strVal = "")
        : SERIALIZER(intPtr_, doublePtr_, otherType_),
          intPtr_(std::make_shared<int>(intVal)),
          doublePtr_(std::make_unique<double>(doubleVal)),
          otherType_(std::move(strVal)) {}
    ~WithSmartPtr() = default;

    [[nodiscard]] std::string otherType() const { return otherType_; }
    [[nodiscard]] double doublePtr() const { return *doublePtr_; }
    [[nodiscard]] int intPtr() const { return *intPtr_; }

  private:
    std::shared_ptr<int> intPtr_;
    std::unique_ptr<double> doublePtr_;
    std::string otherType_;
};

#endif
