#pragma once

#include <stdx/result.hh>
#include <stdx/types.hh>

namespace pbnj {

enum class error : u8 {};

template <typename T> using result = stdx::result<T, error>;

} // namespace pbnj
