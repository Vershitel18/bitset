#pragma once
#include <strings.h>

#include <cstddef>
#include <cstdint>

namespace ct {

// TODO BitSet reference
template <typename Word>
class BitsetReference {
  using ConstReference = BitsetReference<const Word>;
  using Reference = BitsetReference;
  std::size_t _index;
  Word* _data;

public:
  BitsetReference(Word* data, size_t index)
      : _index(index)
      , _data(data) {}

  operator ConstReference() const {
    return ConstReference(_data, _index);
  }

  BitsetReference(const BitsetReference& other) = default;
  BitsetReference& operator=(const BitsetReference& other) = default;

  BitsetReference& operator=(const bool b) {
    if (b) {
      *_data |= (Word{1} << _index);
    } else {
      *_data &= ~(Word{1} << _index);
    }
    return *this;
  }

  operator bool() const {
    return (*_data >> _index) & Word{1};
  }

  // bool operator==(bool b) const {
  //   return (*_data >> _index) & b;
  // }
  // bool operator!=(bool b) const {
  //   return !((*_data >> _index) & b);
  // }
  BitsetReference& flip() {
    *_data ^= (Word{1} << _index);
    return *this;
  }
};

} // namespace ct
