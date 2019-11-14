#ifndef CHIP8_SFML_WIDGETS_HPP
#define CHIP8_SFML_WIDGETS_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <gsl-lite/gsl-lite.hpp>
#include <imgui.h>
#include <limits>
#include <string_view>

template <typename Number>
constexpr auto imgui_data_type() -> ImGuiDataType;

template <>
constexpr auto imgui_data_type<std::uint8_t>() -> ImGuiDataType
{
    return ImGuiDataType_U8;
}

template <>
constexpr auto imgui_data_type<std::uint16_t>() -> ImGuiDataType
{
    return ImGuiDataType_U16;
}

template <typename Number>
auto drag_scalar(
    std::string_view label, Number& v,
    Number min = std::numeric_limits<Number>::min(),
    Number max = std::numeric_limits<Number>::max(), float speed = 1.F,
    std::string_view format = "") -> bool
{
    return ImGui::DragScalar(
        label.data(), imgui_data_type<Number>(), &v, speed, &min, &max,
        format.empty() ? nullptr : format.data());
}

template <typename Number>
auto drag_scalar_n(
    std::string_view label, gsl::span<Number> v,
    Number min = std::numeric_limits<Number>::min(),
    Number max = std::numeric_limits<Number>::max(), float speed = 1.F,
    std::string_view format = "") -> bool
{
    const auto fmt = format.empty() ? nullptr : format.data();
    const auto v_size = gsl::narrow<int>(v.size());

    return ImGui::DragScalarN(
        label.data(), imgui_data_type<Number>(), v.data(), v_size, speed, &min,
        &max, fmt);
}

auto register_widget(
    gsl::span<std::uint8_t> registers, std::uint16_t& i_register) -> void;
auto timer_widget(std::uint8_t& sound_timer, std::uint8_t& delay_timer) -> void;
auto keypad_widget(std::bitset<16>& keypad) -> void;

#endif // CHIP8_SFML_WIDGETS_HPP
