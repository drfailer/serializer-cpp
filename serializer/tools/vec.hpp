#ifndef SERIALIZER_CPP_VEC_H
#define SERIALIZER_CPP_VEC_H
#include <algorithm>
#include <cstddef>
#include <cstring>

namespace serializer::tools {

template <typename T>
class vec {
  public:
    constexpr vec() = default;
    constexpr vec(size_t capacity)
        : mem_(new T[capacity]), capacity_(capacity) {}
    constexpr vec(vec<T> const &other)
        : mem_(new T[other.capacity_]), capacity_(other.capacity_),
          size_(other.size_) {
        std::memcpy(mem_, other.mem_, size_);
    }
    constexpr vec(vec<T> &&other) noexcept
        : mem_(other.mem_), capacity_(other.capacity_), size_(other.size_) {
        other.mem_ = nullptr;
    }
    constexpr ~vec() { delete[] mem_; }

    constexpr T *data() { return mem_; }
    constexpr T const *data() const { return mem_; }
    constexpr size_t size() const { return size_; }
    constexpr size_t capacity() const { return capacity_; }

    constexpr void size(size_t new_size) { size_ = new_size; }

    constexpr void insert(size_t pos, T const *src, size_t count) {
        upsize(pos + count);
        size_ = std::max(pos + count, size_);
        std::memcpy(mem_ + pos, src, count);
    }

    constexpr void append(size_t pos, T const *src, size_t count) {
        upsize(pos + count);
        size_ = pos + count;
        std::memcpy(mem_ + pos, src, count);
    }

    constexpr void upsize(size_t size) {
        if (size > capacity_) [[unlikely]] {
            if (size > (capacity_ * 2)) [[unlikely]] {
                alloc(size);
            } else {
                alloc(capacity_ * 2);
            }
        }
    }

    constexpr void alloc(size_t size) {
        capacity_ = size;
        T *tmp = mem_;
        mem_ = new T[capacity_];
        std::memcpy(mem_, tmp, size_);
        delete[] tmp;
    }

    constexpr void clear() { size_ = 0; }

    constexpr T &operator[](size_t idx) { return mem_[idx]; }

    constexpr T const &operator[](size_t idx) const { return mem_[idx]; }

    constexpr vec<T> &operator=(vec<T> const &other) {
        if (&other == this) {
            return *this;
        }
        capacity_ = other.capacity_;
        delete[] mem_;
        mem_ = new T[capacity_];
        size_ = other.size_;
        std::memcpy(mem_, other.mem_, size_);
        return *this;
    }

    constexpr vec<T> &operator=(vec<T> &&other) noexcept {
        capacity_ = other.capacity_;
        size_ = other.size_;
        std::swap(this->mem, other.mem_);
        return *this;
    }

    std::vector<T> vector() const { return std::vector<T>(mem_, mem_ + size_); }

  private:
    T *mem_ = nullptr;
    size_t capacity_ = 0;
    size_t size_ = 0;
};

} // end namespace serializer::tools

#endif
