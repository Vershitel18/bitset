#pragma once
#include "bitset-reference.h"
#include "common.h"

#include <cstddef>
#include <iterator>

namespace ct {
template <typename Word>
struct BitsetIterator {
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = bool;
  using reference = BitsetReference<Word>;
  using pointer = void;
  using word_type = Word;
  using ConstIterator = BitsetIterator<const word_type>;

  BitsetIterator() = default;
  BitsetIterator(const BitsetIterator&) = default;

  operator ConstIterator() const {
    return ConstIterator(_ptr, _index);
  }

  ~BitsetIterator() = default;

  reference operator*() const {
    return reference(_ptr + ct::word_index(_index), ct::bit_offset(_index));
  }

  Word* word_ptr() const noexcept {
    return _ptr + ct::word_index(_index);
  }

  std::size_t bit_offset() const noexcept {
    return ct::bit_offset(_index);
  }

  reference operator[](const difference_type idx) const {
    return *(*this + idx);
  }

  BitsetIterator& operator++() {
    _index++;
    return *this;
  }

  BitsetIterator operator++(int) {
    BitsetIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  BitsetIterator& operator--() {
    _index--;
    return *this;
  }

  BitsetIterator operator--(int) {
    BitsetIterator tmp = *this;
    --(*this);
    return tmp;
  }

  friend BitsetIterator operator+(difference_type n, const BitsetIterator& other) {
    return other + n;
  }

  friend difference_type operator-(const BitsetIterator& lhs, const BitsetIterator& rhs) {
    return (lhs._index - rhs._index);
  }

  BitsetIterator& operator+=(const difference_type n) {
    _index += n;
    return *this;
  }

  BitsetIterator& operator-=(const difference_type n) {
    _index -= n;
    return *this;
  }

  BitsetIterator operator+(const difference_type n) const {
    BitsetIterator tmp = *this;
    tmp += n;
    return tmp;
  }

  BitsetIterator operator-(const difference_type n) const {
    BitsetIterator tmp = *this;
    tmp -= n;
    return tmp;
  }

  std::strong_ordering operator<=>(const BitsetIterator& other) const {
    return _index <=> other._index;
  }

  friend bool operator==(const BitsetIterator& lhs, const BitsetIterator& rhs) {
    return lhs._ptr == rhs._ptr && lhs._index == rhs._index;
  }

private:
  Word* _ptr;
  difference_type _index;
  template <typename>
  friend struct BitsetIterator;

  BitsetIterator(Word* ptr, size_t index)
      : _ptr(ptr)
      , _index(static_cast<difference_type>(index)) {}
  friend class BitSet;
};

} // namespace ct
