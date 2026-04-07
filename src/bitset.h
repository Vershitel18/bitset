#pragma once

#include <cstddef>
#include <string_view>

namespace ct {

class BitSet {
public:
  using Value = bool;
  using Reference = void;
  using ConstReference = void;
  using Iterator = void;
  using ConstIterator = void;
  using View = void;
  using ConstView = void;
  using Word = void;

  static constexpr std::size_t NPOS = -1;

  BitSet();
  BitSet(std::size_t size, bool value);
  BitSet(const BitSet& other);
  explicit BitSet(std::string_view str);
  explicit BitSet(const ConstView& other);
  BitSet(ConstIterator first, ConstIterator last);

  BitSet& operator=(const BitSet& other) &;
  BitSet& operator=(std::string_view str) &;
  BitSet& operator=(const ConstView& other) &;

  ~BitSet();

  void swap(BitSet& other);

  std::size_t size() const;
  bool empty() const;

  Reference operator[](std::size_t index);
  ConstReference operator[](std::size_t index) const;

  Iterator begin();
  ConstIterator begin() const;

  Iterator end();
  ConstIterator end() const;

  BitSet& operator&=(const ConstView& other) &;
  BitSet& operator|=(const ConstView& other) &;
  BitSet& operator^=(const ConstView& other) &;
  BitSet& operator<<=(std::size_t count) &;
  BitSet& operator>>=(std::size_t count) &;
  BitSet& flip() &;

  BitSet& set() &;
  BitSet& reset() &;

  bool all() const;
  bool any() const;
  std::size_t count() const;

  operator ConstView() const;
  operator View();

  View subview(std::size_t offset = 0, std::size_t count = NPOS);
  ConstView subview(std::size_t offset = 0, std::size_t count = NPOS) const;
};

bool operator==(const BitSet& left, const BitSet& right);
bool operator!=(const BitSet& left, const BitSet& right);

void swap(BitSet& lhs, BitSet& rhs);

} // namespace ct
