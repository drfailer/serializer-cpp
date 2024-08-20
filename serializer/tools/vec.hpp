#ifndef SERIALIZER_CPP_VEC_H
#define SERIALIZER_CPP_VEC_H
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace serializer::tools {

template <typename T>
    requires(std::is_fundamental_v<T>)
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
        if (pos + count > capacity_) {
            if (capacity_ == 0) [[unlikely]] {
                alloc(pos + count);
            } else {
                alloc(capacity_ * 2);
            }
        }
        size_ = std::max(pos + count, size_);
        std::memcpy(mem_ + pos, src, count);
    }

    constexpr void append(size_t pos, T const *src, size_t count) {
        if (pos + count > capacity_) {
            if (capacity_ == 0) [[unlikely]] {
                alloc(pos + count);
            } else {
                alloc(capacity_ * 2);
            }
        }
        size_ = pos + count;
        std::memcpy(mem_ + pos, src, count);
    }

    constexpr void append(T const *src, size_t count) {
        if (size_ + count > capacity_) {
            if (capacity_ == 0) [[unlikely]] {
                alloc(size_ + count);
            } else {
                alloc(capacity_ * 2);
            }
        }
        std::memcpy(mem_ + size_, src, count);
        size_ += count;
    }

    constexpr void alloc(size_t size) {
        capacity_ = size;
        T *tmp = mem_;
        mem_ = new T[capacity_];
        std::memcpy(mem_, tmp, size_);
        delete[] tmp;
    }

    constexpr void resize(size_t new_size) {
        if (new_size > capacity_) {
            if (capacity_ == 0) [[unlikely]] {
                alloc(new_size);
            } else {
                alloc(capacity_ * 2);
            }
        }
        size_ = new_size;
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

  private:
    T *mem_ = nullptr;
    size_t capacity_ = 0;
    size_t size_ = 0;
};

} // end namespace serializer::tools

#endif
