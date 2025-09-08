#ifndef SERIALIZER_BYTES_H
#define SERIALIZER_BYTES_H
#include <algorithm>
#include <bit>
#include <cassert>
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
    requires(sizeof(T) == sizeof(char))
class Bytes {
  public:
    /* type alias *************************************************************/

    using byte_type = T;

    /* constructors & destructor **********************************************/

    /// @brief Default constructor.
    constexpr Bytes() = default;

    /// @brief Constructor with capacity.
    constexpr Bytes(size_t capacity, size_t size = 0) : size_(size) {
        assert(capacity >= size);
        alloc(capacity);
    }

    /// @brief constructor with a pointer and a size
    constexpr Bytes(T *ptr, size_t capacity, size_t size = 0)
        : mem_(ptr), capacity_(capacity), size_(size) {}

    /// @brief deleted copy constructor
    constexpr Bytes(Bytes<T> const &other) = delete;

    constexpr Bytes(Bytes<T> &&other) noexcept {
        std::swap(this->mem_, other.mem_);
        std::swap(this->capacity_, other.capacity_);
        std::swap(this->size_, other.size_);
    }

    /// @brief Destructor.
    constexpr ~Bytes() { free(mem_); }

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
    constexpr void size(size_t size) { size_ = size; }

    /// @breif Clear the buffer (set the size to 0 but do not reallocate).
    constexpr void clear() { size_ = 0; }

    /* memory ownership *******************************************************/

    // @brief Drop memory buffer (transfer ownership).
    template <typename RT = T*>
    constexpr RT dropMem() {
        static_assert(sizeof(std::remove_pointer_t<RT>) == sizeof(T));
        RT mem = std::bit_cast<RT>(this->mem_);
        this->mem_ = nullptr;
        this->capacity_ = 0;
        this->size_ = 0;
        return mem;
    }

    // @brief Transfer ownership of the memory buffer, and take ownership of a
    //        pointer.
    template <typename PtrType = T>
    constexpr void swapMem(PtrType *&ptr, size_t capacity, size_t size = 0) {
        static_assert(sizeof(PtrType) == sizeof(T));
        T *mem = this->mem_;
        this->mem_ = std::bit_cast<T*>(ptr);
        ptr = std::bit_cast<PtrType*>(mem);
        this->capacity_ = capacity;
        this->size_ = size;
    }

    /* append *****************************************************************/

    /// @brief Appends some bytes at pos. The function is called "append" and
    ///        not "insert" because the size is always equal to `pos + count` at
    ///        the end.
    /// @param pos     Position where the bytes are appended.
    /// @param bytes   Buffer of bytes to append.
    /// @param nbBytes Number of bytes to append.
    constexpr void append(size_t pos, T const *bytes, size_t nbBytes) {
        growMemIfRequired(pos + nbBytes);
        size_ = pos + nbBytes;
        std::memcpy(mem_ + pos, bytes, nbBytes);
    }

    /// @brief Appends some bytes.
    /// @param bytes   Buffer of bytes to append.
    /// @param nbBytes Number of bytes to append.
    constexpr void append(T const *bytes, size_t nbBytes) {
        append(this->size_, bytes, nbBytes);
    }

    /* change size and capacity ***********************************************/

    /// @brief Increase the capacity of the memory buffer if size bytes cannot
    ///        be stored.
    /// @param size New size.
    constexpr void growMemIfRequired(size_t size) {
        if (size > capacity_) [[unlikely]] {
            if (size > (capacity_ * 2)) [[unlikely]] {
                alloc(size);
            } else {
                alloc(capacity_ * 2);
            }
        }
    }

    /// @brief Set capacity and size and realloc if required
    /// @param newSize New buffer size.
    constexpr void resize(size_t newSize) {
        size_ = newSize;
        growMemIfRequired(newSize);
    }

    /// @brief Reallocate memory and change the capacity.
    /// @param newCapacity New capacity of the the buffer.
    constexpr void alloc(size_t newCapacity) {
        capacity_ = newCapacity;
        mem_ = (T *)std::realloc(mem_, capacity_);
    }

    /* operators **************************************************************/

    /// @brief Give read/write access to the byte `idx`.
    constexpr T &operator[](size_t idx) { return mem_[idx]; }

    /// @brief Give read access to the byte `idx`
    constexpr T const &operator[](size_t idx) const { return mem_[idx]; }

    /// @brief Deleted copy assignment operator
    Bytes<T> &operator=(Bytes<T> const &other) = delete;

    /// @brief Move assignment
    constexpr Bytes<T> &operator=(Bytes<T> &&other) noexcept {
        capacity_ = other.capacity_;
        size_ = other.size_;
        std::swap(this->mem_, other.mem_);
        return *this;
    }

    /// @brief Clone method (this should be used instead of copy
    ///        operator/contructor since it's more explicit).
    constexpr Bytes<T> clone() const {
        Bytes<T> newBytes(this->capacity_, this->size_);
        std::memcpy(newBytes.mem_, this->mem_, this->size_);
        return newBytes;
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
