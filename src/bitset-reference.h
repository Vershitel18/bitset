#pragma once
#include "common.h"

#include <strings.h>

#include <cstddef>

namespace ct {

// TODO BitSet reference
template <typename TWord>
class BitsetReference {
  using ConstReference = BitsetReference<const TWord>;
  using Reference = BitsetReference;
  std::size_t _index;
  TWord* _data;

public:
  BitsetReference(TWord* data, size_t index)
      : _index(index)
      , _data(data) {}

  operator ConstReference() const {
    return ConstReference(_data, _index);
  }

  BitsetReference(const BitsetReference& other) = default;
  BitsetReference& operator=(const BitsetReference& other) = default;

  BitsetReference& operator=(const bool b) {
    if (b) {
      *_data |= (ct::Word{1} << _index);
    } else {
      *_data &= ~(ct::Word{1} << _index);
    }
    return *this;
  }

  operator bool() const {
    return (*_data >> _index) & ct::Word{1};
  }

  // bool operator==(bool b) const {
  //   return (*_data >> _index) & b;
  // }
  // bool operator!=(bool b) const {
  //   return !((*_data >> _index) & b);
  // }
  BitsetReference& flip() {
    *_data ^= (ct::Word{1} << _index);
    return *this;
  }
};

} // namespace ct
