#include "ch8/frame_buffer.hpp"
#include <array>
#include <catch2/catch.hpp>
#include <type_traits>
#include <utility>

TEST_CASE("frame_buffer constructor initializes each pixel to 0, 0, 0, 0")
{
    const auto buffer = ch8::frame_buffer<64, 32>{};
    REQUIRE(buffer.data() == std::array<std::uint8_t, 64 * 32 * 4>{});
}

TEST_CASE("frame_buffer<64, 32>::width returns 64")
{
    const auto buffer = ch8::frame_buffer<64, 32>{};
    REQUIRE(buffer.width() == 64);
}

TEST_CASE("frame_buffer<83, 21>::width returns 83")
{
    const auto buffer = ch8::frame_buffer<83, 21>{};
    REQUIRE(buffer.width() == 83);
}

TEST_CASE("frame_buffer<64, 32>::height returns 32")
{
    const auto buffer = ch8::frame_buffer<64, 32>{};
    REQUIRE(buffer.height() == 32);
}

TEST_CASE("frame_buffer<83, 21>::height returns 21")
{
    const auto buffer = ch8::frame_buffer<83, 21>{};
    REQUIRE(buffer.height() == 21);
}

TEST_CASE(
    "Modifying the array returned by frame_buffer::data modifies the framebuffer")
{
    auto buffer = ch8::frame_buffer<2, 4>{};
    buffer.data().front() = 58;
    REQUIRE(buffer.data().front() == 58);
}

TEST_CASE("frame_buffer::data returns a const ref when frame_buffer is const")
{
    using buffer = ch8::frame_buffer<8, 4>;
    using result = decltype(std::declval<const buffer>().data());
    using const_data_ref = const std::array<std::uint8_t, 8 * 4 * 4>&;

    STATIC_REQUIRE(std::is_same_v<result, const_data_ref>);
}

TEST_CASE("frame_buffer::pixel returns the color of the pixel at (10, 30)")
{
    constexpr auto x = std::size_t{10};
    constexpr auto y = std::size_t{29};
    constexpr auto color = ch8::color{26, 105, 46, 234};

    auto buffer = ch8::frame_buffer<28, 30>{};
    auto& data = buffer.data();
    data.at(y * buffer.width() * 4 + x * 4 + 0) = color.r;
    data.at(y * buffer.width() * 4 + x * 4 + 1) = color.g;
    data.at(y * buffer.width() * 4 + x * 4 + 2) = color.b;
    data.at(y * buffer.width() * 4 + x * 4 + 3) = color.a;

    REQUIRE(buffer.pixel(x, y) == color);
}

TEST_CASE("frame_buffer::pixel returns the color of the pixel at (50, 20)")
{
    constexpr auto x = std::size_t{50};
    constexpr auto y = std::size_t{20};
    constexpr auto color = ch8::color{12, 84, 27, 85};

    auto buffer = ch8::frame_buffer<70, 35>{};
    auto& data = buffer.data();
    data.at(y * buffer.width() * 4 + x * 4 + 0) = color.r;
    data.at(y * buffer.width() * 4 + x * 4 + 1) = color.g;
    data.at(y * buffer.width() * 4 + x * 4 + 2) = color.b;
    data.at(y * buffer.width() * 4 + x * 4 + 3) = color.a;

    REQUIRE(buffer.pixel(x, y) == color);
}

TEST_CASE("frame_buffer::pixel sets the color of the pixel at (39, 13)")
{
    constexpr auto x = std::size_t{50};
    constexpr auto y = std::size_t{20};
    constexpr auto color = ch8::color{63, 26, 19, 157};

    auto buffer = ch8::frame_buffer<83, 27>{};
    buffer.pixel(x, y, color);

    REQUIRE(buffer.pixel(x, y) == color);
}

TEST_CASE("frame_buffer::pixel sets the color of the pixel at (3, 67)")
{
    constexpr auto x = std::size_t{3};
    constexpr auto y = std::size_t{67};
    constexpr auto color = ch8::color{83, 27, 55, 29};

    auto buffer = ch8::frame_buffer<38, 74>{};
    buffer.pixel(x, y, color);

    REQUIRE(buffer.pixel(x, y) == color);
}

TEST_CASE(
    "frame_buffer::clear sets each pixel to transparent when no color is provided")
{
    auto buffer = ch8::frame_buffer<83, 21>{};
    buffer.data().fill(std::uint8_t{83});

    buffer.clear();

    for (auto x = std::size_t{0}; x < buffer.width(); ++x) {
        for (auto y = std::size_t{0}; y < buffer.height(); ++y) {
            REQUIRE(buffer.pixel(x, y) == ch8::color{0, 0, 0, 0});
        }
    }
}

TEST_CASE("frame_buffer::clear sets each pixel to the color provided")
{
    constexpr auto color = ch8::color{73, 28, 95, 23};

    auto buffer = ch8::frame_buffer<83, 21>{};
    buffer.data().fill(std::uint8_t{83});

    buffer.clear(color);

    for (auto x = std::size_t{0}; x < buffer.width(); ++x) {
        for (auto y = std::size_t{0}; y < buffer.height(); ++y) {
            REQUIRE(buffer.pixel(x, y) == color);
        }
    }
}

TEST_CASE("color{103, 39, 38, 255} == color{103, 39, 38, 255}")
{
    REQUIRE(ch8::color{103, 39, 38, 255} == ch8::color{103, 39, 38, 255});
}

TEST_CASE("color{83, 67, 201, 143} == color{83, 67, 201, 143}")
{
    REQUIRE(ch8::color{83, 67, 201, 143} == ch8::color{83, 67, 201, 143});
}

TEST_CASE("color{103, 39, 38, 255} != color{38, 165, 44, 221}")
{
    REQUIRE(ch8::color{103, 39, 38, 255} != ch8::color{38, 165, 44, 221});
}

TEST_CASE("color{83, 67, 201, 143} != color{84, 32, 65, 93}")
{
    REQUIRE(ch8::color{83, 67, 201, 143} != ch8::color{84, 32, 65, 93});
}
