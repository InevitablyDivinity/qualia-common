#pragma once
#include "common/algorithm.hpp"
#include <cstddef>

namespace ql
{

class String
{
public:

  using iterator = char *;
  using const_iterator = const char *;

  String() = default;

  String(const char *src)
  {
    if (src != nullptr)
      assign(src, std::strlen(src));
  }

  String(const char *src, std::size_t size)
  {
    assign(src, size);
  }

  String(const String &src)
  {
    assign(src);
  }

  String(String &&src)
  {
    assign(src);
  }

  ~String()
  {
    destruct();
  }

  String &operator=(const char *rhs)
  {
    if (!empty())
      destruct();

    assign(rhs);
    return *this;
  }

  String &operator=(const String &rhs)
  {
    if (!empty())
      destruct();

    assign(rhs);
    return *this;
  }

  String &operator=(String &&rhs)
  {
    if (!empty())
      destruct();

    assign(rhs);
    return *this;
  }

  bool operator==(const String &rhs) const
  {
    return std::strncmp(m_data, rhs.m_data, m_size) == 0;
  }

  bool operator==(const char *rhs) const
  {
    return std::strcmp(m_data, rhs) == 0;
  }

  std::size_t find_last_of(char c) const
  {
    for (auto i = 0; i < m_size; i++)
    {
      if (m_data[i] == c)
        return i;
    }

    return m_size;
  }

  operator const char *() const { return m_data; }

  char *data() { return m_data; }
  const char *data() const { return m_data; }
  const char *c_str() const { return m_data; }

  std::size_t size() const { return m_size; }
  bool empty() const { return m_size > 0; }

  iterator begin() { return m_data; }
  iterator end() { return nullptr; }
  const_iterator begin() const { return m_data; }
  const_iterator cbegin() const { return m_data; }
  const_iterator end() const { return nullptr; }
  const_iterator cend() const { return nullptr; }

  void clear()
  {
    if (!empty())
      destruct();
  }

  void assign(const char *src, std::size_t size)
  {
    if (src == nullptr)
      return;

    if (size + 1 > alignof(std::max_align_t))
      m_data = new char[size + 1];
    else
      m_data = reinterpret_cast<char *>(&m_stackBuffer);

    std::strncpy(m_data, src, size);
    m_data[size] = '\0';
    m_size = size;
  }

  void assign(const String &src)
  {
    assign(src.m_data, src.m_size);
  }

  void assign(String &&src)
  {
    ql::swap(m_data, src.m_data);
    ql::swap(m_size, src.m_size);
    ql::copy(&m_stackBuffer, &m_stackBuffer, &src.m_stackBuffer);
  }

private:

  bool using_ssbo() const
  {
    return m_size + 1 <= alignof(std::max_align_t);
  }

  void destruct()
  {
    if (using_ssbo())
    {
      std::memset(&m_stackBuffer, 0, alignof(std::max_align_t));
    }
    else
    {
      delete[] m_data;
    }

    m_size = 0;
  }

  std::max_align_t m_stackBuffer;
  char *m_data = nullptr;
  std::size_t m_size = 0;
};

/*template<typename Type>
std::uint64_t hash(const Type &value);

template <>
std::uint64_t hash<String>(const String &value)
{
  return fnv1a_hash(value.data(), value.size());
}*/

template<typename Type>
struct hash;

template<>
struct hash<String>
{
  std::size_t operator()(const String &value)
  {
    return fnv1a_hash(value.data(), value.size());
  }
};

} // namespace ql
