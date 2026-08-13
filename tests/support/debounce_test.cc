#include <chrono> // IWYU pragma: keep
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <stdx/types.hh>

#include "support/debounce.hh"

namespace pbnj::tests {

using namespace std::chrono_literals;

TEST_CASE("Debounce leading edge execution") {
    u32      call_count = 0;
    debounce d{[&] { ++call_count; }, 50ms};

    SECTION("First call executes immediately") {
        CHECK(d());
        CHECK(call_count == 1);
    }

    SECTION("Rapid successive calls within interval are dropped") {
        CHECK(d());
        CHECK_FALSE(d());
        CHECK_FALSE(d());
        CHECK_FALSE(d());
        CHECK(call_count == 1);
    }

    SECTION("Call executes after interval elapses") {
        CHECK(d());
        CHECK(call_count == 1);

        std::this_thread::sleep_for(60ms);

        CHECK(d());
        CHECK(call_count == 2);
    }

    SECTION("Multiple cycles over time") {
        CHECK(d());
        CHECK(call_count == 1);

        for (u32 i = 0; i < 3; ++i) {
            std::this_thread::sleep_for(60ms);
            CHECK(d());
            CHECK_FALSE(d());
        }

        CHECK(call_count == 4);
    }
}

TEST_CASE("Debounce callback management") {
    SECTION("Constructed without callback returns false") {
        debounce d{50ms};
        CHECK_FALSE(d());

        u32 call_count = 0;
        d.set_callback([&] { ++call_count; });

        CHECK(d());
        CHECK(call_count == 1);
    }

    SECTION("set_callback updates the invoked function") {
        u32 first_count  = 0;
        u32 second_count = 0;

        debounce d{[&] { ++first_count; }, 50ms};
        CHECK(d());
        CHECK(first_count == 1);

        d.set_callback([&] { ++second_count; });

        std::this_thread::sleep_for(60ms);
        CHECK(d());
        CHECK(first_count == 1);
        CHECK(second_count == 1);
    }
}

} // namespace pbnj::tests
