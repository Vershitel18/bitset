#pragma once

#include <cstddef>
#include <cstdint>

namespace ct {

using Word = uint64_t;

inline constexpr std::size_t WORD_BITS = 64;

constexpr std::size_t word_index(std::size_t bit_index) noexcept {
  return bit_index / WORD_BITS;
}

constexpr std::size_t bit_offset(std::size_t bit_index) noexcept {
  return bit_index % WORD_BITS;
}

constexpr std::size_t word_count(std::size_t bit_count) noexcept {
  return (bit_count + WORD_BITS - 1) / WORD_BITS;
}

constexpr Word low_mask(std::size_t bit_count) noexcept {
  return bit_count >= WORD_BITS ? ~Word{0} : ((Word{1} << bit_count) - 1);
}

} // namespace ct
