add_library(bitset
        )

# Add sources
target_sources(bitset
    PRIVATE
        "bitset.cpp"
    PUBLIC FILE_SET HEADERS FILES
        "bitset.h"
        "bitset-iterator.h"
        "bitset-reference.h"
        "bitset-view.h"
        "common.h"
)

ct_set_compiler_warnings(bitset)
