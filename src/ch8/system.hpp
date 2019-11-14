#ifndef CHIP8_SYSTEM_HPP
#define CHIP8_SYSTEM_HPP

#include "ch8/frame_buffer.hpp"
#include "ch8/observable.hpp"
#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <random>

namespace ch8 {
    struct chip8_data;
    class chip8_system;

    struct chip8_data {
        static constexpr auto program_start = 0x200U;

        chip8_data() noexcept;
        std::uint16_t program_counter;
        std::uint16_t i_register;
        std::uint8_t delay_timer;
        std::uint8_t sound_timer;
        std::int8_t stack_pointer;
        std::array<std::uint8_t, 4096> ram;
        std::array<std::uint8_t, 16> registers;
        std::array<std::uint16_t, 16> stack;
        std::bitset<16> keypad;
        ch8::frame_buffer<64, 32> screen;
        bool waiting_for_keypress;
    };

    enum class load_status { ok, file_too_big, file_does_not_exist };

    class chip8_system {
    public:
        using delta_time = std::chrono::microseconds;
        enum class observable_event { draw };

        chip8_system();

        auto load_program(const std::filesystem::path& program_file)
            -> load_status;

        template <typename Callback>
        auto observe_event(observable_event event, Callback&& observer) -> void;

        auto step() -> void;
        auto execute(delta_time dt) -> void;
        auto reset() noexcept -> void;

        // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
        chip8_data data;

        // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
        int updates_per_second;
        // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
        bool accurate_8xyE;
        // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
        bool accurate_8xy6;

    private:
        observable<const frame_buffer<64, 32>&> on_draw;
        std::chrono::microseconds time_since_update;
        std::chrono::microseconds time_since_timer_update;
        std::mt19937 rng;
    };
} // namespace ch8

template <typename Callback>
auto ch8::chip8_system::observe_event(
    observable_event event, Callback&& observer) -> void
{
    switch (event) {
    case observable_event::draw:
        on_draw.attach(observer);
        break;
    default:
        break;
    }
}

#endif // CHIP8_SYSTEM_HPP
