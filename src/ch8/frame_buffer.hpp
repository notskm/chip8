#ifndef CH8_FRAME_BUFFER_HPP
#define CH8_FRAME_BUFFER_HPP

#include <array>
#include <cstdint>
#include <gsl-lite/gsl-lite.hpp>

namespace ch8 {
    struct color {
        constexpr auto operator==(const color& other) const noexcept -> bool;
        constexpr auto operator!=(const color& other) const noexcept -> bool;

        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
    };

    template <std::size_t Width, std::size_t Height>
    class frame_buffer {
    public:
        using rgba_array = std::array<std::uint8_t, Width * Height * 4>;

        [[nodiscard]] constexpr auto width() const noexcept -> std::size_t;
        [[nodiscard]] constexpr auto height() const noexcept -> std::size_t;
        [[nodiscard]] constexpr auto data() noexcept -> rgba_array&;
        [[nodiscard]] constexpr auto data() const noexcept -> const rgba_array&;
        [[nodiscard]] constexpr auto pixel(std::size_t x, std::size_t y) const
            noexcept -> color;
        constexpr auto pixel(std::size_t x, std::size_t y, color color) noexcept
            -> void;
        constexpr auto clear(const color& color = {0, 0, 0, 0}) noexcept
            -> void;

    private:
        rgba_array rgba_data;
    };
} // namespace ch8

constexpr auto ch8::color::operator==(const color& other) const noexcept -> bool
{
    return this->r == other.r && this->g == other.g && this->b == other.b &&
           this->a == other.a;
}

constexpr auto ch8::color::operator!=(const color& other) const noexcept -> bool
{
    return !(*this == other);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CH8_FRAME_BUFFER ch8::frame_buffer<Width, Height>

template <std::size_t Width, std::size_t Height>
constexpr auto CH8_FRAME_BUFFER::width() const noexcept -> std::size_t
{
    return Width;
}

template <std::size_t Width, std::size_t Height>
constexpr auto CH8_FRAME_BUFFER::height() const noexcept -> std::size_t
{
    return Height;
}

template <std::size_t Width, std::size_t Height>
constexpr auto CH8_FRAME_BUFFER::data() const noexcept -> const rgba_array&
{
    return rgba_data;
}

template <std::size_t Width, std::size_t Height>
constexpr auto CH8_FRAME_BUFFER::data() noexcept -> rgba_array&
{
    return rgba_data;
}

template <std::size_t Width, std::size_t Height>
constexpr auto CH8_FRAME_BUFFER::pixel(std::size_t x, std::size_t y) const
    noexcept -> color
{
    gsl_Expects(x < Width);
    gsl_Expects(y < Height);

    return {rgba_data[y * Width * 4 + x * 4 + 0],
            rgba_data[y * Width * 4 + x * 4 + 1],
            rgba_data[y * Width * 4 + x * 4 + 2],
            rgba_data[y * Width * 4 + x * 4 + 3]};
}

template <std::size_t Width, std::size_t Height>
constexpr auto
CH8_FRAME_BUFFER::pixel(std::size_t x, std::size_t y, color color) noexcept
    -> void
{
    gsl_Expects(x < Width);
    gsl_Expects(y < Height);

    rgba_data[y * Width * 4 + x * 4 + 0] = color.r;
    rgba_data[y * Width * 4 + x * 4 + 1] = color.g;
    rgba_data[y * Width * 4 + x * 4 + 2] = color.b;
    rgba_data[y * Width * 4 + x * 4 + 3] = color.a;
}

template <std::size_t Width, std::size_t Height>
constexpr auto CH8_FRAME_BUFFER::clear(const color& color) noexcept -> void
{
    for (auto x = std::size_t{0}; x < Width; ++x) {
        for (auto y = std::size_t{0}; y < Height; ++y) {
            pixel(x, y, color);
        }
    }
}

#undef CH8_FRAME_BUFFER

#endif // CH8_FRAME_BUFFER_HPP
