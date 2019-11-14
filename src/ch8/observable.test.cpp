#include "ch8/observable.hpp"
#include <catch2/catch.hpp>

void observer_function()
{
}

class observer_functor {
public:
    auto operator()() -> void
    {
    }
};

TEST_CASE("Observer function can be attached")
{
    auto observable = ch8::observable<>{};
    observable.attach(observer_function);
}

TEST_CASE("Observer lambda can be attached")
{
    auto observable = ch8::observable<>{};
    observable.attach([]() {});
}

TEST_CASE("Observer capturing lambda can be attached")
{
    auto observable = ch8::observable<>{};
    auto captured_var = 39;
    observable.attach([&captured_var]() { captured_var++; });
}

TEST_CASE("Observer function object can be attached")
{
    auto observable = ch8::observable<>{};
    observable.attach(observer_functor{});
}

TEST_CASE("Multiple observers can be attached")
{
    auto observable = ch8::observable<>{};
    observable.attach([]() {});
    observable.attach(observer_function);
    observable.attach(observer_functor{});
}

TEST_CASE("Notify calls all attached observers")
{
    auto observable = ch8::observable<>{};

    auto o1_called = false;
    auto o2_called = false;
    auto o3_called = false;

    observable.attach([&o1_called]() { o1_called = true; });
    observable.attach([&o2_called]() { o2_called = true; });
    observable.attach([&o3_called]() { o3_called = true; });

    observable.notify();

    REQUIRE((o1_called && o2_called && o3_called));
}

TEST_CASE("Notify forwards arguments to observers")
{
    auto observable = ch8::observable<int>{};
    auto counter = 5;
    observable.attach([&counter](auto i) { counter += i; });
    observable.notify(5);
    REQUIRE(counter == 10);
}
