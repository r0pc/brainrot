// Do not change file. IDK how this works.
#pragma once


#include <cstddef>
#include <memory>
#include <utility>

using namespace std;

class ArenaAllocator {
public:
  explicit ArenaAllocator(const size_t max_size)
      : m_size{max_size}, m_buffer{new byte[max_size]}, m_offset{m_buffer} {}

  ArenaAllocator(const ArenaAllocator &) = delete;
  ArenaAllocator &operator=(const ArenaAllocator &) = delete;

  // Move Constructor
  ArenaAllocator(ArenaAllocator &&other) noexcept
      : m_size{exchange(other.m_size, 0)},
        m_buffer{exchange(other.m_buffer, nullptr)},
        m_offset{exchange(other.m_buffer, nullptr)} {}

  // Move Assignment operator
  ArenaAllocator &operator=(ArenaAllocator &&other) noexcept {
    std::swap(m_size, other.m_size);
    std::swap(m_buffer, other.m_buffer);
    std::swap(m_offset, other.m_offset);
    return *this;
  }

  // Allocates memory and something else. IDK how.
  template <typename T> [[nodiscard]] T *alloc() {
    size_t bytes_rem = m_size - static_cast<size_t>(m_offset - m_buffer);
    auto ptr = static_cast<void *>(m_offset);
    const auto aligned_addr = align(alignof(T), sizeof(T), ptr, bytes_rem);
    if (aligned_addr == nullptr) {
      throw bad_alloc{};
    }
    m_offset = static_cast<std::byte *>(aligned_addr) + sizeof(T);
    return static_cast<T *>(aligned_addr);
  }

  template <typename T, typename... Args>
  [[nodiscard]] T *emplace(Args &&...args) {
    const auto allocated_memory = alloc<T>();
    return new (allocated_memory) T{std::forward<Args>(args)...};
  }

  ~ArenaAllocator() { delete[] m_buffer; } // destructor

private:
  size_t m_size;
  byte *m_buffer;
  byte *m_offset;
};
