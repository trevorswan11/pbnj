#include "testhelpers/stub.hh"

#include <stdx/types.hh>

namespace pbnj::tests::helpers {

auto some_really_complicated_work(i32 a, i32 b) noexcept -> i32 { return a + b; }

} // namespace pbnj::tests::helpers
