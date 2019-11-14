#include "chip8-sfml/widgets.hpp"
#include <fmt/format.h>
#include <limits>

auto register_widget(
    gsl::span<std::uint8_t> registers, std::uint16_t& i_register) -> void
{
    for (auto i = std::size_t{0}; i < registers.size(); i += 2) {
        auto xs = std::array<std::uint8_t, 2>{};
        xs[0] = registers.at(i);
        xs[1] = registers.at(i + 1);
        const auto label = fmt::format("{:#X}, {:#X}", i, i + 1);

        constexpr auto max = std::numeric_limits<std::uint8_t>::max();
        if (drag_scalar_n<std::uint8_t>(label, xs, 0, max, 1.F, "0x%02X")) {
            registers.at(i) = xs[0];
            registers.at(i + 1) = xs[1];
        }
    }

    constexpr auto i_max = std::numeric_limits<std::uint16_t>::max();
    drag_scalar("i", i_register, std::uint16_t{0}, i_max, 1.F, "0x%04X");
}

auto timer_widget(std::uint8_t& sound_timer, std::uint8_t& delay_timer) -> void
{
    drag_scalar("Sound Timer", sound_timer);
    drag_scalar("Delay Timer", delay_timer);
}

auto keypad_widget(std::bitset<16>& keypad) -> void
{
    const auto key_widget = [&keypad](std::array<std::size_t, 4> keys) {
        for (auto i = std::size_t{0}; i < keys.size(); ++i) {
            if (i % 4 != 0) {
                ImGui::SameLine();
            }
            auto key_pressed = keypad.test(keys.at(i));
            ImGui::Checkbox("", &key_pressed);
        }
    };

    key_widget({0x1, 0x2, 0x3, 0xC});
    key_widget({0x4, 0x5, 0x6, 0xD});
    key_widget({0x7, 0x8, 0x9, 0xE});
    key_widget({0xA, 0x0, 0xB, 0xF});
}
