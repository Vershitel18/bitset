#include "bitset.h"

#include <algorithm>
#include <ostream>

namespace ct {
// View operations
BitSet operator~(const ConstView& a) {
  BitSet result(a);
  return result.flip();
}

BitSet operator&(const ConstView& a, const ConstView& b) {
  BitSet result(a);
  result &= b;
  return result;
}

BitSet operator|(const ConstView& a, const ConstView& b) {
  BitSet result(a);
  result |= b;
  return result;
}

BitSet operator^(const ConstView& a, const ConstView& b) {
  BitSet result(a);
  result ^= b;
  return result;
}

BitSet operator<<(const ConstView& a, size_t shift) {
  BitSet tmp(a);
  tmp <<= shift;
  return tmp;
}

BitSet operator>>(const ConstView& a, size_t shift) {
  BitSet tmp(a);
  tmp >>= shift;
  return tmp;
}

std::string to_string(const ConstView& view) {
  std::string result;
  result.reserve(view.size());

  for (std::size_t i = 0; i < view.size(); ++i) {
    result.push_back(view[i] ? '1' : '0');
  }

  return result;
}

std::ostream& operator<<(std::ostream& out_stream, const ConstView& view) {
  auto s = to_string(view);
  out_stream.write(s.data(), s.size());
  return out_stream;
}

// BitSet all
BitSet::BitSet()
    : _size(0)
    , _data(nullptr) {}

BitSet::BitSet(std::size_t size, bool value)
    : _size(size)
    , _data(new Word[ct::word_count(size)]()) {
  if (value) {
    std::size_t size_word = word_index(size);
    std::size_t tail_size = bit_offset(size);
    std::fill(_data, _data + size_word, ~Word{0});
    if (tail_size != 0) {
      _data[size_word] = low_mask(tail_size);
    }
  }
}

BitSet::BitSet(const BitSet& other)
    : _size(other._size)
    , _data(new Word[ct::word_count(other._size)]()) {
  std::size_t size_word = word_count(other._size);
  for (std::size_t i = 0; i < size_word; ++i) {
    *(_data + i) = *(other._data + i);
  }
}

BitSet::BitSet(std::string_view str)
    : _size(str.size())
    , _data(new Word[ct::word_count(str.size())]()) {
  for (std::size_t i = 0; i < str.size(); ++i) {
    (*this)[i] = (str[i] == '1');
  }
}

BitSet::BitSet(const ConstView& other)
    : BitSet(other.begin(), other.end()) {}

BitSet::BitSet(ConstIterator first, ConstIterator last)
    : _size(static_cast<size_t>(last - first))
    , _data(new Word[ct::word_count(_size)]()) {
  std::size_t word_length = word_count(size());
  for (std::size_t i = 0; i < word_length; ++i) {
    ConstIterator it = first + static_cast<std::ptrdiff_t>(i * WORD_BITS);
    const Word* word = it.word_ptr();
    const std::size_t bit_index = it.bit_offset();

    Word word_right = 0;
    if (bit_index == 0) {
      word_right = *word;
    } else {
      word_right = (*word >> bit_index);

      std::size_t bit_length_left = size() - i * WORD_BITS;

      if (bit_length_left > WORD_BITS - bit_index) {
        word_right |= (*(word + 1) << (WORD_BITS - bit_index));
      }
    }
    std::size_t bit_in_word = std::min(WORD_BITS, size() - i * WORD_BITS);
    _data[i] = word_right & low_mask(bit_in_word);
  }
}

BitSet& BitSet::operator=(const BitSet& other) & {
  BitSet(other).swap(*this);
  return *this;
}

BitSet& BitSet::operator=(std::string_view str) & {
  BitSet(str).swap(*this);
  return *this;
}

BitSet& BitSet::operator=(const ConstView& other) & {
  BitSet tmp(other);
  tmp.swap(*this);
  return *this;
}

BitSet::~BitSet() {
  delete[] _data;
}

void BitSet::swap(BitSet& other) noexcept {
  using std::swap;
  swap(_data, other._data);
  swap(_size, other._size);
}

std::size_t BitSet::size() const {
  return _size;
}

bool BitSet::empty() const {
  return size() == 0;
}

BitSet::Reference BitSet::operator[](std::size_t index) {
  return Reference{_data + ct::word_index(index), ct::bit_offset(index)};
}

BitSet::ConstReference BitSet::operator[](std::size_t index) const {
  return ConstReference{_data + ct::word_index(index), ct::bit_offset(index)};
}

BitSet::Iterator BitSet::begin() {
  return Iterator{_data, 0};
}

BitSet::ConstIterator BitSet::begin() const {
  return ConstIterator{_data, 0};
}

BitSet::Iterator BitSet::end() {
  return Iterator{_data, size()};
}

BitSet::ConstIterator BitSet::end() const {
  return ConstIterator{_data, size()};
}

BitSet operator~(const BitSet& lhs) {
  return {~lhs.subview()};
}

BitSet operator&(const BitSet& lhs, const BitSet& rhs) {
  BitSet tmp = lhs;
  tmp &= rhs;
  return tmp;
}

BitSet operator^(const BitSet& lhs, const BitSet& rhs) {
  BitSet tmp = lhs;
  tmp ^= rhs;
  return tmp;
}

BitSet operator|(const BitSet& lhs, const BitSet& rhs) {
  BitSet tmp = lhs;
  tmp |= rhs;
  return tmp;
}

BitSet& BitSet::operator&=(const ConstView& other) & {
  subview() &= other;
  return *this;
}

BitSet& BitSet::operator|=(const ConstView& other) & {
  subview() |= other;
  return *this;
}

BitSet& BitSet::operator^=(const ConstView& other) & {
  subview() ^= other;
  return *this;
}

BitSet& BitSet::operator<<=(std::size_t count) & {
  BitSet tmp(size() + count, false);
  std::copy_n(_data, ct::word_count(size()), tmp._data);
  swap(tmp);
  return *this;
}

BitSet& BitSet::operator>>=(std::size_t count) & {
  std::size_t bit_del = std::min(count, size());
  BitSet tmp(begin(), end() - bit_del);
  swap(tmp);
  return *this;
}

BitSet BitSet::operator<<(std::size_t shift) const {
  BitSet tmp = *this;
  tmp <<= shift;
  return tmp;
}

BitSet BitSet::operator>>(std::size_t shift) const {
  BitSet tmp = *this;
  tmp >>= shift;
  return tmp;
}

BitSet& BitSet::flip() & { // можно вызывать только у lvalue
  subview().flip();
  return *this;
}

BitSet& BitSet::set() & {
  subview().set();
  return *this;
}

BitSet& BitSet::reset() & {
  subview().reset();
  return *this;
}

bool BitSet::all() const {
  return subview().all();
}

bool BitSet::any() const {
  return subview().any();
}

std::size_t BitSet::count() const {
  return subview().count();
}

BitSet::operator ConstView() const {
  return subview();
}

BitSet::operator View() {
  return subview();
}

BitSet::View BitSet::subview(std::size_t offset, std::size_t count) {
  return make_subview_impl<View>(*this, offset, count);
}

BitSet::ConstView BitSet::subview(std::size_t offset, std::size_t count) const {
  return make_subview_impl<ConstView>(*this, offset, count);
}

void swap(BitSet& lhs, BitSet& rhs) noexcept {
  lhs.swap(rhs);
}
} // namespace ct
