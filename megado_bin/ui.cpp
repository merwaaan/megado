#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "audio.h"
#include "core.h"
#include "settings.h"
#include "ui.h"

using namespace std;

#define DISASSEMBLY_LENGTH 10
#define MEMORY_VIEWER_COLUMNS 16

#define PALETTE_ENTRY_WIDTH 16
#define PATTERNS_COUNT 2048
#define PATTERNS_COLUMNS 32
#define PATTERN_MAGNIFICATION_FACTOR 16

#define GREYSCALE(g) {g, g, g}

// Black & white debug palette
static Color debug_palette[16] = {
    GREYSCALE(0),
    GREYSCALE(17),
    GREYSCALE(34),
    GREYSCALE(45),
    GREYSCALE(68),
    GREYSCALE(85),
    GREYSCALE(106),
    GREYSCALE(119),
    GREYSCALE(136),
    GREYSCALE(153),
    GREYSCALE(170),
    GREYSCALE(187),
    GREYSCALE(204),
    GREYSCALE(221),
    GREYSCALE(238),
    GREYSCALE(255)};

static void gen_texture(GLuint* texture)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static const ImVec4 color_black = {0.0f, 0.0f, 0.0f, 1.0f};
static const ImVec4 color_white = {1.0f, 1.0f, 1.0f, 1.0f};
static const ImVec4 color_dimmed = {0.5f, 0.5f, 0.5f, 1.0f};
static const ImVec4 color_disabled = {0.25f, 0.25f, 0.25f, 1.0f};
static const ImVec4 color_accent = {1.0f, 0.07f, 0.57f, 1.0f};
static const ImVec4 color_success = {0.0f, 1.0f, 0.0f, 1.0f};
static const ImVec4 color_error = {1.0f, 0.0f, 0.0f, 1.0f};
static const ImVec4 color_title = {0.0f, 0.68f, 0.71f, 1.0f};

static const ImVec2 vec_zero = {0.0f, 0.0f};
static const ImVec2 vec_one = {1.0f, 1.0f};

static void memory_viewer(const char* name, bool* opened, void* data, Size data_size, uint32_t data_length, uint32_t* target_address)
{
    if (ImGui::Begin(name, opened, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::BeginChild("##memory", ImVec2(0, 300), ImGuiChildFlags_AutoResizeX, 0);

        for (int row = 0; row < data_length / MEMORY_VIEWER_COLUMNS; ++row)
        {
            // Count digits for the address

            int addr_digits = 0;

            for (int i = data_length - 1; i > 0; i >>= 4)
            {
                ++addr_digits;
            }

            uint32_t row_address = row * MEMORY_VIEWER_COLUMNS;
            ImGui::Text("%0*X:  ", addr_digits, row_address);

            for (int column = 0; column < MEMORY_VIEWER_COLUMNS; ++column)
            {
                ImGui::SameLine(0, 0);

                if (data_size == Byte)
                {
                    ImGui::Text("%02X ", ((uint8_t*)data)[row_address + column]);
                }
                else
                {
                    ImGui::Text("%04X ", ((uint16_t*)data)[row_address + column]);
                }
            }
        }

        ImGui::EndChild();

        if (target_address != nullptr)
        {
            ImGui::Separator();

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Go to address");
            ImGui::SameLine(0, 10);
            ImGui::InputInt("##address", (int*)target_address, 16, 32, ImGuiInputTextFlags_CharsHexadecimal);

            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                // Re-enter the context of the scrolling area
                ImGui::BeginChild("##memory", ImVec2(0, 300), ImGuiChildFlags_AutoResizeX, 0);

                // Scroll to the appropriate row
                ImVec2 cursor_start_position = ImGui::GetCursorStartPos();
                ImGui::SetScrollFromPosY(cursor_start_position.y + (*target_address / MEMORY_VIEWER_COLUMNS) * ImGui::GetTextLineHeight(), 0);

                ImGui::EndChild();
            }
        }
    }
    ImGui::End();
}

static ImVec2 get_screen_cursor()
{
    ImVec2 window = ImGui::GetWindowPos();
    ImVec2 content = ImGui::GetCursorStartPos();

    return ImVec2(window.x + content.x, window.y + content.y);
}

static ImVec2 get_window_cursor()
{
    ImVec2 mouse = ImGui::GetMousePos();
    ImVec2 window = ImGui::GetWindowPos();
    ImVec2 content = ImGui::GetCursorStartPos();

    return ImVec2(mouse.x - (window.x + content.x), mouse.y - (window.y + content.y));
}

static void text_on_off(bool state, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    ImGui::TextV(format, args);
    va_end(args);

    ImGui::SameLine(0, 5);
    ImGui::TextColored(state ? color_accent : color_dimmed, state ? "on" : "off");
}

static void plot_metric(const Metric& m, const char* title)
{
    ImVec2 size = {0, 40};
    ImGui::PlotHistogram(title, m.values.data(), static_cast<int>(m.values.size()), static_cast<int>(m.index),
                         nullptr, 0, m.average * 2, size, sizeof(float));
}

UiEvents Ui::draw(Genesis* genesis, Settings& settings, const Audio& audio, const SnapshotSlots& snapshot_slots, double remaining_time)
{
    UiEvents events;

    // TODO wrapp all the igBegin in if, otherwise collapsed windows still render

    bool dummy_flag = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Game", true))
        {
            // Snapshots

            if (ImGui::BeginMenu("Save slots", true))
            {
                for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
                {
                    ImGui::Text("Slot %d:", slot);
                    ImGui::SameLine(0, 5);

                    if (!snapshot_slots[slot])
                    {
                        ImGui::TextColored(color_dimmed, "       empty       ");
                    }
                    else
                    {
                        char date[30];
                        strftime(date, 30, "%Y-%m-%d %H:%M:%S", localtime(&snapshot_slots[slot]->date));
                        ImGui::TextColored(color_accent, "%s", date);
                    }

                    ImGui::SameLine(0, 10);

                    char button_name[10];
                    sprintf(button_name, "save##%d", slot);
                    if (ImGui::Button(button_name, ImVec2(50, 20)))
                    {
                        events.save_snapshot = slot;
                    }

                    ImGui::SameLine(0, 3);

                    if (!snapshot_slots[slot])
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, color_disabled);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_disabled);
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color_disabled);
                        ImGui::PushStyleColor(ImGuiCol_Text, color_black);
                        ImGui::Button("load", ImVec2(50, 20));
                        ImGui::PopStyleColor(4);
                    }
                    else
                    {
                        sprintf(button_name, "load##%d", slot);
                        if (ImGui::Button(button_name, ImVec2(50, 20)))
                        {
                            events.load_snapshot = slot;
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            // Emulation speed (with reset button when != 1)

            ImGui::SliderFloat("Speed", &settings.emulation_speed, 0.2f, 3.0f, "%f");

            if (settings.emulation_speed != 1.0f)
            {
                ImGui::SameLine();

                if (ImGui::SmallButton("Reset##speed"))
                {
                    settings.emulation_speed = 1.0f;
                }
            }

            if (ImGui::MenuItem("Reset", nullptr, false, true))
            {
                events.reset = true;
            }

            if (ImGui::MenuItem(genesis->status == Status_Running ? "Pause" : "Resume", glfwGetKeyName(GLFW_KEY_P, 0), false, genesis->status != Status_NoGameLoaded))
            {
                events.toggle_pause = true;
            }

            if (ImGui::MenuItem("Step", "Space", false, genesis->status == Status_Pause))
            {
                events.step = true;
            }

            ImGui::Separator();

            ImGui::MenuItem("Metrics", nullptr, &settings.show_metrics, true);
            ImGui::MenuItem("Rewinding", nullptr, &settings.rewinding_enabled, true);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("CPU", true))
        {
            ImGui::MenuItem("Registers", nullptr, &settings.show_m68k_registers, true);
            ImGui::MenuItem("Disassembly", nullptr, &settings.show_m68k_disassembly, true);
            ImGui::MenuItem("Log", nullptr, &settings.show_m68k_log, true);

            ImGui::Separator();

            ImGui::MenuItem("ROM", nullptr, &settings.show_rom, true);
            ImGui::MenuItem("RAM", nullptr, &settings.show_ram, true);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Video", true))
        {

            ImGui::SliderFloat("Scaling", &settings.video_scale, 1.0f, 5.0f, "%f");

            if (ImGui::MenuItem("Full screen", nullptr, settings.full_screen, true))
            {
                events.toggle_full_screen = true;
            }

            ImGui::Separator();

            ImGui::MenuItem("Registers", nullptr, &settings.show_vdp_registers, true);
            ImGui::MenuItem("Palettes", nullptr, &settings.show_vdp_palettes, true);
            ImGui::MenuItem("Patterns", nullptr, &settings.show_vdp_patterns, true);
            ImGui::MenuItem("Planes", nullptr, &settings.show_vdp_planes, true);
            ImGui::MenuItem("Sprites", nullptr, &settings.show_vdp_sprites, true);

            ImGui::Separator();

            ImGui::MenuItem("VRAM", nullptr, &settings.show_vram, true);
            ImGui::MenuItem("VSRAM", nullptr, &settings.show_vsram, true);
            ImGui::MenuItem("CRAM", nullptr, &settings.show_cram, true);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio", true))
        {
            ImGui::MenuItem("Z80 registers", nullptr, &settings.show_z80_registers, true);
            ImGui::MenuItem("Z80 disassembly", nullptr, &settings.show_z80_disassembly, true);
            ImGui::MenuItem("Z80 log", nullptr, &settings.show_z80_log, true);

            ImGui::Separator();

            ImGui::MenuItem("PSG registers", nullptr, &settings.show_psg_registers, true);
            ImGui::MenuItem("YM2612 registers", nullptr, &settings.show_ym2612_registers, true);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // M68K registers

    if (settings.show_m68k_registers)
    {
        ImGui::SetNextWindowSize(ImVec2(440, 270), ImGuiCond_FirstUseEver);

        ImGui::Begin("CPU registers", &settings.show_m68k_registers, 0);

        ImGui::Columns(2, nullptr, true);

        for (int i = 0; i < 8; ++i)
        {
            ImGui::Text("D%d: %08X", i, genesis->m68k->data_registers[i]);
        }

        ImGui::NextColumn();

        for (int i = 0; i < 8; ++i)
        {
            ImGui::Text("A%d: %08X", i, genesis->m68k->address_registers[i]);
        }

        ImGui::Columns(1, nullptr, false);

        ImGui::Separator();

        ImGui::Text("Status: %04X", genesis->m68k->status);

#define READ_ONLY_CHECKBOX(label, value)                        \
    do                                                          \
    {                                                           \
        bool read_only_value = value;                           \
        ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f); \
        ImGui::BeginDisabled();                                 \
        ImGui::Checkbox(label, &read_only_value);               \
        ImGui::EndDisabled();                                   \
        ImGui::PopStyleVar(1);                                  \
    } while (0)

#define STATUS_BIT(label, bit)           \
    ImGui::Text(label);                  \
    READ_ONLY_CHECKBOX("##" label, bit); \
    ImGui::NextColumn()

        bool extended = EXTENDED(genesis->m68k);
        bool negative = NEGATIVE(genesis->m68k);
        bool zero = ZERO(genesis->m68k);
        bool overflow = OVERFLOW(genesis->m68k);
        bool carry = CARRY(genesis->m68k);

        ImGui::PushStyleColor(ImGuiCol_CheckMark, color_accent);
        ImGui::Columns(5, nullptr, false);
        STATUS_BIT("X", extended);
        STATUS_BIT("N", negative);
        STATUS_BIT("Z", zero);
        STATUS_BIT("V", overflow);
        STATUS_BIT("C", carry);
        ImGui::PopStyleColor(1);

        ImGui::Columns(1, nullptr, false);
        ImGui::Separator();

        ImGui::Text("PC:     %08X", genesis->m68k->pc);
        ImGui::Text("Cycles: %08X", genesis->m68k->cycles);

        ImGui::End();
    }

    // M68K disassembly

    if (settings.show_m68k_disassembly)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 320), ImGuiCond_FirstUseEver);
        ImGui::Begin("CPU Disassembly", &settings.show_m68k_disassembly, 0);

        ImGui::Columns(4, nullptr, false);
        ImGui::SetColumnOffset(1, 20);
        ImGui::SetColumnOffset(2, 100);

        ImGui::NextColumn();
        ImGui::Text("Address");
        ImGui::NextColumn();
        ImGui::Text("Instruction");
        ImGui::NextColumn();
        ImGui::Text("Opcode");
        ImGui::NextColumn();
        ImGui::Separator();

        uint32_t address = genesis->m68k->pc;
        for (int i = 0; i < DISASSEMBLY_LENGTH; ++i)
        {
            DecodedInstruction* instr = m68k_decode(genesis->m68k, address);

            // The memory may not contain valid opcodes, especially after branching instructions
            if (instr == nullptr)
            {
                ImGui::Columns(1, nullptr, false);
                ImGui::TextColored(color_dimmed, "Cannot decode opcode at %06X", address);

                // Fill the disassembler with empty lines to keep the same height
                for (int j = i + 1; j < DISASSEMBLY_LENGTH; ++j)
                {
                    ImGui::Text("");
                }

                break;
            }

            // Draw a bubble on lines with a breakpoint
            if (debugger_get_breakpoint(genesis->debugger, address) != nullptr)
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                ImVec2 window = ImGui::GetWindowPos();
                ImVec2 cursor = ImGui::GetCursorPos();
                ImVec2 bubble = ImVec2(window.x + cursor.x + 5, window.y + cursor.y + 7);

                draw_list->AddCircleFilled(bubble, 4, ImGui::GetColorU32(color_accent), 32);
            }
            ImGui::NextColumn();

            ImGui::TextColored(i == 0 ? color_accent : color_white, "%06X", address);
            ImGui::NextColumn();

            ImGui::TextColored(i == 0 ? color_accent : color_white, instr->mnemonics);

            // Toggle breakpoint when the instruction is clicked
            // TODO would be nice to have a hover feedback
            // TODO would be better to click the whole row but grouping seems to be interrupted by columns :(

            if (ImGui::IsItemClicked(0))
            {
                debugger_toggle_breakpoint(genesis->debugger, address);
            }

            ImGui::NextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                ImGui::TextColored(color_dimmed, "%02X ", m68k_read_b(genesis->m68k, address + byte));
                ImGui::SameLine(0, 0);
            }
            ImGui::NextColumn();

            address += instr->length;
            decoded_instruction_free(instr);
        }

        ImGui::Columns(1, nullptr, false);
        ImGui::Separator();

        ImGui::Text("Breakpoints");

        for (uint8_t i = 0; i < BREAKPOINTS_COUNT; ++i)
        {
            Breakpoint* b = &genesis->debugger->breakpoints[i];

            char name_buffer[100];

            sprintf(name_buffer, "##be%d", i);
            ImGui::Checkbox(name_buffer, &b->enabled);

            ImGui::SameLine(0, 10);

            sprintf(name_buffer, "##ba%d", i);
            ImGui::InputInt(name_buffer, (int*)&b->address, 1, 2, ImGuiInputTextFlags_CharsHexadecimal);
        }

        ImGui::End();
    }

    // Z80 registers

    if (settings.show_z80_registers)
    {
        Z80* z = genesis->z80;

        ImGui::SetNextWindowSize(ImVec2(440, 230), ImGuiCond_FirstUseEver);
        ImGui::Begin("Z80 registers", &settings.show_z80_registers, 0);
        ImGui::Columns(2, nullptr, true);

        ImGui::Text("AF: %04X", z->af);
        ImGui::Text("BC: %04X", z->bc);
        ImGui::Text("DE: %04X", z->de);
        ImGui::Text("HL: %04X", z->hl);
        ImGui::Text("IX: %04X", z->ix);
        ImGui::Text("I:    %02X", z->i);
        ImGui::Text("R:    %02X", z->r);

        ImGui::NextColumn();

        ImGui::Text("AF': %04X", z->af_);
        ImGui::Text("BC': %04X", z->bc_);
        ImGui::Text("DE': %04X", z->de_);
        ImGui::Text("HL': %04X", z->hl_);
        ImGui::Text("IY:  %04X", z->iy);
        ImGui::Text("PC:  %04X", genesis->z80->pc);
        ImGui::Text("SP:  %04X", genesis->z80->sp);

        ImGui::Columns(1, nullptr, false);
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_CheckMark, color_accent);
        ImGui::Columns(2, nullptr, false);
        READ_ONLY_CHECKBOX("running", z->running);
        ImGui::NextColumn();
        READ_ONLY_CHECKBOX("resetting", z->resetting);
        ImGui::NextColumn();

        bool sf = z->sf;
        bool zf = z->zf;
        bool yf = z->yf;
        bool hf = z->hf;
        bool xf = z->xf;
        bool pf = z->pf;
        bool nf = z->nf;
        bool cf = z->cf;

        ImGui::Columns(8, nullptr, false);
        STATUS_BIT("S", sf);
        STATUS_BIT("Z", zf);
        STATUS_BIT("Y", yf);
        STATUS_BIT("H", hf);
        STATUS_BIT("X", xf);
        STATUS_BIT("P", pf);
        STATUS_BIT("N", nf);
        STATUS_BIT("C", cf);
        ImGui::PopStyleColor(1);

        ImGui::End();
    }

    // M68K program log

    if (settings.show_m68k_log)
    {
        ImGui::SetNextWindowSize(ImVec2(380, 460), ImGuiCond_FirstUseEver);
        ImGui::Begin("CPU log", &settings.show_m68k_log, 0);

        Debugger* d = genesis->debugger;
        for (int i = 0; i < M68K_LOG_LENGTH; ++i)
        {
            uint32_t address = d->m68k_log_addresses[(d->m68k_log_cursor + 1 + i) % M68K_LOG_LENGTH];
            DecodedInstruction* instr = m68k_decode(genesis->m68k, address);
            if (instr != nullptr)
            {
                ImGui::Text("%04X   %s", address, instr->mnemonics);
                decoded_instruction_free(instr);
            }
        }

        ImGui::SetScrollHereY(0); // scroll to bottom
        // ImGui::Button("clear", )
        ImGui::End();
    }

    // Z80 disassembly

    if (settings.show_z80_disassembly)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 230), ImGuiCond_FirstUseEver);
        ImGui::Begin("Z80 Disassembly", &settings.show_z80_disassembly, 0);

        ImGui::Columns(4, nullptr, false);
        ImGui::SetColumnOffset(1, 0);
        ImGui::SetColumnOffset(2, 60);
        ImGui::SetColumnOffset(3, 250);

        ImGui::NextColumn();
        ImGui::Text("Address");
        ImGui::NextColumn();
        ImGui::Text("Instruction");
        ImGui::NextColumn();
        ImGui::Text("Opcode");
        ImGui::NextColumn();
        ImGui::Separator();

        uint32_t address = genesis->z80->pc;
        for (int i = 0; i < DISASSEMBLY_LENGTH; ++i)
        {
            FullyDecodedZ80Instruction* instr = z80_decode(genesis->z80, address);

            // The memory may not contain valid opcodes, especially after branching instructions

            if (instr == nullptr)
            {
                ImGui::Columns(1, nullptr, false);
                ImGui::TextColored(color_dimmed, "Cannot decode opcode at %04X", address);

                // Fill the disassembler with empty lines to keep the same height

                for (int j = i + 1; j < DISASSEMBLY_LENGTH; ++j)
                {
                    ImGui::Text("");
                }

                break;
            }
            ImGui::NextColumn();

            ImGui::TextColored(i == 0 ? color_accent : color_white, "%04X", address);
            ImGui::NextColumn();

            ImGui::TextColored(i == 0 ? color_accent : color_white, instr->mnemonics);
            ImGui::NextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                ImGui::TextColored(color_dimmed, "%02X ", z80_read(genesis->z80, address + byte));
                ImGui::SameLine(0, 0);
            }
            ImGui::NextColumn();

            address += instr->length;

            fully_decoded_z80_instruction_free(instr);
        }

        ImGui::End();
    }

    // Z80 program log

    if (settings.show_z80_log)
    {
        ImGui::SetNextWindowSize(ImVec2(260, 460), ImGuiCond_FirstUseEver);
        ImGui::Begin("Z80 log", &settings.show_z80_log, 0);

        Debugger* d = genesis->debugger;
        for (int i = 0; i < Z80_LOG_LENGTH; ++i)
        {
            LoggedZ80Instruction* instr = &d->z80_log_instrs[(d->z80_log_cursor + 1 + i) % Z80_LOG_LENGTH];
            ImGui::Text("%04X   %s", instr->address, instr->mnemonics);
        }

        ImGui::SetScrollHereY(0); // scroll to bottom
        ImGui::End();
    }

    // ROM

    if (settings.show_rom)
    {
        memory_viewer("ROM", &settings.show_rom, genesis->rom, Byte, 0x400000, &rom_target_address);
    }

    // RAM

    if (settings.show_ram)
    {
        memory_viewer("RAM", &settings.show_ram, genesis->ram, Byte, 0x10000, &ram_target_address);
    }

    // VDP registers

    if (settings.show_vdp_registers)
    {
        ImGui::SetNextWindowSize(ImVec2(720, 330), ImGuiCond_FirstUseEver);
        ImGui::Begin("VDP registers", &settings.show_vdp_registers, 0);
        ImGui::Columns(3, nullptr, false);

        Vdp* vdp = genesis->vdp;

#define REGISTER_SECTION(reg) ImGui::TextColored(color_title, "Register %0X [%02X]", reg, vdp->register_raw_values[reg])

        ImGui::BeginChild("column1", vec_zero, ImGuiChildFlags_None, 0);

        REGISTER_SECTION(0);
        ImGui::Bullet();
        text_on_off(vdp->hblank_interrupt_enabled, "H-blank interrupt:");
        ImGui::Bullet();
        text_on_off(vdp->hv_counter_latched, "HV-counter latched:");
        ImGui::Separator();

        REGISTER_SECTION(1);
        ImGui::Bullet();
        text_on_off(vdp->display_enabled, "Display enabled:");
        ImGui::Bullet();
        text_on_off(vdp->vblank_interrupt_enabled, "V-blank interrupt:");
        ImGui::Bullet();
        text_on_off(vdp->dma_enabled, "DMA enabled:");
        ImGui::Bullet();
        ImGui::Text("Display height: %0X", vdp->display_height);
        ImGui::Separator();

        REGISTER_SECTION(2);
        ImGui::Bullet();
        ImGui::Text("Plane A location: %0X", vdp->plane_a_nametable);
        ImGui::Separator();

        REGISTER_SECTION(3);
        ImGui::Bullet();
        ImGui::Text("Window location: %0X", vdp->window_nametable);
        ImGui::Separator();

        REGISTER_SECTION(4);
        ImGui::Bullet();
        ImGui::Text("Plane B location: %0X", vdp->plane_b_nametable);
        ImGui::Separator();

        REGISTER_SECTION(5);
        ImGui::Bullet();
        ImGui::Text("Sprite table location: %0X", vdp->sprites_attribute_table);

        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("column2", vec_zero, ImGuiChildFlags_None, 0);

        REGISTER_SECTION(7);
        ImGui::Bullet();
        ImGui::Text("Background palette: %0X", vdp->background_color_palette);
        ImGui::Bullet();
        ImGui::Text("Background color: %0X", vdp->background_color_entry);
        ImGui::Separator();

        REGISTER_SECTION(0xA);
        ImGui::Bullet();
        ImGui::Text("H-blank counter: %0X", vdp->hblank_line);
        ImGui::Separator();

        REGISTER_SECTION(0xB);
        const char* vertical_scrolling_mode_names[] = {"Screen", "two columns"};
        ImGui::Bullet();
        ImGui::Text("Vertical scrolling: %s", vertical_scrolling_mode_names[vdp->vertical_scrolling_mode]);
        const char* horizontal_scrolling_mode_names[] = {"Screen", "Invalid", "Row", "Line"};
        ImGui::Bullet();
        ImGui::Text("Horizontal scrolling: %s", horizontal_scrolling_mode_names[vdp->horizontal_scrolling_mode]);

        REGISTER_SECTION(0xC);
        ImGui::Bullet();
        ImGui::Text("Display width: %0X", vdp->display_width);
        ImGui::Bullet();
        text_on_off(vdp->shadow_highlight_enabled, "Shadow/Highlight:");
        ImGui::Bullet();
        ImGui::Text("Interlace mode: %0X", vdp->interlace_mode);
        ImGui::Separator();

        REGISTER_SECTION(0xD);
        ImGui::Bullet();
        ImGui::Text("Horizontal scroll table: %0X", vdp->horizontal_scrolltable);
        ImGui::Separator();

        REGISTER_SECTION(0xF);
        ImGui::Bullet();
        ImGui::Text("Auto-increment: %0X", vdp->auto_increment);

        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("column3", vec_zero, ImGuiChildFlags_None, 0);

        REGISTER_SECTION(0x10);
        ImGui::Bullet();
        ImGui::Text("Plane height: %0X", vdp->plane_width);
        ImGui::Bullet();
        ImGui::Text("Plane width: %0X", vdp->plane_height);
        ImGui::Separator();

        REGISTER_SECTION(0x11);
        ImGui::Bullet();
        ImGui::Text("Window horizontal direction: %s", vdp->window_plane_horizontal_direction ? "left" : "right");
        ImGui::Bullet();
        ImGui::Text("Window horizontal position: %0X", vdp->window_plane_horizontal_offset);
        ImGui::Separator();

        REGISTER_SECTION(0x12);
        ImGui::Bullet();
        ImGui::Text("Window vertical direction: %s", vdp->window_plane_vertical_direction ? "up" : "down");
        ImGui::Bullet();
        ImGui::Text("Window vertical position: %0X", vdp->window_plane_vertical_offset);
        ImGui::Separator();

        REGISTER_SECTION(0x13);
        REGISTER_SECTION(0x14);
        ImGui::Bullet();
        ImGui::Text("DMA length: %0X", vdp->dma_length);
        ImGui::Separator();

        REGISTER_SECTION(0x15);
        REGISTER_SECTION(0x16);
        REGISTER_SECTION(0x17);
        ImGui::Bullet();
        ImGui::Text("DMA source: %0X", vdp->dma_source_address_hi << 16 | vdp->dma_source_address_lo);

        ImGui::EndChild();
        ImGui::Columns(1, nullptr, false);
        ImGui::End();
    }

    // VDP palettes

    if (settings.show_vdp_palettes)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec_zero);
        ImGui::Begin("VDP palettes", &settings.show_vdp_palettes, ImGuiWindowFlags_AlwaysAutoResize);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 window_position = ImGui::GetCursorScreenPos();

        ImVec2 cell_top_left, cell_bottom_right;

        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 16; ++col)
            {
                cell_top_left.x = window_position.x + col * PALETTE_ENTRY_WIDTH;
                cell_top_left.y = window_position.y + row * PALETTE_ENTRY_WIDTH;
                cell_bottom_right.x = cell_top_left.x + PALETTE_ENTRY_WIDTH;
                cell_bottom_right.y = cell_top_left.y + PALETTE_ENTRY_WIDTH;

                Color color = genesis->vdp->cram[row * 16 + col];
                draw_list->AddRectFilled(cell_top_left, cell_bottom_right, 255u << 24 | color.b << 16 | color.g << 8 | color.r);
            }
        }

        ImGui::Dummy(ImVec2(16 * PALETTE_ENTRY_WIDTH, 4 * PALETTE_ENTRY_WIDTH));

        ImGui::End();
        ImGui::PopStyleVar(1);
    }

    // VDP patterns

    if (settings.show_vdp_patterns)
    {
        uint16_t patterns_width = PATTERNS_COLUMNS * 8;
        uint16_t patterns_height = PATTERNS_COUNT / PATTERNS_COLUMNS * 8;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec_zero);
        ImGui::Begin("VDP patterns", &settings.show_vdp_patterns, ImGuiWindowFlags_AlwaysAutoResize);

        // Update the pattern texture with the VRAM contents

        uint8_t patterns_buffer[PATTERNS_COUNT * 64 * 3]; // n patterns * 64 pixels * 3 color components

        Color* patterns_palette = patterns_palette_index < 0 ? debug_palette : genesis->vdp->cram + patterns_palette_index * 16;

        for (int pattern = 0; pattern < PATTERNS_COUNT; ++pattern)
        {
            int x = pattern % PATTERNS_COLUMNS * 8;
            int y = pattern / PATTERNS_COLUMNS * 8;
            vdp_draw_pattern(genesis->vdp, pattern, patterns_palette, patterns_buffer, patterns_width, x, y, false, false);
        }

        glBindTexture(GL_TEXTURE_2D, patterns_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, patterns_width, patterns_height, 0, GL_RGB, GL_UNSIGNED_BYTE, patterns_buffer);

        ImGui::ImageWithBg((ImTextureID)patterns_texture, ImVec2(patterns_width, patterns_height), vec_zero, vec_one, color_black, color_white);

        // If a pattern is hovered, show a tooltip with a magnified view

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
        {
            ImVec2 pattern_pos = get_window_cursor();
            uint16_t pattern_index = (int)pattern_pos.y / 8 * PATTERNS_COLUMNS + (int)pattern_pos.x / 8;

            uint8_t magnified_pattern_buffer[64 * 3];
            vdp_draw_pattern(genesis->vdp, pattern_index, patterns_palette, magnified_pattern_buffer, 8, 0, 0, false, false);

            glBindTexture(GL_TEXTURE_2D, magnified_pattern_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, magnified_pattern_buffer);

            ImGui::BeginTooltip();
            ImGui::Text("Pattern #%d", pattern_index);
            ImGui::ImageWithBg((ImTextureID)magnified_pattern_texture, ImVec2(8 * PATTERN_MAGNIFICATION_FACTOR, 8 * PATTERN_MAGNIFICATION_FACTOR), vec_zero, vec_one, color_white, color_white);
            ImGui::EndTooltip();
        }

        // Palette picker

        ImGui::RadioButton("Grayscale", &patterns_palette_index, -1);

        for (int palette = 0; palette < 4; ++palette)
        {
            char radio_name[32];
            snprintf(radio_name, sizeof(radio_name), "%d##patterns_palette", palette);
            ImGui::SameLine();
            ImGui::RadioButton(radio_name, &patterns_palette_index, palette);
        }

        ImGui::End();
        ImGui::PopStyleVar(1);
    }

    // VDP planes

    if (settings.show_vdp_planes)
    {
        uint16_t plane_width = 64 * 8;
        uint16_t plane_height = 64 * 8;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec_zero);
        ImGui::Begin("VDP planes", &settings.show_vdp_planes, ImGuiWindowFlags_AlwaysAutoResize);

        // Update the plane texture with the selected plane

        ranges::fill(plane_buffer, 0);
        vdp_draw_plane(genesis->vdp, selected_plane, plane_buffer.data(), 512);

        glBindTexture(GL_TEXTURE_2D, planes_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, plane_width, plane_height, 0, GL_RGB, GL_UNSIGNED_BYTE, plane_buffer.data());

        ImGui::ImageWithBg((ImTextureID)planes_texture, ImVec2(plane_width, plane_height), vec_zero, vec_one, color_black, color_white);

        // If a cell is hovered, show a tooltip with details
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
        {
            // Get the data of the hovered cell

            ImVec2 cell_pos = get_window_cursor();
            uint16_t cell_index = (int)cell_pos.y / 8 * genesis->vdp->plane_height + (int)cell_pos.x / 8;

            uint16_t pattern_index, palette_index;
            bool priority, horizontal_flip, vertical_flip;
            vdp_get_plane_cell_data(genesis->vdp, selected_plane, cell_index, &pattern_index, &palette_index, &priority, &horizontal_flip, &vertical_flip);

            // Draw a border around the hovered cell

            ImVec2 window = get_screen_cursor();
            ImVec2 top_left = {window.x + (int)(cell_pos.x / 8) * 8 + 1, window.y + (int)(cell_pos.y / 8) * 8 + 1};
            ImVec2 bottom_right = {top_left.x + 8, top_left.y + 8};

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(top_left, bottom_right, ImGui::GetColorU32(color_accent));

            // Draw a magnified version of the pattern

            Color* palette = genesis->vdp->cram + palette_index * 16;

            uint8_t magnified_pattern_buffer[64 * 3];
            vdp_draw_pattern(genesis->vdp, pattern_index, palette, magnified_pattern_buffer, 8, 0, 0, horizontal_flip, vertical_flip);

            glBindTexture(GL_TEXTURE_2D, magnified_pattern_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, magnified_pattern_buffer);

            ImGui::BeginTooltip();
            ImGui::Text("Cell (%d, %d)", (int)cell_pos.x / 8, (int)cell_pos.y / 8);
            ImGui::Text("Pattern #%d", pattern_index);

            ImGui::Text("%-18s", "Horizontal flip: ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(horizontal_flip ? color_accent : color_dimmed, horizontal_flip ? "on" : "off");

            ImGui::Text("%-18s", "Vertical flip: ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(vertical_flip ? color_accent : color_dimmed, vertical_flip ? "on" : "off");

            ImGui::Text("%-18s", "Priority: ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(priority ? color_accent : color_dimmed, priority ? "on" : "off");

            ImGui::ImageWithBg((ImTextureID)magnified_pattern_texture, ImVec2(8 * PATTERN_MAGNIFICATION_FACTOR, 8 * PATTERN_MAGNIFICATION_FACTOR), vec_zero, vec_one, color_white, color_white);
            ImGui::EndTooltip();
        }

        ImGui::Columns(3, nullptr, false);
        ImGui::RadioButton("Plane A", (int*)&selected_plane, Plane_A);
        ImGui::NextColumn();
        ImGui::RadioButton("Plane B", (int*)&selected_plane, Plane_B);
        ImGui::NextColumn();
        ImGui::RadioButton("Window", (int*)&selected_plane, Plane_Window);
        ImGui::Columns(1, nullptr, false);

        ImGui::End();
        ImGui::PopStyleVar(1);
    }

    // VDP sprites

    if (settings.show_vdp_sprites)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec_zero);
        ImGui::Begin("VDP sprites", &settings.show_vdp_sprites, ImGuiWindowFlags_AlwaysAutoResize);

        ImVec2 pos = get_screen_cursor();

        // Update the sprite texture

        ranges::fill(sprites_buffer, 0);
        vdp_draw_sprites(genesis->vdp, sprites_buffer.data(), 552);

        glBindTexture(GL_TEXTURE_2D, sprites_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 552, 552, 0, GL_RGB, GL_UNSIGNED_BYTE, sprites_buffer.data());

        ImGui::ImageWithBg((ImTextureID)sprites_texture, ImVec2(552, 552), vec_zero, vec_one, color_black, color_white);

        // Draw the screen border

        ImVec2 a = {pos.x + 128, pos.y + 128};
        ImVec2 b = {a.x + genesis->vdp->display_width * 8, a.y + genesis->vdp->display_height * 8};

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRect(a, b, ImGui::GetColorU32(color_accent));

        ImGui::End();
        ImGui::PopStyleVar(1);
    }

    // VRAM

    if (settings.show_vram)
    {
        memory_viewer("VRAM", &settings.show_vram, genesis->vdp->vram, Byte, 0x10000, nullptr);
    }

    // VSRAM

    if (settings.show_vsram)
    {
        memory_viewer("VSRAM", &settings.show_vsram, genesis->vdp->vsram, Word, 0x40, nullptr);
    }

    // CRAM

    if (settings.show_cram)
    {
        // Because we do not store raw CRAM data in the VDP implementation, convert
        // the decoded colors back to words for the debug view
        uint16_t raw_cram[0x40];
        for (int c = 0; c < 0x40; ++c)
        {
            raw_cram[c] = COLOR_STRUCT_TO_11(genesis->vdp->cram[c]);
        }

        memory_viewer("CRAM", &settings.show_cram, raw_cram, Word, 0x40, nullptr);
    }

    // Status bubble when the emulation is paused/rewinding
    // TODO would be better to always keep this overlay on top of the other windows but I don't think it's currently possible

    if (genesis->status == Status_Pause || genesis->status == Status_Rewinding)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_None, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 0.10f));
        ImGui::Begin("Example: Fixed Overlay", &dummy_flag, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(color_accent, genesis->status == Status_Pause ? "Paused" : "Rewinding");
        ImGui::End();
        ImGui::PopStyleColor(1);
    }

    double now = glfwGetTime();
    double dt = now - last_time;
    last_time = now;

    time_per_frame.push(dt * 1000);
    audio_buffer_queue.push(audio.queued_samples());

    metrics_refresh_counter += dt;
    if (metrics_refresh_counter > 1)
    {
        metrics_refresh_counter = 0;
        time_per_frame.compute_average();
        audio_buffer_queue.compute_average();
    }

    if (settings.show_metrics)
    {
        ImGui::SetNextWindowSize(ImVec2(620, 230), ImGuiCond_FirstUseEver);
        ImGui::Begin("Metrics", &settings.show_metrics, 0);
        char buf[50];

        // Time per frame, in milliseconds

        snprintf(buf, sizeof(buf), "time per frame (ms)\navg: %.2f\nfps: %.2f",
                 time_per_frame.average, time_per_frame.average > 0 ? 1000.0 / time_per_frame.average : 0);
        plot_metric(time_per_frame, buf);

        // (M68k) instructions per frame, in kilos

        snprintf(buf, sizeof buf, "audio queue (samples)\navg: %.2f", audio_buffer_queue.average);
        plot_metric(audio_buffer_queue, buf);

        ImGui::TextColored(color_title, "Remaining audio time: ");
        ImGui::SameLine(0, 0);
        ImGui::Text("%.6fms", remaining_time * 1000);

        ImGui::TextColored(color_title, "Remaining master cycles");
        ImGui::Text("M68k:   %d", genesis->m68k->remaining_master_cycles);
        ImGui::Text("Z80:    %d", genesis->z80->remaining_master_cycles);
        ImGui::Text("PSG:    %d", genesis->psg->remaining_master_cycles);
        ImGui::Text("YM2612: %d", genesis->ym2612->remaining_master_cycles);

        ImGui::End();
    }

    // PSG registers

    if (settings.show_psg_registers)
    {
        ImGui::Begin("PSG registers", &settings.show_psg_registers, ImGuiWindowFlags_AlwaysAutoResize);

        PSG* p = genesis->psg;

        for (int i = 0; i < 3; ++i)
        {
            ImGui::TextColored(color_title, "Square %d", i);
            if (p->square[i].volume == 0xF)
            {
                ImGui::SameLine(0, 0);
                ImGui::TextColored(color_dimmed, " [silent]");
            }
            ImGui::Text("volume:  %0X", p->square[i].volume);
            ImGui::Text("tone:    %0X [%.2fHz]", p->square[i].tone, square_tone_in_hertz(&p->square[i]));
            ImGui::Text("counter: %0X", p->square[i].counter);
            ImGui::Text("output:  %d", square_output(&p->square[i]));
        }

        ImGui::TextColored(color_title, "Noise");
        if (p->noise.volume == 0xF)
        {
            ImGui::SameLine(0, 0);
            ImGui::TextColored(color_dimmed, " [silent]");
        }
        ImGui::Text("volume:     %0X", p->noise.volume);
        ImGui::Text("mode:       %0X [%s]", p->noise.mode, p->noise.mode ? "white" : "periodic");
        ImGui::Text("shift rate: %0X", p->noise.shift_rate);
        ImGui::Text("counter:    %0X", p->noise.counter);
        ImGui::Text("lfsr:       %0X", p->noise.lfsr);
        ImGui::Text("output:     %d", noise_output(&p->noise));
        ImGui::End();
    }

    // YM2612 registers

    if (settings.show_ym2612_registers)
    {
        ImGui::SetNextWindowSize(ImVec2(700, 360), ImGuiCond_FirstUseEver);
        ImGui::Begin("YM2612 registers", &settings.show_ym2612_registers, 0);

        YM2612* y = genesis->ym2612;

        // Global registers

        ImGui::TextColored(color_title, "Global registers");
        ImGui::Columns(3, nullptr, false);

        ImGui::Text("LFO enabled:   %s", y->lfo_enabled ? "true" : "false");
        ImGui::Text("LFO frequency: %0X", y->lfo_frequency_index);

        ImGui::NextColumn();

        ImGui::Text("timer A: %0X", y->timer_a);
        ImGui::Text("timer B: %0X", y->timer_b);

        ImGui::NextColumn();

        ImGui::Text("DAC enabled: %0X", y->dac_enabled);
        ImGui::Text("DAC data:    %0X", y->dac_data);

        ImGui::Separator();

        // Channels

        ImGui::Columns(7, nullptr, false);

        ImVec2 dummy = {0, 0}; // Don't know what it does, but needed
                               // as argument to IgSelectable
        ImGui::TextColored(color_title, "Channel");
        ImGui::Selectable("enabled", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("freq number", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("frequency", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("feedback", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("algorithm", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("stereo", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("amp modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        ImGui::Selectable("freq modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);

        ImGui::NextColumn();

        for (int i = 0; i < 6; ++i)
        {
            ImGui::TextColored(color_title, "%d", i + 1);

            // FIXME: the checkbox is too tall for the line, shifting all the
            // lines in the table after it.
            ImGui::PushStyleColor(ImGuiCol_CheckMark, color_accent);
            ImGui::SameLine(20, 0);
            char name_buffer[10];
            sprintf(name_buffer, "##chan%d", i);
            ImGui::Checkbox(name_buffer, &y->channels[i].muted);
            ImGui::PopStyleColor(1);

            if ((i + 1) % 3 == 0)
            {
                uint8_t channel_mode = i == 3 ? y->channel3_mode : y->channel6_mode;

                const char* mode_string = "normal";
                switch (channel_mode)
                {
                case 0:
                    mode_string = "normal";
                    break;
                case 1:
                    mode_string = "special";
                    break;
                default:
                    mode_string = "illegal";
                    break;
                }

                ImGui::SameLine(0, 0);
                ImGui::Text(" (%s)", mode_string);
            }

            ImGui::Text("%d", y->channels[i].enabled);
            ImGui::Text("%d:%d", y->channels[i].frequency.block, y->channels[i].frequency.freq);
            ImGui::Text("%.2fHz", channel_frequency_in_hertz(&y->channels[i], genesis_master_frequency(genesis)));
            ImGui::Text("%0X", y->channels[i].feedback);
            ImGui::Text("%0X", y->channels[i].algorithm);
            ImGui::Text("%s%s", y->channels[i].left_output ? "L" : " ", y->channels[i].right_output ? "R" : " ");
            ImGui::Text("%0X", y->channels[i].amplitude_modulation_sensitivity);
            ImGui::Text("%0X", y->channels[i].frequency_modulation_sensitivity);

            ImGui::NextColumn();
        }

        ImGui::Separator();

        // Operators

        for (int i = 0; i < 6; ++i)
        {
            ImGui::Columns(1, nullptr, false);
            if (ImGui::TreeNode((void*)(intptr_t)i, "Operators for channel %d", i + 1))
            {

                ImGui::Columns(5, nullptr, false);

                ImGui::TextColored(color_title, "Operator");
                ImGui::Selectable("detune", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("multiple", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("total level", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("attack rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("decay rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("sustain level", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("sustain rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("release rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("rate scaling", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                ImGui::Selectable("amp modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);

                ImGui::NextColumn();

                for (int j = 0; j < 4; ++j)
                {
                    ImGui::TextColored(color_title, "%d", j + 1);

                    ImGui::Text("%0X", y->channels[i].operators[j].detune);
                    ImGui::Text("%0X", y->channels[i].operators[j].multiple);
                    ImGui::Text("%0X", y->channels[i].operators[j].total_level);
                    ImGui::Text("%0X", y->channels[i].operators[j].attack_rate);
                    ImGui::Text("%0X", y->channels[i].operators[j].decay_rate);
                    ImGui::Text("%0X", y->channels[i].operators[j].sustain_level);
                    ImGui::Text("%0X", y->channels[i].operators[j].sustain_rate);
                    ImGui::Text("%0X", y->channels[i].operators[j].release_rate);
                    ImGui::Text("%0X", y->channels[i].operators[j].rate_scaling);
                    ImGui::Text("%0X", y->channels[i].operators[j].amplitude_modulation_enabled);

                    ImGui::NextColumn();
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    return events;
}

Ui::Ui()
    : plane_buffer(64 * 8 * 64 * 8 * 3), sprites_buffer(552 * 552 * 3)
{
    ImGui::GetStyle().WindowRounding = 0.0f;

    gen_texture(&patterns_texture);
    gen_texture(&magnified_pattern_texture);
    gen_texture(&planes_texture);
    gen_texture(&sprites_texture);
}

Ui::~Ui()
{
    glDeleteTextures(1, &patterns_texture);
    glDeleteTextures(1, &magnified_pattern_texture);
    glDeleteTextures(1, &planes_texture);
    glDeleteTextures(1, &sprites_texture);
}
