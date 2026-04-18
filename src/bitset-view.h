#pragma once
#include "bitset-iterator.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

namespace ct {
class BitSet;

template <typename It>
class BitSetView {
public:
  using Value = bool;
  using Word = typename It::word_type;
  using Reference = It::reference;
  using ConstReference = BitsetReference<const Word>;
  using Iterator = It;
  using ConstIterator = BitsetIterator<const Word>;
  using View = BitSetView;
  using const_view = BitSetView<ConstIterator>;
  static constexpr std::size_t NPOS = std::numeric_limits<std::size_t>::max();

  BitSetView() = default;
  // BitSetView(BitsetIterator<const unsigned long> bs, size_t end);
  // BitSetView(const BitSet& bs);
  BitSetView(const BitSetView&) = default;
  ~BitSetView() = default;

  // BitSetView& operator=(const BitSetView& other) {
  //   if (this == &other) { return *this; }
  //   BitSetView tmp = other;
  //   swap(tmp);
  //   return *this;
  // }

  operator const_view() const {
    return const_view(_begin, _end); // хзхз
  }

  It begin() const {
    return _begin;
  }

  It end() const {
    return _end;
  }

  std::size_t size() const {
    return _end - _begin;
  }

  bool empty() const {
    return size() == 0;
  }

  Reference operator[](size_t idx) const {
    It tmp = _begin + idx;
    return *tmp;
  }

  void swap(BitSetView& other) {
    using std::swap;
    swap(_begin, other._begin);
    swap(_end, other._end);
  }

  bool all() const {
    if (empty()) {
      return false;
    }
    auto ptr_begin = _begin.word_ptr();
    auto index_begin = _begin.bit_offset();
    auto ptr_end = _end.word_ptr();
    auto index_end = _end.bit_offset();
    if (ptr_begin == ptr_end) { // не уверен что здесь вот это нужно вообще
      Word mask = (~Word{0} << index_begin) & ((Word{1} << index_end) - 1);
      return static_cast<bool>((*ptr_begin & mask) == mask);
    }
    if (index_begin != 0) {
      if ((*ptr_begin & (~Word{0} << index_begin)) != ~Word{0} << index_begin) {
        return false;
      }
      ++ptr_begin;
    }
    if (index_end > 0) {
      Word mask = (index_end == 64) ? ~Word{0} : (Word{1} << index_end) - 1;
      if ((*ptr_end & mask) != mask) {
        return false;
      }
      --ptr_end;
    }
    while (ptr_begin < ptr_end) {
      if (*ptr_begin != ~Word{0}) {
        return false;
      }
      ++ptr_begin;
    }
    return true;
  }

  bool any() const {
    if (empty()) {
      return false;
    }
    auto ptr_begin = _begin.word_ptr();
    auto index_begin = _begin.bit_offset();
    auto ptr_end = _end.word_ptr();
    auto index_end = _end.bit_offset();
    if (ptr_begin == ptr_end) { // не уверен что здесь вот это нужно вообще
      Word mask = (~Word{0} << index_begin) & ((index_end == 64) ? ~Word{0} : (Word{1} << index_end) - 1);
      return static_cast<bool>((*ptr_begin & mask) != 0);
    }
    if (index_begin != 0) {
      Word mask = ~Word{0} << index_begin;
      if ((*ptr_begin & mask) != 0) {
        return true;
      }
      ++ptr_begin;
    }
    if (index_end > 0) {
      Word mask = (index_end == 64) ? ~Word{0} : (Word{1} << index_end) - 1;
      if ((*ptr_end & mask) != 0) {
        return true;
      }
      --ptr_end;
    }
    while (ptr_begin <= ptr_end) {
      if (*ptr_begin != 0) {
        return true;
      }
      ++ptr_begin;
    }
    return false;
  }

  std::size_t count() const {
    if (empty()) {
      return 0;
    }
    size_t count = 0;
    auto ptr_begin = _begin.word_ptr();
    auto index_begin = _begin.bit_offset();
    auto ptr_end = _end.word_ptr();
    auto index_end = _end.bit_offset();
    if (ptr_begin == ptr_end) { // не уверен что здесь вот это нужно вообще
      Word mask = (~Word{0} << index_begin) & ((index_end == 64) ? ~Word{0} : (Word{1} << index_end) - 1);
      return __builtin_popcountll(static_cast<Word>(*ptr_begin & mask));
    }
    if (index_begin != 0) {
      Word mask = ~Word{0} << index_begin;
      count += __builtin_popcountll(static_cast<Word>(*ptr_begin & mask));
      ++ptr_begin;
    }
    if (index_end > 0) {
      Word mask = (index_end == 64) ? ~Word{0} : (Word{1} << index_end) - 1;
      count += __builtin_popcountll(static_cast<Word>(*ptr_end & mask));
      --ptr_end;
    }
    while (ptr_begin <= ptr_end) {
      count += __builtin_popcountll(static_cast<Word>(*ptr_begin));
      ++ptr_begin;
    }
    return count;
  }

  BitSetView subview(std::size_t offset = 0, std::size_t count = NPOS) const {
    if (offset > size()) {
      offset = size();
    } else if (count == NPOS || offset + count > size()) {
      count = size() - offset;
    }
    return BitSetView(_begin + offset, _begin + offset + count);
  }

  friend void swap(BitSetView& lhs, BitSetView& rhs) noexcept {
    lhs.swap(rhs);
  }
  enum class Op {
    Flip,
    Set,
    Reset
  };

  const View& apply(Op op) const {
    if (empty()) {
      return *this;
    }

    auto begin_it = this->begin();
    auto end_it = this->end();

    auto ptr_begin = begin_it.word_ptr();
    auto idx_begin = begin_it.bit_offset();

    auto ptr_end = end_it.word_ptr();
    auto idx_end = end_it.bit_offset();

    auto full_op = [&](Word& w) {
      switch (op) {
      case Op::Flip:
        w = ~w;
        break;
      case Op::Set:
        w = ~Word{0};
        break;
      case Op::Reset:
        w = Word{0};
        break;
      }
    };

    auto mask_op = [&](Word& w, Word mask) {
      switch (op) {
      case Op::Flip:
        w ^= mask;
        break;
      case Op::Set:
        w |= mask;
        break;
      case Op::Reset:
        w &= ~mask;
        break;
      }
    };

    if (ptr_begin == ptr_end) {
      Word mask = (~Word{0} << idx_begin) & ((idx_end == 64) ? ~Word{0} : ((Word{1} << idx_end) - 1));

      mask_op(*ptr_begin, mask);
      return *this;
    }

    if (idx_begin != 0) {
      Word mask = ~Word{0} << idx_begin;
      mask_op(*ptr_begin, mask);
      ++ptr_begin;
    }

    if (idx_end > 0) {
      Word mask = (idx_end == 64) ? ~Word{0} : ((Word{1} << idx_end) - 1);

      mask_op(*ptr_end, mask);
      --ptr_end;
    }

    while (ptr_begin <= ptr_end) {
      full_op(*ptr_begin);
      ++ptr_begin;
    }

    return *this;
  }

  const View& flip() const {
    return apply(Op::Flip);
  }

  const View& reset() const {
    return apply(Op::Reset);
  }

  const View& set() const {
    return apply(Op::Set);
  }

  template <typename Operation>
  const View& binary_operation(const const_view& other, Operation op) const {
    if (empty() || other.empty()) {
      return *this;
    }
    auto ptr1 = this->begin().word_ptr();
    auto idx1 = this->begin().bit_offset();

    auto ptr2 = other.begin().word_ptr();
    auto idx2 = other.begin().bit_offset();

    auto end_ptr1 = this->end().word_ptr();
    auto end_idx1 = this->end().bit_offset();

    auto get_rhs_word = [&](auto p) -> Word {
      const int shift = static_cast<int>(idx2) - static_cast<int>(idx1);

      Word cur = *p;

      Word next = 0;
      if ((p + 1) != other.end().word_ptr()) {
        next = *(p + 1);
      }

      if (shift == 0) {
        return cur;
      }
      return (cur >> shift) | (next << (64 - shift));
    };

    if (ptr1 == end_ptr1) {
      Word mask = (~Word{0} << idx1) & ((end_idx1 == 64) ? ~Word{0} : (Word{1} << end_idx1) - 1);

      Word rhs = get_rhs_word(ptr2);
      *ptr1 = (*ptr1 & ~mask) | (op(*ptr1, rhs) & mask);
      return *this;
    }

    if (idx1 != 0) {
      Word mask = ~Word{0} << idx1;
      Word rhs = get_rhs_word(ptr2);

      *ptr1 = (*ptr1 & ~mask) | (op(*ptr1, rhs) & mask);

      ++ptr1;
      ++ptr2;
    }

    while (ptr1 < end_ptr1) {
      Word rhs = get_rhs_word(ptr2);
      *ptr1 = op(*ptr1, rhs);

      ++ptr1;
      ++ptr2;
    }

    if (end_idx1 > 0) {
      Word mask = (end_idx1 == 64) ? ~Word{0} : (Word{1} << end_idx1) - 1;

      Word rhs = get_rhs_word(ptr2);
      *ptr1 = (*ptr1 & ~mask) | (op(*ptr1, rhs) & mask);
    }

    return *this; /// asdasdasdasd
  }

  // всего два типа const и non-const
  // const потому что конверсия неявная, а значит она возвращает rvalue
  // только константная ссылка принимает rvlaue
  const View& operator&=(const const_view& other) const {
    return binary_operation(other, [](Word a, Word b) {
      return a & b;
    });
  }

  const View& operator|=(const const_view& other) const {
    return binary_operation(other, [](Word a, Word b) {
      return a | b;
    });
  }

  const View& operator^=(const const_view& other) const {
    return binary_operation(other, [](Word a, Word b) {
      return a ^ b;
    });
  }

private:
  It _begin;
  It _end;

public:
  BitSetView(It begin, It end)
      : _begin(begin)
      , _end(end) {}
};

using ConstView = BitSetView<BitsetIterator<const uint64_t>>;
using View = BitSetView<BitsetIterator<uint64_t>>;

inline bool operator==(const ConstView& lhs, const ConstView& rhs) {
  if (lhs.size() == 0 && rhs.size() == 0) {
    return true;
  }
  if (lhs.size() != rhs.size()) {
    return false;
  }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

inline bool operator!=(const ConstView& lhs, const ConstView& rhs) {
  return !(lhs == rhs);
}

BitSet operator~(const ConstView& a);
BitSet operator&(const ConstView& a, const ConstView& b);
BitSet operator|(const ConstView& a, const ConstView& b);
BitSet operator^(const ConstView& a, const ConstView& b);
BitSet operator<<(const ConstView& a, size_t shift);
BitSet operator>>(const ConstView& a, size_t shift);
std::string to_string(const ConstView&);
std::ostream& operator<<(std::ostream&, const ConstView&);

} // namespace ct
