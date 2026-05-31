#pragma once
#include "bitset-iterator.h"
#include "common.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>

namespace ct {
class BitSet;

template <typename It>
class BitSetView {
public:
  using Value = bool;
  using Word = ct::Word;
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
    return ConstView(_begin, _end);
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

  bool any() const {
    return unary_operation([](Word word, Word mask) {
      return (word & mask) != 0;
    });
  }

  bool all() const {
    return !unary_operation([](Word word, Word mask) {
      return (word & mask) != mask;
    });
  }

  std::size_t count() const {
    std::size_t result = 0;

    unary_operation([&](Word word, Word mask) {
      result += std::popcount(word & mask);
      return false;
    });

    return result;
  }

  BitSetView subview(std::size_t offset = 0, std::size_t count = NPOS) const {
    if (empty() || offset > size()) {
      return {end(), end()};
    }
    if (count == NPOS || count > size() - offset) {
      count = size() - offset;
    }
    return BitSetView(_begin + offset, _begin + offset + count);
  }

  friend void swap(BitSetView& lhs, BitSetView& rhs) noexcept {
    lhs.swap(rhs);
  }

  template <typename Op>
  const View& unary_modified_operations(Op op) const {
    unary_operation([&](Word& w, Word mask) {
      op(w, mask);
      return false;
    });

    return *this;
  }

  const View& flip() const {
    return unary_modified_operations([](Word& w, Word mask) {
      w ^= mask;
    });
  }

  const View& reset() const {
    return unary_modified_operations([](Word& w, Word mask) {
      w &= ~mask;
    });
  }

  const View& set() const {
    return unary_modified_operations([](Word& w, Word mask) {
      w |= mask;
    });
  }

  const View& operator&=(const ConstView& other) const {
    return binary_operation(other, std::bit_and<>());
  }

  const View& operator|=(const ConstView& other) const {
    return binary_operation(other, std::bit_or<>());
  }

  const View& operator^=(const ConstView& other) const {
    return binary_operation(other, std::bit_xor<>());
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
    std::size_t bit_count = size();

    const Word* p1 = begin().word_ptr();
    const Word* p2 = other.begin().word_ptr();
    const auto end2 = other.end();
    const Word* last2 = last_word_ptr(end2.word_ptr(), end2.bit_offset());

    if (lhs_bit != 0) {
      const std::size_t bits = std::min<std::size_t>(bit_count, WORD_BITS - lhs_bit);
      const Word mask = ct::low_mask(bits) << lhs_bit;
      if (!op(*p1 & mask, (word_build(p2, last2, rhs_bit) << lhs_bit) & mask)) {
        return false;
      }
      bit_count -= bits;
      rhs_bit += bits;
      p2 += word_index(rhs_bit);
      rhs_bit = bit_offset(rhs_bit);
      ++p1;
    }
    while (bit_count >= WORD_BITS) {
      if (!op(*p1, word_build(p2, last2, rhs_bit))) {
        return false;
      }

      bit_count -= WORD_BITS;
      ++p1;

      rhs_bit += WORD_BITS;
      p2 += word_index(rhs_bit);
      rhs_bit = bit_offset(rhs_bit);
    }

    if (bit_count != 0) {
      const Word mask = ct::low_mask(bit_count);
      if (!op(*p1 & mask, word_build(p2, last2, rhs_bit) & mask)) {
        return false;
      }
    }

    return true;
  }

private:
  It _begin;
  It _end;

  template <typename Operation>
  const View& binary_operation(const ConstView& other, Operation op) const {
    if (empty()) {
      return *this;
    }

    const std::size_t bit_index_this = begin().bit_offset();
    std::size_t bit_index_other = other.begin().bit_offset();
    std::size_t bit_count = size();

    Word* p1 = begin().word_ptr();
    const Word* p2 = other.begin().word_ptr();
    const auto end2 = other.end();
    const Word* last2 = last_word_ptr(end2.word_ptr(), end2.bit_offset());

    auto word_masked = [&](Word& lhs, Word mask, Word rhs) {
      lhs = (lhs & ~mask) | (op(lhs, rhs) & mask);
    }; // лямбда функция для применения маски

    // Обработка головы: если первое слово частичное
    if (bit_index_this != 0) {
      const std::size_t bits = std::min<std::size_t>(bit_count, WORD_BITS - bit_index_this);
      word_masked(*p1, ct::low_mask(bits) << bit_index_this, word_build(p2, last2, bit_index_other) << bit_index_this);
      bit_count -= bits;
      bit_index_other += bits;
      p2 += word_index(bit_index_other);
      bit_index_other = bit_offset(bit_index_other);
      ++p1;
    }

    // Обработка полных слов
    while (bit_count >= WORD_BITS) {
      *p1 = op(*p1, word_build(p2, last2, bit_index_other));
      bit_count -= WORD_BITS;
      ++p1;

      bit_index_other += WORD_BITS;
      p2 += word_index(bit_index_other);
      bit_index_other = bit_offset(bit_index_other);
    }

    // Обработка хвоста: если последнее слово частичное
    if (bit_count != 0) {
      word_masked(*p1, ct::low_mask(bit_count), word_build(p2, last2, bit_index_other));
    }

    return *this;
  }

  template <typename Operation>
  bool unary_operation(Operation op) const {
    if (empty()) {
      return false;
    }

    auto ptr_begin = _begin.word_ptr();
    const auto bit_begin = _begin.bit_offset();

    auto ptr_end = last_word_ptr(_end.word_ptr(), _end.bit_offset());
    const auto bit_end = _end.bit_offset();

    if (ptr_begin == ptr_end) {
      const Word right = (bit_end == 0) ? ~Word{0} : ct::low_mask(bit_end);
      return op(*ptr_begin, (~Word{0} << bit_begin) & right);
    }

    if (bit_begin != 0) {
      if (op(*ptr_begin, ~Word{0} << bit_begin)) {
        return true;
      }
      ++ptr_begin;
    }

    while (ptr_begin < ptr_end) {
      if (op(*ptr_begin, ~Word{0})) {
        return true;
      }
      ++ptr_begin;
    }

    const Word last_mask = (bit_end == 0) ? ~Word{0} : ct::low_mask(bit_end);
    return op(*ptr_end, last_mask);
  }

  template <typename TWord>
  static TWord* last_word_ptr(TWord* end_word_ptr, std::size_t end_bit_offset) noexcept {
    return end_bit_offset == 0 ? end_word_ptr - 1 : end_word_ptr;
  } // Получаем указатель на последнее валидное слово(если bit_offset == 0, значит искомое слово предыдущее)

  // функция сбори слова из двух слов, с проверко выхода за гранницы(last_word)
  static Word word_build(const Word* word, const Word* last_word, std::size_t shift) noexcept {
    Word word_one = *word;
    if (shift == 0) {
      return word_one;
    }

    Word next = 0;
    if (word < last_word) {
      next = *(word + 1);
    }

    return (word_one >> shift) | (next << (WORD_BITS - shift));
  }

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
