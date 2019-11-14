#ifndef CHIP8_SFML_CONFIG_HPP
#define CHIP8_SFML_CONFIG_HPP

#include <SFML/Window/Keyboard.hpp>
#include <filesystem>
#include <string>
#include <toml11/toml.hpp>

struct config {
    struct {
        unsigned width{};
        unsigned height{};
        std::string title{};
    } window{};

    struct {
        unsigned volume{};
        double pitch{};
    } sound{};

    struct {
        int speed{};
    } interpreter{};

    struct {
        sf::Keyboard::Key key_0{};
        sf::Keyboard::Key key_1{};
        sf::Keyboard::Key key_2{};
        sf::Keyboard::Key key_3{};
        sf::Keyboard::Key key_4{};
        sf::Keyboard::Key key_5{};
        sf::Keyboard::Key key_6{};
        sf::Keyboard::Key key_7{};
        sf::Keyboard::Key key_8{};
        sf::Keyboard::Key key_9{};
        sf::Keyboard::Key key_a{};
        sf::Keyboard::Key key_b{};
        sf::Keyboard::Key key_c{};
        sf::Keyboard::Key key_d{};
        sf::Keyboard::Key key_e{};
        sf::Keyboard::Key key_f{};
    } keybinds{};

    auto from_toml(const toml::value& v) -> void;
    [[nodiscard]] auto into_toml() const -> toml::table;
};

[[nodiscard]] auto load_configs(const std::filesystem::path& filename)
    -> config;

auto save_configs(const config& configs, const std::filesystem::path& filename)
    -> void;

#endif // CHIP8_SFML_CONFIG_HPP
