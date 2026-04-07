#include "bitset.h"
#include "test-helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <ranges>
#include <utility>

namespace ct_test {

TEST_CASE("bitset forward iteration") {
  SECTION("empty") {
    ct::BitSet bs;
    CHECK(bs.begin() == bs.end());
    CHECK(std::as_const(bs).begin() == std::as_const(bs).end());
  }

  SECTION("single word") {
    std::string_view str = "1101";
    std::vector bools = {true, true, false, true};

    ct::BitSet bs(str);

    CHECK_THAT(bs, Catch::Matchers::RangeEquals(bools));
    CHECK_THAT(std::as_const(bs), Catch::Matchers::RangeEquals(bools));
  }

  SECTION("multiple words") {
    std::string_view str = "11110110111010000100101111101000011011111111000001100110010010001011100100110101";
    std::vector bools = string_to_bools(str);

    ct::BitSet bs(str);

    CHECK_THAT(bs, Catch::Matchers::RangeEquals(bools));
    CHECK_THAT(std::as_const(bs), Catch::Matchers::RangeEquals(bools));
  }
}

TEST_CASE("bitset backward iteration") {
  SECTION("empty") {
    ct::BitSet bs;
    CHECK(bs.begin() == bs.end());
    CHECK(std::as_const(bs).begin() == std::as_const(bs).end());
  }

  SECTION("single word") {
    std::string_view str = "1101";
    std::vector bools = {true, false, true, true};

    ct::BitSet bs(str);

    CHECK_THAT(bs | std::views::reverse, Catch::Matchers::RangeEquals(bools));
    CHECK_THAT(std::as_const(bs) | std::views::reverse, Catch::Matchers::RangeEquals(bools));
  }

  SECTION("multiple words") {
    std::string_view str = "11110110111010000100101111101000011011111111000001100110010010001011100100110101";
    std::vector bools = string_to_bools(std::string(str.rbegin(), str.rend()));

    ct::BitSet bs(str);

    CHECK_THAT(bs | std::views::reverse, Catch::Matchers::RangeEquals(bools));
    CHECK_THAT(std::as_const(bs) | std::views::reverse, Catch::Matchers::RangeEquals(bools));
  }
}

TEST_CASE("bitset is iterable") {
  SECTION("single word") {
    std::string str = "1101";
    ct::BitSet bs(str);

    for (std::size_t i = 0; auto bit : bs) {
      CAPTURE(i);
      REQUIRE(bit == (str[i] == '1'));
      ++i;
    }
  }
  SECTION("multiple words") {
    std::string str = "11010001001101000100110100010011010001001101000100110100010011010001001101000100";
    ct::BitSet bs(str);

    for (std::size_t i = 0; auto bit : bs) {
      CAPTURE(i);
      REQUIRE(bit == (str[i] == '1'));
      ++i;
    }
  }
}

TEST_CASE("bitset is writable when iterating") {
  std::string str = "1101";
  ct::BitSet bs(str.size(), false);

  for (std::size_t i = 0; auto bit : bs) {
    bit = (str[i] == '1');
    ++i;
  }

  for (std::size_t i = 0; auto bit : bs) {
    CAPTURE(i);
    REQUIRE((bit ? '1' : '0') == str[i]);
    ++i;
  }
}

TEST_CASE("subview position and size are correct") {
  const ct::BitSet bs("110101");

  ct::BitSet::ConstView view_1 = bs.subview(1, 3);
  ct::BitSet::ConstView view_2 = bs.subview(1, ct::BitSet::NPOS);

  CHECK(view_1.size() == 3);
  CHECK(view_1.begin() == bs.begin() + 1);
  CHECK(view_1.end() == bs.end() - 2);

  CHECK(view_2.size() == bs.size() - 1);
  CHECK(view_2.begin() == bs.begin() + 1);
  CHECK(view_2.end() == bs.end());
}

TEST_CASE("full subview is equal to the original bitset") {
  const ct::BitSet bs("110101");
  CHECK(bs.subview() == bs);
}

TEST_CASE("empty subviews are equal") {
  const ct::BitSet bs_1("110101");
  const ct::BitSet bs_2("110101");
  CHECK(bs_1.subview(0, 0) == bs_2.subview(bs_2.size(), 0));
}

} // namespace ct_test
