#include "ch8/system.hpp"
#include <algorithm>
#include <catch2/catch.hpp>
#include <fstream>
#include <random>
#include <vector>

auto operator"" _u8(unsigned long long num) -> std::uint8_t
{
    return static_cast<std::uint8_t>(num);
}

auto operator"" _i8(unsigned long long num) -> std::int8_t
{
    return static_cast<std::int8_t>(num);
}

auto operator"" _u16(unsigned long long num) -> std::uint16_t
{
    return static_cast<std::uint16_t>(num);
}

TEST_CASE("ch8::chip8_data::program_start is 0x200")
{
    STATIC_REQUIRE(ch8::chip8_data::program_start == 0x200);
}

TEST_CASE("ch8::chip8_system constructor sets program_counter to 0x200")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.program_counter == 0x200);
}

TEST_CASE("ch8::chip8_system constructor sets i_register to 0x0000")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.i_register == 0x0000);
}

TEST_CASE("ch8::chip8_system constructor sets delay_timer to 0")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.delay_timer == 0);
}

TEST_CASE("ch8::chip8_system constructor sets sound_timer to 0")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.sound_timer == 0);
}

TEST_CASE("ch8::chip8_system constructor sets stack_pointer to -1")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.stack_pointer == -1);
}

TEST_CASE("ch8::chip8_system constructor sets each register to 0")
{
    const auto system = ch8::chip8_system{};
    for (auto i : system.data.registers) {
        REQUIRE(i == 0);
    }
}

TEST_CASE("ch8::chip8_system constructor zero initializes the stack")
{
    const auto system = ch8::chip8_system{};
    for (auto i : system.data.stack) {
        REQUIRE(i == 0);
    }
}

TEST_CASE("ch8::chip8_system constructor sets each key in the keypad to false")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.keypad.none());
}

TEST_CASE("ch8::chip8_system constructor sets each pixel in screen to black")
{
    const auto system = ch8::chip8_system{};
    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            REQUIRE(system.data.screen.pixel(x, y) == ch8::color{0, 0, 0, 255});
        }
    }
}

TEST_CASE("ch8::chip8_system constructor waiting_for_keypress to false")
{
    const auto system = ch8::chip8_system{};
    REQUIRE(system.data.waiting_for_keypress == false);
}

TEST_CASE("ch8::chip8_system constructor puts the font in ram at address 0")
{
    constexpr auto font = std::array<std::uint8_t, 80>{
        0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10,
        0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
        0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
        0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
        0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
        0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};
    const auto system = ch8::chip8_system{};
    REQUIRE(std::equal(font.begin(), font.end(), system.data.ram.begin()));
}

TEST_CASE("ch8::chip8_system constructor sets each value in ram from 0x50 to 0")
{
    const auto system = ch8::chip8_system{};

    REQUIRE(std::all_of(
        system.data.ram.begin() + 0x50, system.data.ram.end(),
        [](auto i) { return i == 0; }));
}

TEST_CASE("load_program loads a program into ram starting at program start")
{
    namespace fs = std::filesystem;

    auto system = ch8::chip8_system{};
    const auto temp_dir = fs::temp_directory_path();
    const auto file =
        temp_dir / "ch8_load_program_test_loads_at_program_start.ch8";

    constexpr auto program =
        std::array<std::uint8_t, 6>{0x82, 0xFF, 0x00, 0x23, 0x75, 0x20};

    auto file_stream = std::ofstream{file, std::ios::binary};
    std::for_each(
        program.begin(), program.end(), [&](auto i) { file_stream << i; });
    file_stream.close();

    system.load_program(file);
    fs::remove(file);

    REQUIRE(std::equal(
        program.begin(), program.end(),
        system.data.ram.begin() + ch8::chip8_data::program_start));
}

TEST_CASE("load_program does not modify ram if the program is too big")
{
    namespace fs = std::filesystem;

    auto system = ch8::chip8_system{};
    const auto temp_dir = fs::temp_directory_path();
    const auto file =
        temp_dir / "ch8_load_program_test_does_not_modify_ram_if_too_big.ch8";

    auto program =
        std::array<std::uint8_t, 4096 + 1 - ch8::chip8_data::program_start>{};
    std::fill(program.begin(), program.end(), 0x30_u8);

    auto file_stream = std::ofstream{file, std::ios::binary};
    std::for_each(
        program.begin(), program.end(), [&](auto i) { file_stream << i; });
    file_stream.close();

    const auto ram_before = system.data.ram;
    system.load_program(file);
    fs::remove(file);

    REQUIRE(std::equal(
        system.data.ram.begin(), system.data.ram.end(), ram_before.begin()));
}

TEST_CASE("load_program returns ok if the program fits in ram")
{
    namespace fs = std::filesystem;

    auto system = ch8::chip8_system{};
    const auto temp_dir = fs::temp_directory_path();
    const auto file = temp_dir / "ch8_load_program_test_returns ok.ch8";

    constexpr auto program =
        std::array<std::uint8_t, 6>{0x82, 0xFF, 0x00, 0x23, 0x75, 0x20};

    auto file_stream = std::ofstream{file, std::ios::binary};
    std::for_each(
        program.begin(), program.end(), [&](auto i) { file_stream << i; });
    file_stream.close();

    const auto status = system.load_program(file);
    fs::remove(file);

    REQUIRE(status == ch8::load_status::ok);
}

TEST_CASE("load_program returns file_too_big if the program is bigger than ram")
{
    namespace fs = std::filesystem;

    auto system = ch8::chip8_system{};
    const auto temp_dir = fs::temp_directory_path();
    const auto file = temp_dir / "ch8_load_program_test_bigger_than_ram.ch8";

    auto program =
        std::array<char, 4096 + 1 - ch8::chip8_data::program_start>{};
    std::fill(program.begin(), program.end(), 0x30_u8);

    auto file_stream = std::ofstream{file, std::ios::binary};
    std::for_each(
        program.begin(), program.end(), [&](auto i) { file_stream << i; });
    file_stream.close();

    const auto status = system.load_program(file);
    fs::remove(file);

    REQUIRE(status == ch8::load_status::file_too_big);
}

TEST_CASE("load_program returns file_does_not_exist if file doesn't exist")
{
    auto system = ch8::chip8_system{};
    const auto status = system.load_program("");

    REQUIRE(status == ch8::load_status::file_does_not_exist);
}

TEST_CASE("load_program does not modify ram if file doesn't exist")
{
    auto system = ch8::chip8_system{};
    system.load_program("");

    REQUIRE(system.data.ram == ch8::chip8_data{}.ram);
}

TEST_CASE("00E0 sets each pixel in the display to black")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x00;
    system.data.ram.at(system.data.program_counter + 1U) = 0xE0;

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            system.data.screen.pixel(x, y, {59, 38, 29, 58});
        }
    }

    system.step();

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            REQUIRE(system.data.screen.pixel(x, y) == ch8::color{0, 0, 0, 255});
        }
    }
}

TEST_CASE("00E0 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x00;
    system.data.ram.at(system.data.program_counter + 1U) = 0xE0;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("00E0 notifies draw event observers")
{
    auto system = ch8::chip8_system{};

    bool observer_called = false;
    system.observe_event(
        ch8::chip8_system::observable_event::draw, [&](const auto& buffer) {
            observer_called = buffer.data() == system.data.screen.data();
        });

    system.data.ram.at(system.data.program_counter) = 0x00;
    system.data.ram.at(system.data.program_counter + 1U) = 0xE0;

    system.step();
    REQUIRE(observer_called);
}

TEST_CASE("00EE decrements the stack pointer")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x00;
    system.data.ram.at(system.data.program_counter + 1U) = 0xEE;

    const auto [sp, expected] = GENERATE(
        table<std::int8_t, std::uint8_t>({{8_i8, 7_u8}, {5_i8, 4_u8}}));

    system.data.stack_pointer = sp;

    system.step();

    REQUIRE(system.data.stack_pointer == expected);
}

TEST_CASE("00EE throws std::out_of_range if the stack pointer is less than 0")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x00;
    system.data.ram.at(system.data.program_counter + 1U) = 0xEE;

    system.data.stack_pointer = -1;

    REQUIRE_THROWS_AS(system.step(), std::out_of_range);
}

TEST_CASE(
    "00EE sets the program counter to the address at the top of the stack")
{
    const auto stack_value = GENERATE(0x047A_u16, 0x1000_u16);
    const auto sp = GENERATE(0_i8, 4_i8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x00;
    system.data.ram.at(system.data.program_counter + 1U) = 0xEE;
    system.data.stack_pointer = sp;

    const auto index = static_cast<std::size_t>(system.data.stack_pointer);
    system.data.stack.at(index) = stack_value;

    system.step();

    REQUIRE(system.data.program_counter == stack_value);
}

TEST_CASE("1nnn sets the program counter to nnn")
{
    auto system = ch8::chip8_system{};

    const auto address = GENERATE(0x0283_u16, 0x0ABC_u16);
    const auto opcode = 0x1000U | address;
    system.data.ram.at(system.data.program_counter) = (opcode & 0xFF00U) >> 8U;
    system.data.ram.at(system.data.program_counter + 1U) = opcode & 0x00FFU;

    system.step();
    REQUIRE(system.data.program_counter == address);
}

TEST_CASE("2nnn increments the stack pointer")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x2A;
    system.data.ram.at(system.data.program_counter + 1U) = 0xBC;

    const auto [sp, expected] = GENERATE(table<std::int8_t, std::uint8_t>(
        {{std::int8_t{-1}, 0_u8}, {5_i8, 6_u8}, {14_i8, 15_u8}}));
    system.data.stack_pointer = sp;

    system.step();
    REQUIRE(system.data.stack_pointer == expected);
}

TEST_CASE("2nnn puts the program counter on top of the stack")
{
    const auto pc = GENERATE(0x000_u16, 0x399, 0x030_u16);

    auto system = ch8::chip8_system{};
    system.data.program_counter = pc;
    system.data.ram.at(system.data.program_counter) = 0x28;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA2;

    system.step();

    const auto index = static_cast<std::size_t>(system.data.stack_pointer);
    const auto stack_top = system.data.stack.at(index);
    REQUIRE(stack_top == pc + 2U);
}

TEST_CASE("2nnn throws std::out_of_range when stack pointer >= stack size - 1")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x23;
    system.data.ram.at(system.data.program_counter + 1U) = 0xD9;

    system.data.stack_pointer =
        static_cast<std::int8_t>(system.data.stack.size()) - 1;

    REQUIRE_THROWS_AS(system.step(), std::out_of_range);
}

TEST_CASE("2nnn sets the program counter to nnn")
{
    const auto address = GENERATE(0x0283_u16, 0x0982_u16);
    const auto opcode = 0x2000U | address;

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = (opcode & 0xFF00U) >> 8U;
    system.data.ram.at(system.data.program_counter + 1U) = opcode & 0x00FFU;

    system.step();

    REQUIRE(system.data.program_counter == address);
}

TEST_CASE("3xkk increments the program counter by 4 when Vx == kk")
{
    const auto reg = GENERATE(0x0_u8, 0xF_u8, 0x5_u8);
    const auto value = GENERATE(0x00_u8, 0x3F_u8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x30_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = value;
    system.data.registers.at(reg) = value;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 4);
}

TEST_CASE("3xkk increments the program counter by 2 when Vx != kk")
{
    const auto reg = GENERATE(0x0_u8, 0xF_u8, 0x5_u8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x30U | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x50;
    system.data.registers.at(reg) = 70;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("4xkk increments the program counter by 4 when Vx != kk")
{
    const auto reg = GENERATE(0x0_u8, 0xF_u8, 0x2_u8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x40U | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x50;
    system.data.registers.at(reg) = 70;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 4);
}

TEST_CASE("4xkk increments the program counter by 2 when Vx == kk")
{
    const auto reg = GENERATE(0x0_u8, 0xF_u8, 0x2_u8);
    const auto value = GENERATE(0x00_u8, 0x3F_u8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x40_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = value;
    system.data.registers.at(reg) = value;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("5xy0 increments the program counter by 4 when Vx == Vy")
{
    const auto [reg_x, reg_y, value] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>({
            {0x0_u8, 0x5_u8, 59_u8},
            {0xA_u8, 0x2_u8, 74_u8},
            {0xF_u8, 0x8_u8, 28_u8},
        }));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x50_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        static_cast<std::uint8_t>(reg_y << 4_u8);

    system.data.registers.at(reg_x) = value;
    system.data.registers.at(reg_y) = value;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 4);
}

TEST_CASE("5xy0 increments the program counter by 2 when Vx != Vy")
{
    const auto [reg_x, reg_y, value] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x0_u8, 0x5_u8, 85_u8},
             {0xF_u8, 0x3_u8, 17_u8},
             {0x7_u8, 0xD_u8, 255_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x50_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        static_cast<std::uint8_t>(reg_y << 4U);
    system.data.registers.at(reg_x) = value;
    system.data.registers.at(reg_y) = 0;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "5xy0 increments the program counter by 4 when registers are the same")
{
    const auto reg = GENERATE(0x0_u8, 0x8_u8, 0xF_u8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x50_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) =
        static_cast<std::uint8_t>(reg << 4U);

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 4U);
}

TEST_CASE("6xkk stores kk in Vx")
{
    auto system = ch8::chip8_system{};
    const auto value = GENERATE(0x75_u8, 0x83_u8);
    const auto reg = GENERATE(0x3_u8, 0xF_u8, 0xC_u8);
    system.data.ram.at(system.data.program_counter) = 0x60_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = value;

    system.step();

    REQUIRE(system.data.registers.at(reg) == value);
}

TEST_CASE("6xkk increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x6A;
    system.data.ram.at(system.data.program_counter + 1U) = 0x7E;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("7xkk stores the result of Vx + kk in Vx")
{
    const auto [reg, reg_init_value, add_value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x08_u8, 0x20_u8, 0x32_u8, 0x52_u8},
             {0x0D_u8, 0x0F_u8, 0x28_u8, 0x37_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x70U | reg;
    system.data.ram.at(system.data.program_counter + 1U) = add_value;
    system.data.registers.at(reg) = reg_init_value;

    system.step();

    REQUIRE(system.data.registers.at(reg) == expected);
}

TEST_CASE("7xkk wraps around when Vx + kk > 255")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x73;
    system.data.ram.at(system.data.program_counter + 1U) = 0x05;
    system.data.registers.at(0x3) = 0xFF;

    system.step();
    REQUIRE(system.data.registers.at(0x3) == 4);
}

TEST_CASE("7xkk increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x7A;
    system.data.ram.at(system.data.program_counter + 1U) = 0xE6;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy0 stores the value of Vy in Vx")
{
    auto system = ch8::chip8_system{};

    const auto reg_x = GENERATE(0x4_u8, 0xF_u8);
    const auto reg_y = GENERATE(0x5_u8, 0x2_u8);
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        static_cast<std::uint8_t>(reg_y << 4_u8);

    const auto value = GENERATE(58_u8, 255_u8, 0_u8);
    system.data.registers.at(reg_y) = value;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == value);
}

TEST_CASE("8xy0 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8A;
    system.data.ram.at(system.data.program_counter + 1U) = 0xB0;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy1 stores the result of Vx | Vy in Vx")
{
    const auto [reg_x, reg_y, x_val, y_val, expected] =
        GENERATE(table<
                 std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t,
                 std::uint8_t>(
            {{0x8_u8, 0x4_u8, 0b10010101_u8, 0b10110100_u8, 0b10110101_u8},
             {0x7_u8, 0xE_u8, 0b11011010_u8, 0b10001001_u8, 0b11011011_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x01_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_val;
    system.data.registers.at(reg_y) = y_val;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy1 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8E;
    system.data.ram.at(system.data.program_counter + 1U) = 0xF1;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy2 stores the result of Vx & Vy in Vx")
{
    const auto [reg_x, reg_y, x_val, y_val, expected] =
        GENERATE(table<
                 std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t,
                 std::uint8_t>(
            {{0xA_u8, 0xD_u8, 0b10010101_u8, 0b10110100_u8, 0b10010100_u8},
             {0xB_u8, 0x7_u8, 0b11011010_u8, 0b10001001_u8, 0b10001000_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x02_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_val;
    system.data.registers.at(reg_y) = y_val;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy2 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80;
    system.data.ram.at(system.data.program_counter + 1U) = 0x72;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy3 stores the result of Vx ^ Vy in Vx")
{
    const auto [reg_x, reg_y, x_val, y_val, expected] =
        GENERATE(table<
                 std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t,
                 std::uint8_t>(
            {{0x4_u8, 0x3_u8, 0b10010101_u8, 0b10110100_u8, 0b00100001_u8},
             {0x8_u8, 0xD_u8, 0b11011010_u8, 0b10001001_u8, 0b01010011_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x03_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_val;
    system.data.registers.at(reg_y) = y_val;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy3 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x88;
    system.data.ram.at(system.data.program_counter + 1U) = 0x23;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy4 stores the result of Vx + Vy in Vx")
{
    const auto [reg_x, reg_y, x_val, y_val, expected] =
        GENERATE(table<
                 std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t,
                 std::uint8_t>({{0x4_u8, 0x3_u8, 24_u8, 87_u8, 111_u8},
                                {0x8_u8, 0xD_u8, 75_u8, 14_u8, 89_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x04_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_val;
    system.data.registers.at(reg_y) = y_val;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy4 addition rolls over to 0 when Vx + Vy > 255")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x83;
    system.data.ram.at(system.data.program_counter + 1U) = 0x74;
    system.data.registers.at(0x3) = 255;
    system.data.registers.at(0x7) = 5;

    system.step();
    REQUIRE(system.data.registers.at(0x3) == 4);
}

TEST_CASE("8xy4 sets VF to 1 when Vx + Vy > 255")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x84;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA4;
    system.data.registers.at(0x4) = 255;
    system.data.registers.at(0xA) = 85;
    system.data.registers.at(0xF) = 0;

    system.step();
    REQUIRE(system.data.registers.at(0xF) == 1);
}

TEST_CASE("8xy4 sets VF to 0 when Vx + Vy <= 255")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8D;
    system.data.ram.at(system.data.program_counter + 1U) = 0x54;
    system.data.registers.at(0xD) = 30;
    system.data.registers.at(0x5) = 20;
    system.data.registers.at(0xF) = 1;

    system.step();
    REQUIRE(system.data.registers.at(0xF) == 0);
}

TEST_CASE("8xy4 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x88;
    system.data.ram.at(system.data.program_counter + 1U) = 0x24;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy5 stores the result of Vx - Vy in Vx")
{
    const auto [reg_x, reg_y, x_val, y_val, expected] =
        GENERATE(table<
                 std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t,
                 std::uint8_t>({{0x8_u8, 0xE_u8, 73_u8, 28_u8, 45_u8},
                                {0xA_u8, 0xD_u8, 93_u8, 82_u8, 11_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x05_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_val;
    system.data.registers.at(reg_y) = y_val;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy5 sets VF to 1 if Vx > Vy")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x82;
    system.data.ram.at(system.data.program_counter + 1U) = 0x05;
    system.data.registers.at(0x2) = 20;
    system.data.registers.at(0x0) = 10;
    system.data.registers.at(0xF) = 0;

    system.step();
    REQUIRE(system.data.registers.at(0xF) == 1);
}

TEST_CASE("8xy5 sets VF to 0 if Vx < Vy")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x82;
    system.data.ram.at(system.data.program_counter + 1U) = 0x05;
    system.data.registers.at(0x2) = 10;
    system.data.registers.at(0x0) = 20;
    system.data.registers.at(0xF) = 1;

    system.step();
    REQUIRE(system.data.registers.at(0xF) == 0);
}

TEST_CASE("8xy5 subtraction wraps to 255 when Vx < Vy")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8C;
    system.data.ram.at(system.data.program_counter + 1U) = 0x85;
    system.data.registers.at(0xC) = 1;
    system.data.registers.at(0x8) = 5;

    system.step();
    REQUIRE(system.data.registers.at(0xC) == 252);
}

TEST_CASE("8xy5 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x82;
    system.data.ram.at(system.data.program_counter + 1U) = 0x05;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy6 stores the result of Vy >> 1 in Vx if 8xy6 accuracy is enabled")
{
    const auto [reg_x, reg_y, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x8_u8, 0xA_u8, 0b10010101_u8, 0b01001010_u8},
             {0x4_u8, 0x2_u8, 0b01001010_u8, 0b00100101_u8}}));

    auto system = ch8::chip8_system{};
    system.accurate_8xy6 = true;

    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x06_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_y) = value;

    system.step();

    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE(
    "8xy6 stores the result of Vy >> 1 in Vx if 8xy6 accuracy is disabled")
{
    const auto [reg_x, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x8_u8, 0b10010101_u8, 0b01001010_u8},
             {0x4_u8, 0b01001010_u8, 0b00100101_u8}}));

    auto system = ch8::chip8_system{};
    system.accurate_8xy6 = false;

    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) = 0xF6_u8;
    system.data.registers.at(reg_x) = value;

    system.step();

    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy6 stores the least significant bit of Vy in VF")
{
    auto [value, expected] = GENERATE(
        table<std::uint8_t, int>({{0b10001010_u8, 0}, {0b011101011_u8, 1}}));
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8E;
    system.data.ram.at(system.data.program_counter + 1U) = 0xD6;
    system.data.registers.at(0xD) = value;
    system.data.registers.at(0xF) = 53;

    system.step();

    REQUIRE(system.data.registers.at(0xF) == expected);
}

TEST_CASE("8xy6 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8C;
    system.data.ram.at(system.data.program_counter + 1U) = 0x26;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xy7 stores the result of Vy - Vx in Vx")
{
    const auto [reg_x, reg_y, x_val, y_val, expected] =
        GENERATE(table<
                 std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t,
                 std::uint8_t>({{0x8_u8, 0xE_u8, 28_u8, 73_u8, 45_u8},
                                {0xA_u8, 0xD_u8, 82_u8, 93_u8, 11_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x07_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_val;
    system.data.registers.at(reg_y) = y_val;

    system.step();
    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xy7 sets VF to 1 if Vy > Vx")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80;
    system.data.ram.at(system.data.program_counter + 1U) = 0x27;
    system.data.registers.at(0x2) = 20;
    system.data.registers.at(0x0) = 10;
    system.data.registers.at(0xF) = 0;

    system.step();
    REQUIRE(system.data.registers.at(0xF) == 1);
}

TEST_CASE("8xy7 sets VF to 0 if Vy < Vx")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x80;
    system.data.ram.at(system.data.program_counter + 1U) = 0x27;
    system.data.registers.at(0x2) = 10;
    system.data.registers.at(0x0) = 20;
    system.data.registers.at(0xF) = 1;

    system.step();
    REQUIRE(system.data.registers.at(0xF) == 0);
}

TEST_CASE("8xy7 subtraction wraps to 255 when Vy < Vx")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8C;
    system.data.ram.at(system.data.program_counter + 1U) = 0x87;
    system.data.registers.at(0xC) = 5;
    system.data.registers.at(0x8) = 1;

    system.step();
    REQUIRE(system.data.registers.at(0xC) == 252);
}

TEST_CASE("8xy7 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x82;
    system.data.ram.at(system.data.program_counter + 1U) = 0x07;

    const auto pc = system.data.program_counter;

    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("8xyE stores the result of Vy << 1 in Vx if 8xyE accuracy is enabled")
{
    const auto [reg_x, reg_y, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x8_u8, 0xA_u8, 0b10010101_u8, 0b00101010_u8},
             {0x4_u8, 0x2_u8, 0b01001010_u8, 0b10010100_u8}}));

    auto system = ch8::chip8_system{};
    system.accurate_8xyE = true;

    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x0E_u8 | static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_y) = value;

    system.step();

    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE(
    "8xyE stores the result of Vx << 1 in Vx if 8xyE accuracy is disabled")
{
    const auto [reg_x, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x8_u8, 0b10010101_u8, 0b00101010_u8},
             {0x4_u8, 0b01001010_u8, 0b10010100_u8}}));

    auto system = ch8::chip8_system{};
    system.accurate_8xyE = false;

    system.data.ram.at(system.data.program_counter) = 0x80_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) = 0xFE_u8;
    system.data.registers.at(reg_x) = value;

    system.step();

    REQUIRE(system.data.registers.at(reg_x) == expected);
}

TEST_CASE("8xyE stores the most significant bit of Vy in VF")
{
    auto [value, expected] = GENERATE(
        table<std::uint8_t, int>({{0b10001010_u8, 1}, {0b01110101_u8, 0}}));
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8E;
    system.data.ram.at(system.data.program_counter + 1U) = 0xDE;
    system.data.registers.at(0xD) = value;
    system.data.registers.at(0xF) = 53;

    system.step();

    REQUIRE(system.data.registers.at(0xF) == expected);
}

TEST_CASE("8xyE increments the program counter by 2")
{
    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x8C;
    system.data.ram.at(system.data.program_counter + 1U) = 0x2E;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("9xy0 increments the program counter by 4 when Vx != Vy")
{
    const auto [reg_x, reg_y, x_value, y_value] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x2_u8, 0x7_u8, 37_u8, 28_u8}, {0x9_u8, 0xA_u8, 83_u8, 72_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x90_u8 | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = x_value;
    system.data.registers.at(reg_y) = y_value;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 4);
}

TEST_CASE("9xy0 increments the program counter by 2 when Vx == Vy")
{
    const auto [reg_x, reg_y, value] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x2_u8, 0x7_u8, 37_u8}, {0x9_u8, 0xA_u8, 83_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0x90U | reg_x;
    system.data.ram.at(system.data.program_counter + 1U) =
        static_cast<std::uint8_t>(reg_y << 4_u8);
    system.data.registers.at(reg_x) = value;
    system.data.registers.at(reg_y) = value;

    const auto pc = system.data.program_counter;

    system.step();

    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Annn stores nnn in the i register")
{
    const auto address = GENERATE(0xBD7_u16, 0xB92_u16);

    auto system = ch8::chip8_system{};

    const auto addr_b1_masked = static_cast<uint16_t>(address & 0x0F00_u16);
    const auto addr_b1 = static_cast<std::uint8_t>(addr_b1_masked >> 8_u8);

    const auto byte_1 = static_cast<std::uint8_t>(0xA0_u8 | addr_b1);
    const auto byte_2 = static_cast<std::uint8_t>(address & 0x00FF_u16);

    system.data.ram.at(system.data.program_counter) = byte_1;
    system.data.ram.at(system.data.program_counter + 1U) = byte_2;

    system.step();
    REQUIRE(system.data.i_register == address);
}

TEST_CASE("Annn increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xAB;
    system.data.ram.at(system.data.program_counter + 1U) = 0x52;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Bnnn sets the program counter to nnn + V0")
{
    auto system = ch8::chip8_system{};

    const auto jump = GENERATE(20_u8, 84_u8);
    const auto address = GENERATE(0xBD7_u16, 0xB92_u16);

    const auto addr_b1_masked = static_cast<uint16_t>(address & 0x0F00_u16);
    const auto addr_b1 = static_cast<std::uint8_t>(addr_b1_masked >> 8_u8);

    const auto byte_1 = static_cast<std::uint8_t>(0xB0_u8 | addr_b1);
    const auto byte_2 = static_cast<std::uint8_t>(address & 0x00FF_u16);

    system.data.ram.at(system.data.program_counter) = byte_1;
    system.data.ram.at(system.data.program_counter + 1U) = byte_2;

    system.data.registers.at(0x0) = jump;

    system.step();
    REQUIRE(system.data.program_counter == address + jump);
}

TEST_CASE("Cxkk increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xCA;
    system.data.ram.at(system.data.program_counter + 1U) = 0x30;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Dxyn draws a sprite pointed to by the i register to the screen")
{
    auto sprite = GENERATE(
        std::vector<std::uint8_t>{
            0b01100110, // .xx..xx.
            0b01100110, // .xx..xx.
            0b00010000, // ...x....
            0b10001001, // x...x..x
            0b01111110  // .xxxxxx.
        },
        std::vector<std::uint8_t>{
            0b00100100, // ..x..x..
            0b00111100, // ..xxxx..
            0b00100100, // ..x..x..
            0b00111100, // ..xxxx..
            0b00100100, // ..x..x..
            0b00111100, // ..xxxx..
            0b00100100  // ..x..x..
        });

    auto system = ch8::chip8_system{};
    system.data.i_register = system.data.program_counter + 10;
    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.ram.at(system.data.program_counter) = 0xD0_u8;
    system.data.ram.at(system.data.program_counter + 1U) =
        0x00_u8 | static_cast<std::uint8_t>(sprite.size());

    system.step();

    auto expected_screen = ch8::chip8_data{}.screen;

    for (auto y = std::size_t{0}; y < sprite.size(); ++y) {
        const auto sprite_byte = sprite.at(y);

        const auto sprite_bit_to_color =
            [&sprite_byte](const std::uint8_t mask) {
                constexpr auto white = ch8::color{255, 255, 255, 255};
                constexpr auto black = ch8::color{0, 0, 0, 255};
                return (sprite_byte & mask) > 0 ? white : black;
            };

        expected_screen.pixel(0, y, sprite_bit_to_color(0b10000000));
        expected_screen.pixel(1, y, sprite_bit_to_color(0b01000000));
        expected_screen.pixel(2, y, sprite_bit_to_color(0b00100000));
        expected_screen.pixel(3, y, sprite_bit_to_color(0b00010000));
        expected_screen.pixel(4, y, sprite_bit_to_color(0b00001000));
        expected_screen.pixel(5, y, sprite_bit_to_color(0b00000100));
        expected_screen.pixel(6, y, sprite_bit_to_color(0b00000010));
        expected_screen.pixel(7, y, sprite_bit_to_color(0b00000001));
    }

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            const auto pixel = system.data.screen.pixel(x, y);
            const auto expected_pixel = expected_screen.pixel(x, y);
            REQUIRE(pixel == expected_pixel);
        }
    }
}

TEST_CASE("Dxyn draws sprites to the position stored in Vx, Vy on the screen")
{
    auto sprite = std::array<std::uint8_t, 5>{
        0b01100110, // .xx..xx.
        0b01100110, // .xx..xx.
        0b00010000, // ...x....
        0b10001001, // x...x..x
        0b01111110  // .xxxxxx.
    };

    auto system = ch8::chip8_system{};
    system.data.i_register = system.data.program_counter + 17;
    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.ram.at(system.data.program_counter) = 0xD4;
    system.data.ram.at(system.data.program_counter + 1U) = 0x75U;
    system.data.registers.at(0x4) = 8;
    system.data.registers.at(0x7) = 2;

    system.step();

    auto expected_screen = ch8::chip8_data{}.screen;
    for (auto y = std::size_t{0}; y < sprite.size(); ++y) {
        const auto sprite_byte = sprite.at(y);

        const auto sprite_bit_to_color =
            [&sprite_byte](const std::uint8_t mask) {
                constexpr auto white = ch8::color{255, 255, 255, 255};
                constexpr auto black = ch8::color{0, 0, 0, 255};
                return (sprite_byte & mask) > 0 ? white : black;
            };

        expected_screen.pixel(0 + 8, y + 2, sprite_bit_to_color(0b10000000));
        expected_screen.pixel(1 + 8, y + 2, sprite_bit_to_color(0b01000000));
        expected_screen.pixel(2 + 8, y + 2, sprite_bit_to_color(0b00100000));
        expected_screen.pixel(3 + 8, y + 2, sprite_bit_to_color(0b00010000));
        expected_screen.pixel(4 + 8, y + 2, sprite_bit_to_color(0b00001000));
        expected_screen.pixel(5 + 8, y + 2, sprite_bit_to_color(0b00000100));
        expected_screen.pixel(6 + 8, y + 2, sprite_bit_to_color(0b00000010));
        expected_screen.pixel(7 + 8, y + 2, sprite_bit_to_color(0b00000001));
    }

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            const auto pixel = system.data.screen.pixel(x, y);
            const auto expected_pixel = expected_screen.pixel(x, y);
            REQUIRE(pixel == expected_pixel);
        }
    }
}

TEST_CASE("Dxyn xors sprites onto the screen")
{
    auto sprite = std::array<std::uint8_t, 5>{
        0b01100110, // .xx..xx.
        0b01100110, // .xx..xx.
        0b00010000, // ...x....
        0b10001001, // x...x..x
        0b01111110  // .xxxxxx.
    };
    auto inverted_sprite = std::array<std::uint8_t, 5>{
        0b10011001, // x..xx..x
        0b10011001, // x..xx..x
        0b11101111, // xxx.xxxx
        0b01110110, // .xxx.xx.
        0b10000001  // x......x
    };

    auto system = ch8::chip8_system{};
    system.data.i_register = system.data.program_counter + 33;
    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.screen.data().fill(255_u8);

    system.data.ram.at(system.data.program_counter) = 0xD9;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA5U;
    system.data.registers.at(0x9) = 4;
    system.data.registers.at(0xA) = 7;

    system.step();

    auto expected_screen = ch8::chip8_data{}.screen;
    expected_screen.data().fill(255_u8);

    for (auto y = std::size_t{0}; y < inverted_sprite.size(); ++y) {
        const auto sprite_byte = inverted_sprite.at(y);

        const auto sprite_bit_to_color =
            [&sprite_byte](const std::uint8_t mask) {
                constexpr auto white = ch8::color{255, 255, 255, 255};
                constexpr auto black = ch8::color{0, 0, 0, 255};
                return (sprite_byte & mask) > 0 ? white : black;
            };

        expected_screen.pixel(0 + 4, y + 7, sprite_bit_to_color(0b10000000));
        expected_screen.pixel(1 + 4, y + 7, sprite_bit_to_color(0b01000000));
        expected_screen.pixel(2 + 4, y + 7, sprite_bit_to_color(0b00100000));
        expected_screen.pixel(3 + 4, y + 7, sprite_bit_to_color(0b00010000));
        expected_screen.pixel(4 + 4, y + 7, sprite_bit_to_color(0b00001000));
        expected_screen.pixel(5 + 4, y + 7, sprite_bit_to_color(0b00000100));
        expected_screen.pixel(6 + 4, y + 7, sprite_bit_to_color(0b00000010));
        expected_screen.pixel(7 + 4, y + 7, sprite_bit_to_color(0b00000001));
    }

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            const auto pixel = system.data.screen.pixel(x, y);
            const auto expected_pixel = expected_screen.pixel(x, y);
            REQUIRE(pixel == expected_pixel);
        }
    }
}

TEST_CASE("Dxyn wraps sprites to the left side of the screen")
{
    auto sprite = std::array<std::uint8_t, 5>{
        0b01100110, // .xx..xx.
        0b01100110, // .xx..xx.
        0b00010000, // ...x....
        0b10001001, // x...x..x
        0b01111110  // .xxxxxx.
    };

    auto system = ch8::chip8_system{};
    system.data.i_register = system.data.program_counter + 33;
    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.ram.at(system.data.program_counter) = 0xD3;
    system.data.ram.at(system.data.program_counter + 1U) = 0xE5U;
    system.data.registers.at(0x3) = 60;
    system.data.registers.at(0xE) = 4;

    system.step();

    auto expected_screen = ch8::chip8_data{}.screen;
    for (auto y = std::size_t{0}; y < sprite.size(); ++y) {
        const auto sprite_byte = sprite.at(y);

        const auto sprite_bit_to_color =
            [&sprite_byte](const std::uint8_t mask) {
                constexpr auto white = ch8::color{255, 255, 255, 255};
                constexpr auto black = ch8::color{0, 0, 0, 255};
                return (sprite_byte & mask) > 0 ? white : black;
            };

        const auto x_pos = [&expected_screen](auto x) {
            return static_cast<std::size_t>(x) % expected_screen.width();
        };

        expected_screen.pixel(
            x_pos(0 + 60), y + 4, sprite_bit_to_color(0b10000000));
        expected_screen.pixel(
            x_pos(1 + 60), y + 4, sprite_bit_to_color(0b01000000));
        expected_screen.pixel(
            x_pos(2 + 60), y + 4, sprite_bit_to_color(0b00100000));
        expected_screen.pixel(
            x_pos(3 + 60), y + 4, sprite_bit_to_color(0b00010000));
        expected_screen.pixel(
            x_pos(4 + 60), y + 4, sprite_bit_to_color(0b00001000));
        expected_screen.pixel(
            x_pos(5 + 60), y + 4, sprite_bit_to_color(0b00000100));
        expected_screen.pixel(
            x_pos(6 + 60), y + 4, sprite_bit_to_color(0b00000010));
        expected_screen.pixel(
            x_pos(7 + 60), y + 4, sprite_bit_to_color(0b00000001));
    }

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            const auto pixel = system.data.screen.pixel(x, y);
            const auto expected_pixel = expected_screen.pixel(x, y);
            REQUIRE(pixel == expected_pixel);
        }
    }
}

TEST_CASE("Dxyn wraps sprites to the top of the screen")
{
    auto sprite = std::array<std::uint8_t, 5>{
        0b01100110, // .xx..xx.
        0b01100110, // .xx..xx.
        0b00010000, // ...x....
        0b10001001, // x...x..x
        0b01111110  // .xxxxxx.
    };

    auto system = ch8::chip8_system{};
    system.data.i_register = system.data.program_counter + 33;
    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.ram.at(system.data.program_counter) = 0xD3;
    system.data.ram.at(system.data.program_counter + 1U) = 0xE5U;
    system.data.registers.at(0x3) = 9;
    system.data.registers.at(0xE) = 30;

    system.step();

    auto expected_screen = ch8::chip8_data{}.screen;
    for (auto y = std::size_t{0}; y < sprite.size(); ++y) {
        const auto sprite_byte = sprite.at(y);

        const auto sprite_bit_to_color =
            [&sprite_byte](const std::uint8_t mask) {
                constexpr auto white = ch8::color{255, 255, 255, 255};
                constexpr auto black = ch8::color{0, 0, 0, 255};
                return (sprite_byte & mask) > 0 ? white : black;
            };

        const auto y_pos = (y + 30) % expected_screen.height();
        expected_screen.pixel(9, y_pos, sprite_bit_to_color(0b10000000));
        expected_screen.pixel(10, y_pos, sprite_bit_to_color(0b01000000));
        expected_screen.pixel(11, y_pos, sprite_bit_to_color(0b00100000));
        expected_screen.pixel(12, y_pos, sprite_bit_to_color(0b00010000));
        expected_screen.pixel(13, y_pos, sprite_bit_to_color(0b00001000));
        expected_screen.pixel(14, y_pos, sprite_bit_to_color(0b00000100));
        expected_screen.pixel(15, y_pos, sprite_bit_to_color(0b00000010));
        expected_screen.pixel(16, y_pos, sprite_bit_to_color(0b00000001));
    }

    for (auto x = std::size_t{0}; x < system.data.screen.width(); ++x) {
        for (auto y = std::size_t{0}; y < system.data.screen.height(); ++y) {
            const auto pixel = system.data.screen.pixel(x, y);
            const auto expected_pixel = expected_screen.pixel(x, y);
            REQUIRE(pixel == expected_pixel);
        }
    }
}

TEST_CASE("Dxyn sets VF to 1 when pixels are erased")
{
    auto sprite = std::array<std::uint8_t, 5>{
        0b01100110, // .xx..xx.
        0b01100110, // .xx..xx.
        0b00010000, // ...x....
        0b10001001, // x...x..x
        0b01111110  // .xxxxxx.
    };

    auto system = ch8::chip8_system{};
    system.data.registers.at(0xF) = 0;
    system.data.i_register = system.data.program_counter + 33;

    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.screen.data().fill(255_u8);

    system.data.ram.at(system.data.program_counter) = 0xD9;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA5U;
    system.data.registers.at(0x9) = 4;
    system.data.registers.at(0xA) = 7;

    system.step();

    REQUIRE(system.data.registers.at(0xF) == 1);
}

TEST_CASE("Dxyn sets VF to 1 when pixels are not erased")
{
    auto sprite = std::array<std::uint8_t, 5>{
        0b01100110, // .xx..xx.
        0b01100110, // .xx..xx.
        0b00010000, // ...x....
        0b10001001, // x...x..x
        0b01111110  // .xxxxxx.
    };

    auto system = ch8::chip8_system{};
    system.data.registers.at(0xF) = 1;
    system.data.i_register = system.data.program_counter + 33;

    std::copy(
        sprite.begin(), sprite.end(),
        system.data.ram.begin() + system.data.i_register);

    system.data.ram.at(system.data.program_counter) = 0xD9;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA5U;
    system.data.registers.at(0x9) = 4;
    system.data.registers.at(0xA) = 7;

    system.step();

    REQUIRE(system.data.registers.at(0xF) == 0);
}

TEST_CASE("Dxyn increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xD0;
    system.data.ram.at(system.data.program_counter + 1U) = 0x00;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Dxyn notifies draw event observers")
{
    auto system = ch8::chip8_system{};

    bool observer_called = false;
    system.observe_event(
        ch8::chip8_system::observable_event::draw, [&](const auto& buffer) {
            observer_called = buffer.data() == system.data.screen.data();
        });

    system.data.ram.at(system.data.program_counter) = 0xD0;
    system.data.ram.at(system.data.program_counter + 1U) = 0x00;

    system.step();
    REQUIRE(observer_called);
}

TEST_CASE(
    "Ex9E increments the program counter by 4 if key stored in Vx is pressed")
{
    const auto reg = GENERATE(0x7_u8, 0xA_u8, 0x3_u8);
    const auto key = GENERATE(0x8_u8, 0xE_u8, 0x2_u8);

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xE0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x9E;
    system.data.registers.at(reg) = key;
    system.data.keypad[key] = true;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 4);
}

TEST_CASE(
    "Ex9E increments the program counter by 2 if key stored in Vx is not pressed")
{
    const auto reg = GENERATE(0x7_u8, 0xA_u8, 0x3_u8);
    const auto key = GENERATE(0x8_u8, 0xE_u8, 0x2_u8);
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xE0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x9E;
    system.data.registers.at(reg) = key;
    system.data.keypad[key] = false;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "ExA1 increments the program counter by 2 if key stored in Vx is pressed")
{
    const auto reg = GENERATE(0x7_u8, 0xA_u8, 0x3_u8);
    const auto key = GENERATE(0x8_u8, 0xE_u8, 0x2_u8);

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xE0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA1;
    system.data.registers.at(reg) = key;
    system.data.keypad[key] = true;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "ExA1 increments the program counter by 4 if key stored in Vx is not pressed")
{
    const auto reg = GENERATE(0x7_u8, 0xA_u8, 0x3_u8);
    const auto key = GENERATE(0x8_u8, 0xE_u8, 0x2_u8);
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xE0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0xA1;
    system.data.registers.at(reg) = key;
    system.data.keypad[key] = false;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 4);
}

TEST_CASE("Fx07 stores the value of the delay timer in Vx")
{
    const auto value = GENERATE(0x50_u8, 0x37_u8);
    const auto reg = GENERATE(0x8_u8, 0xD_u8);

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x07;
    system.data.delay_timer = value;

    system.step();
    REQUIRE(system.data.registers.at(reg) == value);
}

TEST_CASE("Fx07 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF1;
    system.data.ram.at(system.data.program_counter + 1U) = 0x07;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "Fx0A does not increment the program counter when not currently waiting")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF5;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc);
}

TEST_CASE("Fx0A sets register x to the first key pressed")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF8;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    system.step();
    system.data.keypad.set(0xC);
    system.step();

    REQUIRE(system.data.registers.at(0x8) == 0xC);
}

TEST_CASE("Fx0A only the first keypress is stored in the x register")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF8;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    system.step();
    system.data.keypad.set(0xA);
    system.step();
    system.data.keypad.set(0xB);
    system.step();
    system.data.keypad.set(0xD);
    system.step();
    system.data.keypad.set(0x1);
    system.step();

    REQUIRE(system.data.registers.at(0x8) == 0xA);
}

TEST_CASE("Fx0A does not increment the program counter when key is pressed")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF3;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    const auto pc = system.data.program_counter;

    system.step();
    system.data.keypad.set(0x3);
    system.step();

    REQUIRE(system.data.program_counter == pc);
}

TEST_CASE(
    "Fx0A does not increment the program counter when key is still pressed")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF3;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    const auto pc = system.data.program_counter;

    system.step();
    system.data.keypad.set(0x3);

    system.step();
    REQUIRE(system.data.program_counter == pc);
    system.step();
    REQUIRE(system.data.program_counter == pc);
    system.step();
    REQUIRE(system.data.program_counter == pc);
}

TEST_CASE(
    "Fx0A increments the program counter by 2 when pressed key is released")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xFE;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    const auto pc = system.data.program_counter;

    system.step();
    system.data.keypad.set(0xE);
    system.step();
    system.data.keypad.reset(0xE);
    system.step();

    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "Fx0A does not increment the program counter if other keys are released")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xFE;
    system.data.ram.at(system.data.program_counter + 1U) = 0x0A;

    const auto pc = system.data.program_counter;

    system.step();
    system.data.keypad.set(0xE);
    system.step();
    system.data.keypad.set(0xF);
    system.step();
    system.data.keypad.reset(0xF);
    system.step();

    REQUIRE(system.data.program_counter == pc);
}

TEST_CASE("Fx15 sets the delay timer to the value of Vx")
{
    const auto value = GENERATE(0x28_u8, 0x84_u8);
    const auto reg = GENERATE(0xE_u8, 0x0_u8);

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x15;
    system.data.registers.at(reg) = value;

    system.step();
    REQUIRE(system.data.delay_timer == value);
}

TEST_CASE("Fx15 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF1;
    system.data.ram.at(system.data.program_counter + 1U) = 0x15;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Fx18 sets the delay timer to the value of Vx")
{
    const auto value = GENERATE(0x28_u8, 0x84_u8);
    const auto reg = GENERATE(0xE_u8, 0x0_u8);

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x18;
    system.data.registers.at(reg) = value;

    system.step();
    REQUIRE(system.data.sound_timer == value);
}

TEST_CASE("Fx18 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF1;
    system.data.ram.at(system.data.program_counter + 1U) = 0x18;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Fx1E stores the result of Vx + i register in the i register")
{
    const auto [reg, x_val, i_val, expected] = GENERATE(
        table<std::uint8_t, std::uint8_t, std::uint16_t, std::uint16_t>(
            {{0x1_u8, 83_u8, 29_u16, 112_u16},
             {0x7_u8, 93_u8, 255_u16, 348_u16}}));

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x1E;
    system.data.registers.at(reg) = x_val;
    system.data.i_register = i_val;

    system.step();
    REQUIRE(system.data.i_register == expected);
}

TEST_CASE("Fx1E increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF4;
    system.data.ram.at(system.data.program_counter + 1U) = 0x1E;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Fx29 sets the i register to 0x0 when Vx is 0")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF9;
    system.data.ram.at(system.data.program_counter + 1U) = 0x29;
    system.data.registers.at(0x9) = 0;
    system.data.i_register = 0x83;

    system.step();

    REQUIRE(system.data.i_register == 0x0);
}

TEST_CASE("Fx29 sets the i register to 0x5 when Vx is 1")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF2;
    system.data.ram.at(system.data.program_counter + 1U) = 0x29;
    system.data.registers.at(0x2) = 1;
    system.data.i_register = 0x83;

    system.step();

    REQUIRE(system.data.i_register == 0x5);
}

TEST_CASE("Fx29 sets the i register to 0xA when Vx is 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xFA;
    system.data.ram.at(system.data.program_counter + 1U) = 0x29;
    system.data.registers.at(0xA) = 2;
    system.data.i_register = 0x83;

    system.step();

    REQUIRE(system.data.i_register == 0xA);
}

TEST_CASE("Fx29 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0;
    system.data.ram.at(system.data.program_counter + 1U) = 0x29;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "Fx33 stores the hundreds digit of Vx in ram at the location in the i register")
{
    const auto [reg, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x4_u8, 255_u8, 2_u8},
             {0x1_u8, 83_u8, 0_u8},
             {0xE_u8, 182_u8, 1_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x33;
    system.data.i_register = GENERATE(0x218_u16, 0x82_u16);
    system.data.registers.at(reg) = value;

    system.step();
    REQUIRE(system.data.ram.at(system.data.i_register) == expected);
}

TEST_CASE(
    "Fx33 stores the tens digit of Vx in ram at the location in the i register + 1")
{
    const auto [reg, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>({
            {0x4_u8, 255_u8, 5_u8},
            {0x1_u8, 83_u8, 8_u8},
            {0xE_u8, 2_u8, 0_u8},
        }));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x33;
    system.data.i_register = GENERATE(0x218_u16, 0x82_u16);
    system.data.registers.at(reg) = value;

    system.step();
    REQUIRE(system.data.ram.at(system.data.i_register + 1U) == expected);
}

TEST_CASE(
    "Fx33 stores the ones digit of Vx in ram at the location in the i register + 2")
{
    const auto [reg, value, expected] =
        GENERATE(table<std::uint8_t, std::uint8_t, std::uint8_t>(
            {{0x4_u8, 255_u8, 5_u8},
             {0x1_u8, 83_u8, 3_u8},
             {0xE_u8, 2_u8, 2_u8}}));

    auto system = ch8::chip8_system{};
    system.data.ram.at(system.data.program_counter) = 0xF0_u8 | reg;
    system.data.ram.at(system.data.program_counter + 1U) = 0x33;
    system.data.i_register = GENERATE(0x218_u16, 0x82_u16);
    system.data.registers.at(reg) = value;

    system.step();
    REQUIRE(system.data.ram.at(system.data.i_register + 2U) == expected);
}

TEST_CASE("Fx33 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0;
    system.data.ram.at(system.data.program_counter + 1U) = 0x33;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE(
    "Fx55 stores the values of V0 to Vx in ram at the address in the i register")
{
    const auto values = GENERATE(
        std::vector<std::uint8_t>{0x82, 0x56, 0x39, 0x73, 0x18, 0x59, 0x52,
                                  0x27, 0x01},
        std::vector<std::uint8_t>{0x83, 0x20, 0x42, 0x96});

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) =
        0xF0_u8 | static_cast<std::uint8_t>(values.size());
    system.data.ram.at(system.data.program_counter + 1U) = 0x55;
    std::copy(values.begin(), values.end(), system.data.registers.begin());
    system.data.i_register = GENERATE(0x20D_u16, 0x81_u16);

    system.step();

    REQUIRE(std::equal(
        values.begin(), values.end(),
        system.data.ram.begin() + system.data.i_register));
}

TEST_CASE("Fx55 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0;
    system.data.ram.at(system.data.program_counter + 1U) = 0x55;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}

TEST_CASE("Fx65 reads values from ram starting at the i register into V0 - Vx")
{
    const auto values = GENERATE(
        std::vector<std::uint8_t>{0x82, 0x56, 0x39, 0x73, 0x18, 0x59, 0x52,
                                  0x27, 0x01},
        std::vector<std::uint8_t>{0x83, 0x20, 0x42, 0x96});

    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) =
        0xF0_u8 | static_cast<std::uint8_t>(values.size());
    system.data.ram.at(system.data.program_counter + 1U) = 0x65;
    system.data.i_register = GENERATE(0x20D_u16, 0x81_u16);
    std::copy(
        values.begin(), values.end(),
        system.data.ram.begin() + system.data.i_register);

    system.step();

    REQUIRE(std::equal(
        values.begin(), values.end(), system.data.registers.begin()));
}

TEST_CASE("Fx65 increments the program counter by 2")
{
    auto system = ch8::chip8_system{};

    system.data.ram.at(system.data.program_counter) = 0xF0;
    system.data.ram.at(system.data.program_counter + 1U) = 0x65;

    const auto pc = system.data.program_counter;
    system.step();
    REQUIRE(system.data.program_counter == pc + 2);
}
