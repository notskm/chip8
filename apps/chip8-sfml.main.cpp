#include "chip8-sfml/config.hpp"
#include "chip8-sfml/widgets.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <array>
#include <ch8/system.hpp>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <gsl-lite/gsl-lite.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <thread>
#include <tinyfiledialogs.h>
#include <whereami.h>

template <unsigned sample_count>
[[nodiscard]] auto
generate_sine_wave(int sample_rate, int amplitude, double frequency)
    -> std::array<sf::Int16, sample_count>
{
    auto samples = std::array<sf::Int16, sample_count>{};
    const auto increment = frequency / sample_rate;

    auto x = 0.0;

    constexpr auto tau = 6.28318;
    for (auto i = std::size_t{0}; i < samples.size(); i++) {
        auto val = sin(x * tau);
        samples.at(i) = static_cast<sf::Int16>(amplitude * val);
        x += increment;
    }

    return samples;
}

[[nodiscard]] auto executable_location() -> std::filesystem::path
{
    const auto length = wai_getExecutablePath(nullptr, 0, nullptr);
    auto str = std::string(static_cast<std::size_t>(length), '\0');
    wai_getExecutablePath(str.data(), length, nullptr);

    return std::filesystem::path{str}.remove_filename();
}

auto handle_keyboard_input(
    ch8::chip8_system& chip8, decltype(config::keybinds) keybinds) -> void
{
    chip8.data.keypad.set(0x0, sf::Keyboard::isKeyPressed(keybinds.key_0));
    chip8.data.keypad.set(0x1, sf::Keyboard::isKeyPressed(keybinds.key_1));
    chip8.data.keypad.set(0x2, sf::Keyboard::isKeyPressed(keybinds.key_2));
    chip8.data.keypad.set(0x3, sf::Keyboard::isKeyPressed(keybinds.key_3));
    chip8.data.keypad.set(0x4, sf::Keyboard::isKeyPressed(keybinds.key_4));
    chip8.data.keypad.set(0x5, sf::Keyboard::isKeyPressed(keybinds.key_5));
    chip8.data.keypad.set(0x6, sf::Keyboard::isKeyPressed(keybinds.key_6));
    chip8.data.keypad.set(0x7, sf::Keyboard::isKeyPressed(keybinds.key_7));
    chip8.data.keypad.set(0x8, sf::Keyboard::isKeyPressed(keybinds.key_8));
    chip8.data.keypad.set(0x9, sf::Keyboard::isKeyPressed(keybinds.key_9));
    chip8.data.keypad.set(0xa, sf::Keyboard::isKeyPressed(keybinds.key_a));
    chip8.data.keypad.set(0xb, sf::Keyboard::isKeyPressed(keybinds.key_b));
    chip8.data.keypad.set(0xc, sf::Keyboard::isKeyPressed(keybinds.key_c));
    chip8.data.keypad.set(0xd, sf::Keyboard::isKeyPressed(keybinds.key_d));
    chip8.data.keypad.set(0xe, sf::Keyboard::isKeyPressed(keybinds.key_e));
    chip8.data.keypad.set(0xf, sf::Keyboard::isKeyPressed(keybinds.key_f));
}

auto open_chip8_program() -> std::filesystem::path
{
    auto filters = std::array<const char*, 1>{"*.ch8"};
    const auto filters_size = static_cast<int>(filters.size());
    const auto file = tinyfd_openFileDialog(
        "Open file", "", filters_size, filters.data(), "", 0);
    return file != nullptr ? file : "";
}

// NOLINTNEXTLINE(bugprone-exception-escape)
auto main() -> int
{
    const auto configs = load_configs(executable_location() / "config.toml");
    save_configs(configs, executable_location() / "config.toml");

    auto window =
        sf::RenderWindow{{configs.window.width, configs.window.height},
                         configs.window.title,
                         sf::Style::Default};

    window.setVerticalSyncEnabled(false);

    sf::SoundBuffer sound_buffer{};
    {
        const auto samples =
            generate_sine_wave<44100>(44100, 3000, configs.sound.pitch);
        sound_buffer.loadFromSamples(samples.data(), samples.size(), 2, 44100);
    }
    sf::Sound sound{sound_buffer};
    sound.setVolume(static_cast<float>(configs.sound.volume));
    sound.setLoop(true);

    namespace chrono = std::chrono;

    auto chip8 = ch8::chip8_system{};
    chip8.updates_per_second = configs.interpreter.speed;
    auto chip8_running = false;

    auto texture = sf::Texture{};
    texture.create(
        gsl::narrow<unsigned>(chip8.data.screen.width()),
        gsl::narrow<unsigned>(chip8.data.screen.height()));

    chip8.observe_event(
        ch8::chip8_system::observable_event::draw,
        [&texture](const auto& frame_buffer) {
            texture.update(frame_buffer.data().data());
        });

    using clock = chrono::steady_clock;
    auto previous_time = chrono::steady_clock::now();

    ImGui::SFML::Init(window);

    while (window.isOpen()) {
        auto event = sf::Event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            default:
                break;
            }
        }

        const auto delta_time = chrono::duration_cast<chrono::microseconds>(
            clock::now() - previous_time);
        previous_time = clock::now();

        ImGui::SFML::Update(window, sf::microseconds(delta_time.count()));

        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File...")) {
                std::thread{[&]() {
                    const auto file = open_chip8_program();
                    if (file.empty()) {
                        chip8_running = false;
                    }
                    else {
                        chip8_running = false;
                        chip8.reset();
                        if (chip8.load_program(file) == ch8::load_status::ok) {
                            chip8_running = true;
                        }
                    }
                }}.detach();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            ImGui::DragInt("Speed", &chip8.updates_per_second, 1.F, 1, 10'000);
            ImGui::Checkbox("Accurate 8xyE", &chip8.accurate_8xyE);
            ImGui::Checkbox("Accurate 8xy6", &chip8.accurate_8xy6);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.F);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.F, 0.F});

        auto aspect_ratio = gsl::narrow<float>(chip8.data.screen.width()) /
                            gsl::narrow<float>(chip8.data.screen.height());

        const auto min_size =
            ImVec2{gsl::narrow<float>(chip8.data.screen.width()),
                   gsl::narrow<float>(chip8.data.screen.height())};
        const auto max_size = ImVec2{min_size.x * 1000.F, min_size.y * 1000.F};

        ImGui::SetNextWindowSizeConstraints(
            min_size, max_size,
            [](ImGuiSizeCallbackData* size_data) {
                const auto ratio = *static_cast<float*>(size_data->UserData);
                size_data->DesiredSize.y = size_data->DesiredSize.x / ratio;
            },
            static_cast<void*>(&aspect_ratio));

        if (ImGui::Begin("Chip8", nullptr, ImGuiWindowFlags_NoTitleBar)) {
            const auto max_content = ImGui::GetWindowContentRegionMax();
            const auto min_content = ImGui::GetWindowContentRegionMin();
            ImGui::Image(
                texture,
                {max_content.x - min_content.x, max_content.y - min_content.y});
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        if (ImGui::Begin("Registers")) {
            register_widget(chip8.data.registers, chip8.data.i_register);
        }
        ImGui::End();

        if (ImGui::Begin("Timers")) {
            timer_widget(chip8.data.sound_timer, chip8.data.delay_timer);
        }
        ImGui::End();

        if (ImGui::Begin("Keypad")) {
            keypad_widget(chip8.data.keypad);
        }
        ImGui::End();

        if (ImGui::Begin("Program")) {
            for (auto i = std::size_t{0}; i < 20; i += 2) {
                const auto ram_location = chip8.data.program_counter + i;
                if (ram_location + i >= chip8.data.ram.size()) {
                    break;
                }

                const auto byte_1 = static_cast<std::uint16_t>(
                    chip8.data.ram.at(ram_location) << 8U);
                const auto byte_2 = chip8.data.ram.at(ram_location + 1);
                const auto opcode = byte_1 | byte_2;

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                ImGui::Text(
                    "%s", fmt::format("{:#06x}: {:#06x}", ram_location, opcode)
                              .c_str());
            }
        }
        ImGui::End();

        if (window.hasFocus()) {
            handle_keyboard_input(chip8, configs.keybinds);
        }

        if (chip8_running) {
            try {
                chip8.execute(delta_time);
            }
            catch (const std::out_of_range&) {
            }

            if (chip8.data.sound_timer > 0) {
                if (sound.getStatus() != sf::Sound::Playing) {
                    sound.play();
                }
            }
            else {
                sound.pause();
            }
        }

        window.clear(sf::Color{50, 50, 50, 255});
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
