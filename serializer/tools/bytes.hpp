#ifndef SERIALIZER_BYTES_H
#define SERIALIZER_BYTES_H
#include <algorithm>
#include <cstddef>
#include <cstring>

/******************************************************************************/
/*                                   bytes                                    */
/******************************************************************************/

/// @brief namespace serializer tools
namespace serializer::tools {

/// @brief Custom vector for serialization (std::vector interface is anoying to
///        used for the serialization).
/// @tparam T Byte type (std::byte, uint8_t, char, ...).
template <typename T>
  requires (sizeof(T) == sizeof(char))
class Bytes {
  public:
    /* type alias *************************************************************/

    using byte_type = T;

    /* constructors & destructor **********************************************/

    /// @brief Default constructor.
    constexpr Bytes() = default;

    /// @brief Constructor with capacity.
    constexpr Bytes(size_t capacity, size_t size = 0)
        : mem_(new T[capacity]), capacity_(capacity), size_(size) {}

    /// @brief constructor with a pointer and a size
    constexpr Bytes(T *ptr, size_t capacity, size_t size = 0)
        : mem_(ptr),capacity_(capacity), size_(size) {}

    /// @brief Copy constructor.
    constexpr Bytes(Bytes<T> const &other)
        : mem_(new T[other.capacity_]), capacity_(other.capacity_),
          size_(other.size_) {
        std::memcpy(mem_, other.mem_, size_);
    }

    /// @brief Move constructor.
    constexpr Bytes(Bytes<T> &&other) noexcept
        : mem_(other.mem_), capacity_(other.capacity_), size_(other.size_) {
        other.mem_ = nullptr;
    }

    /// @brief Destructor.
    constexpr ~Bytes() { delete[] mem_; }

    /* accessors **************************************************************/

    /// @brief Returns a pointer to the bytes buffer.
    constexpr T *data() { return mem_; }

    /// @brief Returns a const pointer to the bytes buffer.
    constexpr T const *data() const { return mem_; }

    /// @brief Returns the current capacity of the buffer.
    constexpr size_t capacity() const { return capacity_; }

    /// @brief Returns the number of bytes stored in the buffer.
    constexpr size_t size() const { return size_; }

    /// @brief Allow to manually resize.
    constexpr void resize(size_t size) { size_ = size; }

    /// @breif Clear the buffer (set the size to 0 but do not reallocate).
    constexpr void clear() { size_ = 0; }

    /* append *****************************************************************/

    /// @brief Appends some bytes at pos. The function is called "append" and
    ///        not "insert" because the size is always equal to `pos + count` at
    ///        the end.
    /// @param pos     Position where the bytes are appended.
    /// @param bytes   Buffer of bytes to append.
    /// @param nbBytes Number of bytes to append.
    constexpr void append(size_t pos, T const *bytes, size_t nbBytes) {
        upsize(pos + nbBytes);
        size_ = pos + nbBytes;
        std::memcpy(mem_ + pos, bytes, nbBytes);
    }

    /* change size and capacity ***********************************************/

    /// @brief Increase the capacity of the memory buffer if size bytes cannot
    ///        be stored.
    /// @param size New size.
    constexpr void upsize(size_t size) {
        if (size > capacity_) [[unlikely]] {
            if (size > (capacity_ * 2)) [[unlikely]] {
                alloc(size);
            } else {
                alloc(capacity_ * 2);
            }
        }
    }

    /// @brief Reallocate memory and change the capacity.
    /// @param newCapacity New capacity of the the buffer.
    constexpr void alloc(size_t newCapacity) {
        capacity_ = newCapacity;
        T *tmp = mem_;
        mem_ = new T[capacity_];
        std::memcpy(mem_, tmp, size_);
        delete[] tmp;
    }

    /* operators **************************************************************/

    /// @brief Give read/write access to the byte `idx`.
    constexpr T &operator[](size_t idx) { return mem_[idx]; }

    /// @brief Give read access to the byte `idx`
    constexpr T const &operator[](size_t idx) const { return mem_[idx]; }

    /// @brief Copy assignment
    constexpr Bytes<T> &operator=(Bytes<T> const &other) {
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

    /// @brief Move assignment
    constexpr Bytes<T> &operator=(Bytes<T> &&other) noexcept {
        capacity_ = other.capacity_;
        size_ = other.size_;
        std::swap(this->mem_, other.mem_);
        return *this;
    }

    /* convertion *************************************************************/

    /// @brief Create a std::vector from the memory buffer.
    std::vector<T> vector() const { return std::vector<T>(mem_, mem_ + size_); }

  private:
    T *mem_ = nullptr;    ///< bytes buffer
    size_t capacity_ = 0; ///< capacity of the buffer
    size_t size_ = 0;     ///< number of bytes stored
};

} // end namespace serializer::tools

#endif
