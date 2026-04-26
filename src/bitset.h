#pragma once

#include "bitset-iterator.h"
#include "bitset-reference.h"
#include "bitset-view.h"
#include "common.h"

#include <cstddef>
#include <string_view>

namespace ct {

class BitSet {
public:
  using Value = bool;
  using Word = ct::Word;
  using Reference = BitsetReference<Word>;
  using ConstReference = BitsetReference<const Word>;
  using Iterator = BitsetIterator<Word>;
  using ConstIterator = BitsetIterator<const Word>;
  using View = BitSetView<Iterator>;
  using ConstView = BitSetView<ConstIterator>;

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

  void swap(BitSet& other) noexcept;

  std::size_t size() const;
  bool empty() const;

  Reference operator[](std::size_t index);
  ConstReference operator[](std::size_t index) const;
  Iterator begin();
  ConstIterator begin() const;

  Iterator end();
  ConstIterator end() const;
  friend BitSet operator~(const BitSet& lhs);
  friend BitSet operator&(const BitSet& lhs, const BitSet& rhs);
  friend BitSet operator^(const BitSet& lhs, const BitSet& rhs);
  friend BitSet operator|(const BitSet& lhs, const BitSet& rhs);

  BitSet& operator&=(const ConstView& other) &;
  BitSet& operator|=(const ConstView& other) &;
  BitSet& operator^=(const ConstView& other) &;
  BitSet& operator<<=(std::size_t count) &;
  BitSet& operator>>=(std::size_t count) &;
  BitSet operator<<(std::size_t shift) const;
  BitSet operator>>(std::size_t shift) const;

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

private:
  template <typename ViewType, typename Self>
  static ViewType make_subview_impl(Self& self, std::size_t offset, std::size_t count) {
    if (self.empty() || offset > self.size()) {
      return {self.end(), self.end()};
    }
    if (count == NPOS || count > self.size() - offset) {
      count = self.size() - offset;
    }
    return {self.begin() + offset, self.begin() + offset + count};
  }

  size_t _size;
  Word* _data;
};

void swap(BitSet& lhs, BitSet& rhs) noexcept;
} // namespace ct
