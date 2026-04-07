#include "bitset.h"
#include "test-helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <sstream>
#include <string>

namespace ct_test {

TEST_CASE("to_string(BitSet)") {
  std::string_view str = "11010001001101000100110100010011010001001101000100110100010011010001001101000100";
  const ct::BitSet bs(str);
  CHECK(to_string(bs) == str);
}

TEST_CASE("ostream << BitSet") {
  std::string_view str = "11010001001101000100110100010011010001001101000100110100010011010001001101000100";
  ct::BitSet bs(str);

  std::stringstream ss;
  ss << bs;
  CHECK(ss.str() == str);
}

TEST_CASE("to_string(ConstView)") {
  std::string_view str = "11010001001101000100110100010011010001001101000100110100010011010001001101000100";
  const ct::BitSet bs(str);

  CHECK(to_string(bs.subview(2, 20)) == str.substr(2, 20));
}

TEST_CASE("ostream << ConstView") {
  std::string_view str = "11010001001101000100110100010011010001001101000100110100010011010001001101000100";
  const ct::BitSet bs(str);

  std::stringstream ss;
  ss << bs.subview(2, 20);
  CHECK(ss.str() == str.substr(2, 20));
}

} // namespace ct_test
