add_executable(bitset-tests)

# Add sources
target_sources(bitset-tests
    PUBLIC FILE_SET HEADERS FILES
        "test-helpers.h"
    PRIVATE
        "test-helpers.cpp"
        "constructors-test.cpp"
        "iteration-test.cpp"
        "operations-test.cpp"
        "template-instantiation-test.cpp"
        "to-string-test.cpp"
        "traits-test.cpp"
)

# Setup warnings
ct_set_compiler_warnings(bitset-tests)

# Link with bitset
target_link_libraries(bitset-tests PRIVATE bitset)

# Link with dependencies
target_link_libraries(bitset-tests PRIVATE Catch2::Catch2WithMain)
