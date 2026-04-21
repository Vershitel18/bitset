#pragma once
#include "bitset-iterator.h"
#include "common.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string>

namespace ct {
class BitSet;

template <typename It>
class BitSetView {
public:
  using Value = bool;
  using Word = typename It::word_type;
  using Reference = typename It::reference;
  using ConstReference = BitsetReference<const Word>;
  using Iterator = It;
  using ConstIterator = BitsetIterator<const Word>;
  using View = BitSetView;
  using ConstView = BitSetView<ConstIterator>;
  static constexpr std::size_t NPOS = -1;

  BitSetView() = default;
  BitSetView(const BitSetView& other) = default;
  ~BitSetView() = default;

  operator ConstView() const {
    return ConstView(_begin, _end); // хзхз
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

  void swap(BitSetView& other) noexcept {
    using std::swap;
    swap(_begin, other._begin);
    swap(_end, other._end);
  }

  template <typename Operation>
  void unary_operations(Operation op) const {
    if (empty()) {
      return;
    }

    auto ptr_begin = _begin.word_ptr();
    const auto bit_begin = _begin.bit_offset();

    auto ptr_end = last_word_ptr(_end.word_ptr(), _end.bit_offset());
    const auto bit_end = _end.bit_offset();

    if (ptr_begin == ptr_end) {
      const Word right = (bit_end == 0) ? ~Word{0} : ct::low_mask(bit_end);
      op(*ptr_begin, (~Word{0} << bit_begin) & right);
      return;
    }

    if (bit_begin != 0) {
      op(*ptr_begin, ~Word{0} << bit_begin);
      ++ptr_begin;
    }

    while (ptr_begin < ptr_end) {
      op(*ptr_begin, ~Word{0});
      ++ptr_begin;
    }

    if (bit_end != 0) {
      op(*ptr_end, ct::low_mask(bit_end));
    } else {
      op(*ptr_end, ~Word{0});
    }
  }

  bool all() const {
    bool result = true;
    unary_operations([&](Word word, Word mask) {
      if ((word & mask) != mask) {
        result = false;
      }
    });
    return result;
  }

  bool any() const {
    bool result = false;
    unary_operations([&](Word word, Word mask) {
      if ((word & mask) != 0) {
        result = true;
      }
    });
    return result;
  }

  std::size_t count() const {
    std::size_t result = 0;
    unary_operations([&](Word word, Word mask) {
      result += __builtin_popcountll(word & mask);
    });
    return result;
  }

  BitSetView subview(std::size_t offset = 0, std::size_t count = NPOS) const {
    if (empty()) {
      return {end(), end()};
    }
    if (offset > size()) {
      return {end(), end()};
    }
    if (count == NPOS || offset + count > size()) {
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

  const View& unary_modefide_operations(Op op) const {
    unary_operations([&](Word& w, Word mask) {
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
    });
    return *this;
  }

  const View& flip() const {
    return unary_modefide_operations(Op::Flip);
  }

  const View& reset() const {
    return unary_modefide_operations(Op::Reset);
  }

  const View& set() const {
    return unary_modefide_operations(Op::Set);
  }

  template <typename TWord>
  static TWord* last_word_ptr(TWord* end_word_ptr, std::size_t end_bit_offset) noexcept {
    return end_bit_offset == 0 ? end_word_ptr - 1 : end_word_ptr;
  }

  static ct::Word word_build(const Word* current, const Word* last, std::size_t shift) noexcept {
    ct::Word cur = *current;
    if (shift == 0) {
      return cur;
    }

    ct::Word next = 0;
    if (current < last) {
      next = *(current + 1);
    }

    return (cur >> shift) | (next << (ct::WORD_BITS - shift));
  }

  template <typename Operation>
  const View& binary_operation(const ConstView& other, Operation op) const {
    if (empty()) {
      return *this;
    }

    const std::size_t dst_bit = begin().bit_offset();
    std::size_t src_bit = other.begin().bit_offset();
    const std::size_t bit_count = size();

    Word* p1 = begin().word_ptr();
    const Word* p2 = other.begin().word_ptr();
    const auto end2 = other.end();
    const Word* last2 = last_word_ptr(end2.word_ptr(), end2.bit_offset());

    auto apply_masked = [&](Word& dst, Word mask, Word rhs) {
      dst = (dst & ~mask) | (op(dst, rhs) & mask);
    };

    auto rhs_word = [&]() {
      return word_build(p2, last2, src_bit);
    };

    std::size_t remaining = bit_count;

    if (dst_bit != 0) {
      const std::size_t bits = std::min<std::size_t>(remaining, 64 - dst_bit);
      apply_masked(*p1, ct::low_mask(bits) << dst_bit, rhs_word() << dst_bit);
      remaining -= bits;
      src_bit += bits;
      p2 += src_bit / 64;
      src_bit %= 64; // потому что потом используется в rhs_word
      ++p1;
    }

    // while (remaining >= 64) {
    //   *p1 = op(*p1, rhs_word());
    //   remaining -= 64;
    //   ++p1;
    //   ++p2;
    // }
    while (remaining >= 64) {
      *p1 = op(*p1, rhs_word());

      remaining -= 64;
      ++p1;

      src_bit += 64;
      p2 += src_bit / 64;
      src_bit %= 64;
    }

    if (remaining != 0) {
      apply_masked(*p1, ct::low_mask(remaining), rhs_word());
    }

    return *this;
  }

  template <typename Operation>
  bool compare_operation(const ConstView& other, Operation op) const {
    if (size() != other.size()) {
      return false;
    }
    if (empty()) {
      return true;
    }

    std::size_t lhs_bit = begin().bit_offset();
    std::size_t rhs_bit = other.begin().bit_offset();
    std::size_t remaining = size();

    const Word* p1 = begin().word_ptr();
    const Word* p2 = other.begin().word_ptr();
    const auto end2 = other.end();
    const Word* last2 = last_word_ptr(end2.word_ptr(), end2.bit_offset());

    auto rhs_word = [&]() {
      return word_build(p2, last2, rhs_bit);
    };

    if (lhs_bit != 0) {
      const std::size_t bits = std::min<std::size_t>(remaining, 64 - lhs_bit);
      const Word mask = ct::low_mask(bits) << lhs_bit;
      if (!op(*p1 & mask, rhs_word() << lhs_bit & mask)) {
        return false;
      }
      remaining -= bits;
      rhs_bit += bits;
      p2 += rhs_bit / 64;
      rhs_bit %= 64;
      ++p1;
    }

    // while (remaining >= 64) {
    //   if (!op(*p1, rhs_word())) {
    //     return false;
    //   }
    //   remaining -= 64;
    //   ++p1;
    //   ++p2;
    // }
    while (remaining >= 64) {
      if (!op(*p1, rhs_word())) {
        return false;
      }

      remaining -= 64;
      ++p1;

      rhs_bit += 64;
      p2 += rhs_bit / 64;
      rhs_bit %= 64;
    }

    if (remaining != 0) {
      const Word mask = ct::low_mask(remaining);
      if (!op(*p1 & mask, rhs_word() & mask)) {
        return false;
      }
    }

    return true;
  }

  const View& operator&=(const ConstView& other) const {
    return binary_operation(other, [](Word a, Word b) {
      return a & b;
    });
  }

  const View& operator|=(const ConstView& other) const {
    return binary_operation(other, [](Word a, Word b) {
      return a | b;
    });
  }

  const View& operator^=(const ConstView& other) const {
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

using ConstView = BitSetView<BitsetIterator<const ct::Word>>;
using View = BitSetView<BitsetIterator<ct::Word>>;

inline bool operator==(const ConstView& lhs, const ConstView& rhs) {
  return lhs.compare_operation(rhs, [](ct::Word a, ct::Word b) {
    return a == b;
  });
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
