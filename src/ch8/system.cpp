#include "ch8/system.hpp"
#include <cstdint>
#include <fstream>

[[nodiscard]] constexpr auto
create_opcode(std::uint8_t byte1, std::uint8_t byte2) noexcept -> std::uint16_t;

auto op_00E0(
    ch8::chip8_data& data,
    ch8::observable<const ch8::frame_buffer<64, 32>&>& on_draw) -> void;
constexpr auto op_00EE(ch8::chip8_data& data) -> void;
constexpr auto op_1nnn(ch8::chip8_data& data, std::uint16_t opcode) noexcept
    -> void;
constexpr auto op_2nnn(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_3xkk(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_4xkk(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_5xy0(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_6xkk(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_7xkk(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_8xy0(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_8xy1(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_8xy2(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_8xy3(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_8xy4(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_8xy5(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto
op_8xy6(ch8::chip8_data& data, std::uint16_t opcode, bool accurate) -> void;
constexpr auto op_8xy7(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto
op_8xyE(ch8::chip8_data& data, std::uint16_t opcode, bool accurate) -> void;
constexpr auto op_9xy0(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Annn(ch8::chip8_data& data, std::uint16_t opcode) noexcept
    -> void;
constexpr auto op_Bnnn(ch8::chip8_data& data, std::uint16_t opcode) -> void;
auto op_Cxkk(ch8::chip8_data& data, std::mt19937& rng, std::uint16_t opcode)
    -> void;
auto op_Dxyn(
    ch8::chip8_data& data,
    ch8::observable<const ch8::frame_buffer<64, 32>&>& on_draw,
    std::uint16_t opcode) -> void;
auto op_Ex9E(ch8::chip8_data& data, std::uint16_t opcode) -> void;
auto op_ExA1(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx07(ch8::chip8_data& data, std::uint16_t opcode) -> void;
auto op_Fx0A(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx15(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx18(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx1E(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx29(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx33(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx55(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto op_Fx65(ch8::chip8_data& data, std::uint16_t opcode) -> void;
constexpr auto unknown_opcode() noexcept -> void;

constexpr auto chip8_font = std::array<std::uint8_t, 80>{
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10,
    0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
    0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
    0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
    0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
    0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};

ch8::chip8_data::chip8_data() noexcept
    : program_counter{program_start}
    , i_register{0x0}
    , delay_timer{0}
    , sound_timer{0}
    , stack_pointer{-1}
    , ram{}
    , registers{}
    , stack{}
    , screen{}
    , waiting_for_keypress{false}
{
    std::copy(chip8_font.begin(), chip8_font.end(), ram.begin());
    screen.clear({0, 0, 0, 255});
}

template <typename Generator>
[[nodiscard]] auto make_random_engine() -> Generator
{
    auto seed_data = std::array<std::uint32_t, Generator::state_size>{};
    auto random_device = std::random_device{};
    std::generate(seed_data.begin(), seed_data.end(), std::ref(random_device));
    auto seed_seq = std::seed_seq(seed_data.begin(), seed_data.end());

    return Generator{seed_seq};
}

ch8::chip8_system::chip8_system()
    : data{}
    , updates_per_second{800}
    , accurate_8xyE{true}
    , accurate_8xy6{true}
    , time_since_update{0}
    , time_since_timer_update{0}
    , rng{make_random_engine<decltype(rng)>()}
{
}

auto ch8::chip8_system::step() -> void
{
    const auto byte1 = data.ram.at(data.program_counter);
    const auto byte2 = data.ram.at(data.program_counter + 1U);
    const auto opcode = create_opcode(byte1, byte2);

    if (!data.waiting_for_keypress) {
        data.program_counter += 2;
    }

    switch (opcode & 0xF000U) {
    case 0x0000:
        switch (opcode) {
        case 0x00E0:
            op_00E0(data, on_draw);
            break;
        case 0x00EE:
            op_00EE(data);
            break;
        default:
            unknown_opcode();
            break;
        }
        break;
    case 0x1000:
        op_1nnn(data, opcode);
        break;
    case 0x2000:
        op_2nnn(data, opcode);
        break;
    case 0x3000:
        op_3xkk(data, opcode);
        break;
    case 0x4000:
        op_4xkk(data, opcode);
        break;
    case 0x5000:
        op_5xy0(data, opcode);
        break;
    case 0x6000:
        op_6xkk(data, opcode);
        break;
    case 0x7000:
        op_7xkk(data, opcode);
        break;
    case 0x8000:
        switch (opcode & 0x000FU) {
        case 0x0000:
            op_8xy0(data, opcode);
            break;
        case 0x0001:
            op_8xy1(data, opcode);
            break;
        case 0x0002:
            op_8xy2(data, opcode);
            break;
        case 0x0003:
            op_8xy3(data, opcode);
            break;
        case 0x0004:
            op_8xy4(data, opcode);
            break;
        case 0x0005:
            op_8xy5(data, opcode);
            break;
        case 0x0006:
            op_8xy6(data, opcode, accurate_8xy6);
            break;
        case 0x0007:
            op_8xy7(data, opcode);
            break;
        case 0x000E:
            op_8xyE(data, opcode, accurate_8xyE);
            break;
        default:
            unknown_opcode();
            break;
        }
        break;
    case 0x9000:
        op_9xy0(data, opcode);
        break;
    case 0xA000:
        op_Annn(data, opcode);
        break;
    case 0xB000:
        op_Bnnn(data, opcode);
        break;
    case 0xC000:
        op_Cxkk(data, rng, opcode);
        break;
    case 0xD000:
        op_Dxyn(data, on_draw, opcode);
        break;
    case 0xE000:
        switch (opcode & 0x00FFU) {
        case 0x009E:
            op_Ex9E(data, opcode);
            break;
        case 0x00A1:
            op_ExA1(data, opcode);
            break;
        default:
            unknown_opcode();
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FFU) {
        case 0x0007:
            op_Fx07(data, opcode);
            break;
        case 0x000A:
            op_Fx0A(data, opcode);
            break;
        case 0x0015:
            op_Fx15(data, opcode);
            break;
        case 0x0018:
            op_Fx18(data, opcode);
            break;
        case 0x001E:
            op_Fx1E(data, opcode);
            break;
        case 0x0029:
            op_Fx29(data, opcode);
            break;
        case 0x0033:
            op_Fx33(data, opcode);
            break;
        case 0x0055:
            op_Fx55(data, opcode);
            break;
        case 0x0065:
            op_Fx65(data, opcode);
            break;
        default:
            unknown_opcode();
            break;
        }
        break;
    default:
        unknown_opcode();
        break;
    }
}

auto ch8::chip8_system::execute(const delta_time dt) -> void
{
    namespace chrono = std::chrono;
    using namespace std::chrono_literals;

    time_since_timer_update += dt;
    time_since_update += dt;

    if (time_since_timer_update >= 16ms) {
        if (data.sound_timer > 0) {
            data.sound_timer--;
        }
        if (data.delay_timer > 0) {
            data.delay_timer--;
        }
        time_since_timer_update = 0us;
    }

    if (updates_per_second <= 0) {
        return;
    }

    const auto tick = chrono::microseconds{1'000'000 / updates_per_second};
    if (time_since_update >= tick) {
        step();
        time_since_update = 0us;
    }
}

auto ch8::chip8_system::reset() noexcept -> void
{
    data = chip8_data{};
}

[[nodiscard]] auto
ch8::chip8_system::load_program(const std::filesystem::path& program_file)
    -> ch8::load_status
{
    const auto max_filesize = data.ram.size() - chip8_data::program_start;

    if (!std::filesystem::exists(program_file)) {
        return ch8::load_status::file_does_not_exist;
    }

    if (std::filesystem::file_size(program_file) > max_filesize) {
        return ch8::load_status::file_too_big;
    }

    auto file = std::ifstream{program_file, std::ios::binary};
    file.unsetf(std::ios::skipws);
    const auto begin = std::istream_iterator<std::uint8_t>{file};
    const auto end = std::istream_iterator<std::uint8_t>{};

    std::copy(begin, end, data.ram.begin() + data.program_counter);

    return ch8::load_status::ok;
}

[[nodiscard]] constexpr auto
create_opcode(const std::uint8_t byte1, const std::uint8_t byte2) noexcept
    -> std::uint16_t
{
    auto opcode = std::uint16_t{0x0};
    opcode |= byte1;
    opcode <<= 8U;
    opcode |= byte2;

    return opcode;
}

auto op_00E0(
    ch8::chip8_data& data,
    ch8::observable<const ch8::frame_buffer<64, 32>&>& on_draw) -> void
{
    data.screen.clear({0, 0, 0, 255});
    on_draw.notify(data.screen);
}

constexpr auto op_00EE(ch8::chip8_data& data) -> void
{
    const auto sp = static_cast<std::size_t>(data.stack_pointer);
    data.program_counter = data.stack.at(sp);
    data.stack_pointer--;
}

constexpr auto
op_1nnn(ch8::chip8_data& data, const std::uint16_t opcode) noexcept -> void
{
    data.program_counter = opcode & 0x0FFFU;
}

constexpr auto op_2nnn(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    data.stack_pointer++;
    const auto sp = static_cast<std::size_t>(data.stack_pointer);
    data.stack.at(sp) = data.program_counter;
    data.program_counter = opcode & 0x0FFFU;
}

constexpr auto op_3xkk(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    const auto reg_value = data.registers.at(reg);

    const auto value = static_cast<std::uint8_t>(opcode & 0x00FFU);

    if (reg_value == value) {
        data.program_counter += 2U;
    }
}

constexpr auto op_4xkk(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    const auto reg_value = data.registers.at(reg);
    const auto value = static_cast<std::uint8_t>(opcode & 0x00FFU);

    if (reg_value != value) {
        data.program_counter += 2U;
    }
}

constexpr auto op_5xy0(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    if (data.registers.at(reg_x) == data.registers.at(reg_y)) {
        data.program_counter += 2U;
    }
}

constexpr auto op_6xkk(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    const auto value = static_cast<std::uint8_t>(opcode & 0x00FFU);
    data.registers.at(reg) = value;
}

constexpr auto op_7xkk(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto value = static_cast<std::uint8_t>(opcode & 0x00FFU);
    const auto reg = (opcode & 0x0F00U) >> 8U;
    data.registers.at(reg) += value;
}

constexpr auto op_8xy0(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;
    data.registers.at(reg_x) = data.registers.at(reg_y);
}

constexpr auto op_8xy1(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;
    data.registers.at(reg_x) |= data.registers.at(reg_y);
}

constexpr auto op_8xy2(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;
    data.registers.at(reg_x) &= data.registers.at(reg_y);
}

constexpr auto op_8xy3(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;
    data.registers.at(reg_x) ^= data.registers.at(reg_y);
}

constexpr auto op_8xy4(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    const auto x_value = data.registers.at(reg_x);
    const auto y_value = data.registers.at(reg_y);

    const auto new_value = static_cast<unsigned>(x_value + y_value);

    data.registers.at(reg_x) = static_cast<std::uint8_t>(new_value);

    if (new_value > 255) {
        data.registers.at(0xF) = 1;
    }
    else {
        data.registers.at(0xF) = 0;
    }
}

constexpr auto op_8xy5(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    const auto x_value = data.registers.at(reg_x);
    const auto y_value = data.registers.at(reg_y);

    data.registers.at(reg_x) = x_value - y_value;

    if (x_value > y_value) {
        data.registers.at(0xF) = 1;
    }
    else {
        data.registers.at(0xF) = 0;
    }
}

constexpr auto
op_8xy6(ch8::chip8_data& data, const std::uint16_t opcode, bool accurate)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    const auto val =
        accurate ? data.registers.at(reg_y) : data.registers.at(reg_x);

    const auto lsb = static_cast<std::uint8_t>(val & 0b00000001U);

    data.registers.at(reg_x) = val >> 1U;
    data.registers.at(0xF) = lsb;
}

constexpr auto op_8xy7(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    const auto x_value = data.registers.at(reg_x);
    const auto y_value = data.registers.at(reg_y);

    data.registers.at(reg_x) = y_value - x_value;

    if (y_value > x_value) {
        data.registers.at(0xF) = 1;
    }
    else {
        data.registers.at(0xF) = 0;
    }
}

constexpr auto
op_8xyE(ch8::chip8_data& data, const std::uint16_t opcode, bool accurate)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    const auto val =
        accurate ? data.registers.at(reg_y) : data.registers.at(reg_x);

    const auto msb = static_cast<std::uint8_t>(val >> 7U);

    data.registers.at(reg_x) = static_cast<std::uint8_t>(val << 1U);
    data.registers.at(0xF) = msb;
}

constexpr auto op_9xy0(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg_x = (opcode & 0x0F00U) >> 8U;
    const auto reg_y = (opcode & 0x00F0U) >> 4U;

    if (data.registers.at(reg_x) != data.registers.at(reg_y)) {
        data.program_counter += 2;
    }
}

constexpr auto
op_Annn(ch8::chip8_data& data, const std::uint16_t opcode) noexcept -> void
{
    data.i_register = opcode & 0x0FFFU;
}

constexpr auto op_Bnnn(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    data.program_counter = (opcode & 0x0FFFU) + data.registers.at(0x0);
}

auto op_Cxkk(
    ch8::chip8_data& data, std::mt19937& rng, const std::uint16_t opcode)
    -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;

    auto dist = std::uniform_int_distribution{0U, 255U};
    const auto random_number = static_cast<std::uint8_t>(dist(rng));

    const auto value = static_cast<std::uint8_t>(opcode & 0x00FFU);

    data.registers.at(reg) = value & random_number;
}

auto op_Dxyn(
    ch8::chip8_data& data,
    ch8::observable<const ch8::frame_buffer<64, 32>&>& on_draw,
    const std::uint16_t opcode) -> void
{
    const auto x_reg = (opcode & 0x0F00U) >> 8U;
    const auto y_reg = (opcode & 0x00F0U) >> 4U;
    const auto x_pos = data.registers.at(x_reg);
    const auto y_pos = data.registers.at(y_reg);
    const auto sprite_size = opcode & 0x000FU;

    bool erased_a_pixel = false;

    for (auto sprite_y = std::size_t{0}; sprite_y < sprite_size; ++sprite_y) {
        const auto sprite_byte = data.ram.at(data.i_register + sprite_y);

        for (auto sprite_x = std::size_t{0}; sprite_x < 8; ++sprite_x) {
            const auto mask =
                static_cast<std::uint8_t>(0b10000000U >> sprite_x);

            const auto sprite_pixel = (sprite_byte & mask) > 0;

            const auto screen_height = data.screen.height();
            const auto screen_width = data.screen.width();

            const auto screen_y_pos = (sprite_y + y_pos) % screen_height;
            const auto screen_x_pos = (sprite_x + x_pos) % screen_width;

            const auto old_pixel =
                data.screen.pixel(screen_x_pos, screen_y_pos) !=
                ch8::color{0, 0, 0, 255};

            const auto new_pixel = static_cast<bool>(old_pixel ^ sprite_pixel);

            if (!new_pixel && old_pixel) {
                erased_a_pixel = true;
            }

            const auto color = new_pixel ? ch8::color{255, 255, 255, 255}
                                         : ch8::color{0, 0, 0, 255};

            data.screen.pixel(screen_x_pos, screen_y_pos, color);
        }
    }

    data.registers.at(0xF) = static_cast<std::uint8_t>(erased_a_pixel);
    on_draw.notify(data.screen);
}

auto op_Ex9E(ch8::chip8_data& data, const std::uint16_t opcode) -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    const auto key = data.registers.at(reg);
    if (data.keypad.test(key)) {
        data.program_counter += 2U;
    }
}

auto op_ExA1(ch8::chip8_data& data, const std::uint16_t opcode) -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    const auto key = data.registers.at(reg);
    if (!data.keypad.test(key)) {
        data.program_counter += 2U;
    }
}

constexpr auto op_Fx07(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    data.registers.at(reg) = data.delay_timer;
}

auto op_Fx0A(ch8::chip8_data& data, const std::uint16_t opcode) -> void
{
    const auto reg = (opcode & 0x0F00U) >> 8U;
    const auto reg_val = data.registers.at(reg);

    if (!data.waiting_for_keypress) {
        data.keypad.reset();
        data.registers.at(reg) = 0xFF;

        data.waiting_for_keypress = true;
        data.program_counter -= 2U;
    }
    else if (reg_val == 0xFF) {
        for (auto i = std::size_t{0}; i < data.keypad.size(); ++i) {
            if (data.keypad.test(i)) {
                data.registers.at(reg) = static_cast<std::uint8_t>(i);
            }
        }
    }
    else if (reg_val != 0xFF && !data.keypad.test(reg_val)) {
        data.waiting_for_keypress = false;
        data.program_counter += 2U;
    }
}

constexpr auto op_Fx15(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    data.delay_timer = data.registers.at(reg);
}

constexpr auto op_Fx18(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    data.sound_timer = data.registers.at(reg);
}

constexpr auto op_Fx1E(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    data.i_register += data.registers.at(reg);
}

constexpr auto op_Fx29(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    data.i_register = data.registers.at(reg) * 5U;
}

constexpr auto op_Fx33(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    const auto reg_val = data.registers.at(reg);

    data.ram.at(data.i_register) = (reg_val / 100) % 10;
    data.ram.at(data.i_register + 1U) = (reg_val / 10) % 10;
    data.ram.at(data.i_register + 2U) = reg_val % 10;
}

constexpr auto op_Fx55(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    for (auto i = std::size_t{0}; i <= reg; ++i) {
        data.ram.at(data.i_register + i) = data.registers.at(i);
    }
}

constexpr auto op_Fx65(ch8::chip8_data& data, const std::uint16_t opcode)
    -> void
{
    const auto reg = (0x0F00U & opcode) >> 8U;
    for (auto i = std::size_t{0}; i <= reg; ++i) {
        data.registers.at(i) = data.ram.at(data.i_register + i);
    }
}

constexpr auto unknown_opcode() noexcept -> void
{
}
