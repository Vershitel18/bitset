#include "bitset.h"


namespace ct {
// View operations
BitSet operator~(const ConstView& a) {
  BitSet result = a;
  return result.flip();
}

BitSet operator&(const ConstView& a, const ConstView& b) {
  BitSet result = a;
  result &= b;
  return result;
}
BitSet operator|(const ConstView& a, const ConstView& b) {
  BitSet result = a;
  result |= b;
  return result;
}
BitSet operator^(const ConstView& a, const ConstView& b) {
  BitSet result = a;
  result ^= b;
  return result;
}
BitSet operator<<(const ConstView& a, size_t shift) {
  BitSet tmp = a;
  tmp <<= shift;
  return tmp;
}
BitSet operator>>(const ConstView& a, size_t shift) {
  BitSet tmp = a;
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
  return out_stream << to_string(view);
}

// BitSet all
BitSet::BitSet() :_size(0), _data(nullptr) {}

BitSet::BitSet(std::size_t size, bool value) : _size(size), _data(new Word[(size + 63) / 64]()) {
    for (std::size_t i = 0; i < size; ++i) {
      (*this)[i] = value;
    }
  }
  BitSet::BitSet(const BitSet& other) : _size(other._size), _data(new Word[(other._size + 63) / 64]()) {
    for (std::size_t i = 0; i < other._size; ++i) {
      (*this)[i] = other[i];
    }
  }
  BitSet::BitSet(std::string_view str) : _size(str.size()), _data(new Word[(str.size() + 63) / 64]()) {
    for (std::size_t i = 0; i < str.size(); ++i) {
      (*this)[i] = (str[i] == '1');
    }
  }
  BitSet::BitSet(const ConstView& other) : _size(other.size()), _data(new Word[(other.size() + 63) / 64]()) {
    for (std::size_t i = 0; i < other.size(); ++i) {
      (*this)[i] = other[i]; // возвращает разыменованный итератор и присваивает его прокси-ссылке
    }
  }

  BitSet::BitSet(ConstIterator first, ConstIterator last) :
    _size(last._index - first._index),
    _data(new Word[(last._index - first._index + 63) / 64]()) {
    for (std::size_t i = 0; i < _size; ++i) {
      (*this)[i] = first[i];
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
  void BitSet::swap(BitSet& other) {
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
    return Reference{_data + (index / 64), index % 64};
  }
  BitSet::ConstReference BitSet::operator[](std::size_t index) const {
    return ConstReference{_data + (index / 64), index % 64};
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
    if (empty()) {return *this; }
    subview() &= other;
    return *this;
  }
  BitSet& BitSet::operator|=(const ConstView& other) & {
  if (empty()) {return *this; }
    subview() |= other;
    return *this;
  }
  BitSet& BitSet::operator^=(const ConstView& other) & {
  if (empty()) {return *this; }
    subview() ^= other;
    return *this;
  }
BitSet& BitSet::operator<<=(std::size_t count) & {
  BitSet tmp(size() + count, false);
  std::copy_n(_data, (size() + 63) / 64, tmp._data);
  swap(tmp);
  return *this;
}

BitSet& BitSet::operator>>=(std::size_t count) & {
  std::size_t erase = std::min(count, size());
  BitSet tmp(begin(), end() - erase);
  swap(tmp);
  return *this;
}
  BitSet BitSet::operator<<(std::size_t shift) const {
  if (empty()) {return {shift, false}; }
    BitSet tmp = *this;
    tmp <<= shift;
    return tmp;
  }
  BitSet BitSet::operator>>(std::size_t shift) const {
  if (empty()) {return {shift, true}; }
  BitSet tmp = *this;
  tmp >>= shift;
  return tmp;
  }

  BitSet& BitSet::flip() & { // можно вызывать только у lvalue
  if (empty()) {return *this; }
    subview().flip();
    return *this;
  }

  BitSet& BitSet::set() & {
  if (empty()) {return *this; }
    subview().set();
    return *this;
  }

  BitSet& BitSet::reset() & {
  if (empty()) {return *this; }
    subview().reset();
    return *this;
  }

  bool BitSet::all() const {
  if (empty()) {return true; }
    for (std::size_t i = 0; i < size(); ++i) {
      if (!(*this)[i]) {
        return false;
      }
    }
    return true;
  }
  bool BitSet::any() const {
  if (empty()) {return false; }
    for (std::size_t i = 0; i < size(); ++i) {
      if ((*this)[i]) {
        return true;
      }
    }
    return false;
  }
  std::size_t BitSet::count() const {
    if (empty()) {return 0; }
    std::size_t count = 0;
    for (std::size_t i = 0; i < size(); ++i) {
      if ((*this)[i]) {
        ++count; // пока тупая реализация
      }
    }
    return count;
  }

  BitSet::operator ConstView() const {
    return subview();
  }

  BitSet::operator View() {
    return subview();
  }

  BitSet::View BitSet::subview(std::size_t offset, std::size_t count) {
    if (empty()) {return {end(), end()};}
    std::size_t n = size();

    if (offset > n)
      offset = n;

    std::size_t avail = n - offset;

    if (count == NPOS || count > avail)
      count = avail;

    auto first = begin() + static_cast<std::ptrdiff_t>(offset);
    auto last  = first + static_cast<std::ptrdiff_t>(count);

    return {first, last};
  }

  BitSet::ConstView BitSet::subview(std::size_t offset, std::size_t count) const {
  if (empty()) {return {end(), end()};}
    std::size_t n = size();

    if (offset > n)
      offset = n;

    std::size_t avail = n - offset;

    if (count == NPOS || count > avail)
      count = avail;

    auto first = begin() + static_cast<std::ptrdiff_t>(offset);
    auto last  = first + static_cast<std::ptrdiff_t>(count);

    return {first, last};
}

void swap(BitSet& lhs, BitSet& rhs) {
    lhs.swap(rhs);
  }
} // namespace ct
