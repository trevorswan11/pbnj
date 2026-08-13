#include <catch2/catch_test_macros.hpp>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "testhelpers/unwrap.hh"

namespace pbnj::tests {

TEST_CASE("AI Stub") {
    stdx::option<i32> a{1};
    CHECK(UNWRAP(a) == 1);
}

} // namespace pbnj::tests
