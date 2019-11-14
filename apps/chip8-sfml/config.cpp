#include "chip8-sfml/config.hpp"
#include <iostream>

auto config::from_toml(const toml::value& v) -> void
{
    const auto window_table = find_or(v, "window", toml::value{});
    const auto sound_table = find_or(v, "sound", toml::value{});
    const auto interpreter_table = find_or(v, "interpreter", toml::value{});
    const auto keybinds_table = find_or(v, "keybinds", toml::value{});

    window.width = find_or(window_table, "width", 1024U);
    window.height = find_or(window_table, "height", 720U);
    window.title = find_or(window_table, "title", "Chip8");

    sound.volume = find_or(sound_table, "volume", 100U);
    sound.volume = std::clamp(sound.volume, 0U, 100U);
    sound.pitch = find_or(sound_table, "pitch", 220.0);

    interpreter.speed = find_or(interpreter_table, "speed", 800);
    if (interpreter.speed < 0) {
        interpreter.speed = 0;
    }

    const auto find_key_or =
        [&keybinds_table](const char* key, sf::Keyboard::Key default_key) {
            const auto def = static_cast<unsigned>(default_key);
            const auto found = find_or(keybinds_table, key, def);
            return static_cast<sf::Keyboard::Key>(found);
        };

    keybinds.key_0 = find_key_or("key_0", sf::Keyboard::X);
    keybinds.key_1 = find_key_or("key_1", sf::Keyboard::Num1);
    keybinds.key_2 = find_key_or("key_2", sf::Keyboard::Num2);
    keybinds.key_3 = find_key_or("key_3", sf::Keyboard::Num3);
    keybinds.key_4 = find_key_or("key_4", sf::Keyboard::Q);
    keybinds.key_5 = find_key_or("key_5", sf::Keyboard::W);
    keybinds.key_6 = find_key_or("key_6", sf::Keyboard::E);
    keybinds.key_7 = find_key_or("key_7", sf::Keyboard::A);
    keybinds.key_8 = find_key_or("key_8", sf::Keyboard::S);
    keybinds.key_9 = find_key_or("key_9", sf::Keyboard::D);
    keybinds.key_a = find_key_or("key_a", sf::Keyboard::Z);
    keybinds.key_b = find_key_or("key_b", sf::Keyboard::C);
    keybinds.key_c = find_key_or("key_c", sf::Keyboard::Num4);
    keybinds.key_d = find_key_or("key_d", sf::Keyboard::R);
    keybinds.key_e = find_key_or("key_e", sf::Keyboard::F);
    keybinds.key_f = find_key_or("key_f", sf::Keyboard::V);
}

auto config::into_toml() const -> toml::table
{
    const auto window_table = toml::table{{"width", window.width},
                                          {"height", window.height},
                                          {"title", window.title}};

    const auto sound_table =
        toml::table{{"volume", sound.volume}, {"pitch", sound.pitch}};

    const auto interpreter_table = toml::table{{"speed", interpreter.speed}};

    const auto keybinds_table =
        toml::table{{"key_0", static_cast<unsigned>(keybinds.key_0)},
                    {"key_1", static_cast<unsigned>(keybinds.key_1)},
                    {"key_2", static_cast<unsigned>(keybinds.key_2)},
                    {"key_3", static_cast<unsigned>(keybinds.key_3)},
                    {"key_4", static_cast<unsigned>(keybinds.key_4)},
                    {"key_5", static_cast<unsigned>(keybinds.key_5)},
                    {"key_6", static_cast<unsigned>(keybinds.key_6)},
                    {"key_7", static_cast<unsigned>(keybinds.key_7)},
                    {"key_8", static_cast<unsigned>(keybinds.key_8)},
                    {"key_9", static_cast<unsigned>(keybinds.key_9)},
                    {"key_a", static_cast<unsigned>(keybinds.key_a)},
                    {"key_b", static_cast<unsigned>(keybinds.key_b)},
                    {"key_c", static_cast<unsigned>(keybinds.key_c)},
                    {"key_d", static_cast<unsigned>(keybinds.key_d)},
                    {"key_e", static_cast<unsigned>(keybinds.key_e)},
                    {"key_f", static_cast<unsigned>(keybinds.key_f)}};

    return {{"window", window_table},
            {"sound", sound_table},
            {"interpreter", interpreter_table},
            {"keybinds", keybinds_table}};
}

auto load_configs(const std::filesystem::path& filename) -> config
{
    auto toml_configs = toml::value{};

    try {
        toml_configs = toml::parse(filename.string());
    }
    catch (const toml::syntax_error& e) {
        std::cerr << e.what() << "\n";
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
    }

    return toml::get<config>(toml_configs);
}

auto save_configs(const config& configs, const std::filesystem::path& filename)
    -> void
{
    auto file = std::ofstream{filename};
    file << std::setw(0) << toml::value(configs);
}
