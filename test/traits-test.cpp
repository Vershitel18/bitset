#include "bitset.h"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace ct_test {

TEST_CASE("member types") {
  SECTION("bitset") {
    STATIC_CHECK(std::is_same_v<ct::BitSet::Value, bool>);
    STATIC_CHECK_FALSE(std::is_same_v<ct::BitSet::Reference, bool>);
    STATIC_CHECK(std::numeric_limits<ct::BitSet::Word>::digits >= 32);
  }

  SECTION("iterators") {
    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::Iterator>::value_type, ct::BitSet::Value>);
    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::ConstIterator>::value_type, ct::BitSet::Value>);

    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::Iterator>::reference, ct::BitSet::Reference>);
    STATIC_CHECK(
        std::is_same_v<std::iterator_traits<ct::BitSet::ConstIterator>::reference, ct::BitSet::ConstReference>
    );

    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::Iterator>::difference_type, std::ptrdiff_t>);
    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::ConstIterator>::difference_type, std::ptrdiff_t>);

    STATIC_CHECK(
        std::is_same_v<std::iterator_traits<ct::BitSet::Iterator>::iterator_category, std::random_access_iterator_tag>
    );
    STATIC_CHECK(
        std::is_same_v<
            std::iterator_traits<ct::BitSet::ConstIterator>::iterator_category,
            std::random_access_iterator_tag>
    );

    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::Iterator>::pointer, void>);
    STATIC_CHECK(std::is_same_v<std::iterator_traits<ct::BitSet::ConstIterator>::pointer, void>);
  }

  SECTION("views") {
    STATIC_CHECK(std::is_same_v<ct::BitSet::View::Value, ct::BitSet::Value>);
    STATIC_CHECK(std::is_same_v<ct::BitSet::ConstView::Value, ct::BitSet::Value>);

    STATIC_CHECK(std::is_same_v<ct::BitSet::View::Reference, ct::BitSet::Reference>);
    STATIC_CHECK(std::is_same_v<ct::BitSet::View::ConstReference, ct::BitSet::ConstReference>);
    STATIC_CHECK(std::is_same_v<ct::BitSet::ConstView::Reference, ct::BitSet::ConstReference>);
    STATIC_CHECK(std::is_same_v<ct::BitSet::ConstView::ConstReference, ct::BitSet::ConstReference>);

    STATIC_CHECK(std::is_same_v<typename ct::BitSet::View::Iterator, typename ct::BitSet::Iterator>);
    STATIC_CHECK(std::is_same_v<typename ct::BitSet::View::ConstIterator, typename ct::BitSet::ConstIterator>);
    STATIC_CHECK(std::is_same_v<typename ct::BitSet::ConstView::Iterator, typename ct::BitSet::ConstIterator>);
    STATIC_CHECK(std::is_same_v<typename ct::BitSet::ConstView::ConstIterator, typename ct::BitSet::ConstIterator>);
  }
}

TEST_CASE("concepts") {
  STATIC_CHECK(std::random_access_iterator<ct::BitSet::ConstIterator>);
  STATIC_CHECK(std::random_access_iterator<ct::BitSet::Iterator>);
}

TEST_CASE("triviality") {
  SECTION("references") {
    STATIC_CHECK(std::is_trivially_copyable_v<ct::BitSet::Reference>);
    STATIC_CHECK(std::is_trivially_copyable_v<ct::BitSet::ConstReference>);
    STATIC_CHECK_FALSE(std::is_default_constructible_v<ct::BitSet::Reference>);
  }
  SECTION("iterators") {
    STATIC_CHECK(std::is_trivial_v<ct::BitSet::Iterator>);
    STATIC_CHECK(std::is_trivial_v<ct::BitSet::ConstIterator>);
  }
  SECTION("views") {
    STATIC_CHECK(std::is_trivially_copyable_v<ct::BitSet::View>);
    STATIC_CHECK(std::is_trivially_copyable_v<ct::BitSet::ConstView>);
  }
}

TEST_CASE("conversions") {
  STATIC_CHECK(std::convertible_to<ct::BitSet::Iterator, ct::BitSet::ConstIterator>);
  STATIC_CHECK(std::convertible_to<const ct::BitSet::Iterator, const ct::BitSet::ConstIterator>);
}

} // namespace ct_test
