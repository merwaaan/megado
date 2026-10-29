#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "genesis.h"
#include "m68k/m68k.h"
#include "metric.h"
#include "psg.h"
#include "settings.h"
#include "snapshot.h"
#include "ui.h"
#include "ym2612.h"
#include "z80.h"

static ImTextureRef_c texture_ref(ImTextureID id)
{
    ImTextureRef_c ref = {0};
    ref._TexID = id;
    return ref;
}

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

static const ImVec4_c color_black = {0.0f, 0.0f, 0.0f, 1.0f};
static const ImVec4_c color_white = {1.0f, 1.0f, 1.0f, 1.0f};
static const ImVec4_c color_dimmed = {0.5f, 0.5f, 0.5f, 1.0f};
static const ImVec4_c color_disabled = {0.25f, 0.25f, 0.25f, 1.0f};
static const ImVec4_c color_accent = {1.0f, 0.07f, 0.57f, 1.0f};
static const ImVec4_c color_success = {0.0f, 1.0f, 0.0f, 1.0f};
static const ImVec4_c color_error = {1.0f, 0.0f, 0.0f, 1.0f};
static const ImVec4_c color_title = {0.0f, 0.68f, 0.71f, 1.0f};

static const ImVec2_c vec_zero = {0.0f, 0.0f};
static const ImVec2_c vec_one = {1.0f, 1.0f};

static void memory_viewer(char* name, bool* opened, void* data, Size data_size, uint32_t data_length, uint32_t* target_address)
{
    // igSetNextWindowSize(vec_zero, 0);
    if (igBegin(name, opened, ImGuiWindowFlags_AlwaysAutoResize))
    {
        igBeginChild_Str("##memory", (ImVec2_c){0, 300}, ImGuiChildFlags_AutoResizeX, 0);

        for (int row = 0; row < data_length / MEMORY_VIEWER_COLUMNS; ++row)
        {
            // Count digits for the address
            int addr_digits = 0;
            for (int i = data_length - 1; i > 0; i >>= 4)
            {
                ++addr_digits;
            }

            uint32_t row_address = row * MEMORY_VIEWER_COLUMNS;
            igText("%0*X:  ", addr_digits, row_address);

            for (int column = 0; column < MEMORY_VIEWER_COLUMNS; ++column)
            {
                igSameLine(0, 0);

                if (data_size == Byte)
                {
                    igText("%02X ", ((uint8_t*)data)[row_address + column]);
                }
                else
                {
                    igText("%04X ", ((uint16_t*)data)[row_address + column]);
                }
            }
        }

        igEndChild();

        if (target_address != NULL)
        {
            igSeparator();

            igAlignTextToFramePadding();
            igText("Go to address");
            igSameLine(0, 10);
            igInputInt("##address", (int*)target_address, 16, 32, ImGuiInputTextFlags_CharsHexadecimal);
            if (igIsItemDeactivatedAfterEdit())
            {
                // Re-enter the context of the scrolling area
                igBeginChild_Str("##memory", (ImVec2_c){0, 300}, ImGuiChildFlags_AutoResizeX, 0);

                // Scroll to the appropriate row
                ImVec2_c cursor_start_position = igGetCursorStartPos();
                igSetScrollFromPosY_Float(cursor_start_position.y + (*target_address / MEMORY_VIEWER_COLUMNS) * igGetTextLineHeight(), 0);

                igEndChild();
            }
        }
    }
    igEnd();
}

// Return the cursor's position in screen space
static ImVec2_c get_cursor()
{
    ImVec2_c window = igGetWindowPos();
    ImVec2_c content = igGetCursorStartPos();

    return (ImVec2_c){window.x + content.x, window.y + content.y};
}

// Return the mouse position in the current window's space
static ImVec2_c get_mouse_wrt_window()
{
    ImVec2_c mouse = igGetMousePos();
    ImVec2_c window = igGetWindowPos();
    ImVec2_c content = igGetCursorStartPos();

    return (ImVec2_c){mouse.x - (window.x + content.x), mouse.y - (window.y + content.y)};
}

static void text_on_off(bool state, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    igTextV(format, args);
    va_end(args);

    igSameLine(0, 5);
    igTextColored(state ? color_accent : color_dimmed, state ? "on" : "off");
}

static void plot_metric(Metric* m, char* title)
{
    ImVec2_c size = {0, 40};
    igPlotHistogram_FloatPtr(title, m->values, m->values_length, m->index,
                             NULL, 0, m->average * 2, size, sizeof(float));
}

int ui_draw(Ui* ui, Genesis* genesis, Settings* settings, Audio* audio)
{
    int actions = UiAction_None;

    // TODO wrapp all the igBegin in if, otherwise collapsed windows still render

    bool dummy_flag = false;

    if (igBeginMainMenuBar())
    {
        if (igBeginMenu("Game", true))
        {
            // Snapshots
            if (igBeginMenu("Save slots", true))
            {
                for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
                {
                    igText("Slot %d:", slot);
                    igSameLine(0, 5);

                    if (ui->snapshots[slot] == NULL)
                    {
                        igTextColored(color_dimmed, "       empty       ");
                    }
                    else
                    {
                        char date[30];
                        strftime(date, 30, "%Y-%m-%d %H:%M:%S", localtime(&ui->snapshots[slot]->date));
                        igTextColored(color_accent, "%s", date);
                    }

                    igSameLine(0, 10);

                    char button_name[10];
                    sprintf(button_name, "save##%d", slot);
                    if (igButton(button_name, (ImVec2_c){50, 20}))
                    {
                        ui->snapshots[slot] = snapshot_save(genesis, slot);
                    }

                    igSameLine(0, 3);

                    if (ui->snapshots[slot] == NULL)
                    {
                        igPushStyleColor_Vec4(ImGuiCol_Button, color_disabled);
                        igPushStyleColor_Vec4(ImGuiCol_ButtonHovered, color_disabled);
                        igPushStyleColor_Vec4(ImGuiCol_ButtonActive, color_disabled);
                        igPushStyleColor_Vec4(ImGuiCol_Text, color_black);
                        igButton("load", (ImVec2_c){50, 20});
                        igPopStyleColor(4);
                    }
                    else
                    {
                        sprintf(button_name, "load##%d", slot);
                        if (igButton(button_name, (ImVec2_c){50, 20}))
                        {
                            snapshot_load(genesis, slot);
                        }
                    }
                }
                igEndMenu();
            }
            igSeparator();

            igSliderFloat("Speed", &settings->emulation_speed, 0.2f, 3.0f, "%f", 1.0f);

            if (igMenuItem_Bool("Reset", NULL, false, true))
            {
                genesis_initialize(genesis);
                audio_initialize(audio);
            }

            if (igMenuItem_Bool(genesis->status == Status_Running ? "Pause" : "Resume", glfwGetKeyName(GLFW_KEY_P, 0), false, genesis->status != Status_NoGameLoaded))
            {
                actions |= UiAction_TogglePause;
            }

            if (igMenuItem_Bool("Step", "Space", false, genesis->status == Status_Pause))
            {
                actions |= UiAction_Step;
            }

            igSeparator();
            igMenuItem_BoolPtr("Metrics", NULL, &settings->show_metrics, true);
            igMenuItem_BoolPtr("Rewinding", NULL, &settings->rewinding_enabled, true);

            igEndMenu();
        }

        if (igBeginMenu("CPU", true))
        {
            igMenuItem_BoolPtr("Registers", NULL, &settings->show_m68k_registers, true);
            igMenuItem_BoolPtr("Disassembly", NULL, &settings->show_m68k_disassembly, true);
            igMenuItem_BoolPtr("Log", NULL, &settings->show_m68k_log, true);
            igSeparator();
            igMenuItem_BoolPtr("ROM", NULL, &settings->show_rom, true);
            igMenuItem_BoolPtr("RAM", NULL, &settings->show_ram, true);
            igEndMenu();
        }

        if (igBeginMenu("Video", true))
        {

            igSliderFloat("Scaling", &settings->video_scale, 1.0f, 5.0f, "%f", 1.0f);
            if (igMenuItem_Bool("Full screen", NULL, settings->full_screen, true))
            {
                actions |= UiAction_ToggleFullScreen;
            }
            igSeparator();
            igMenuItem_BoolPtr("Registers", NULL, &settings->show_vdp_registers, true);
            igMenuItem_BoolPtr("Palettes", NULL, &settings->show_vdp_palettes, true);
            igMenuItem_BoolPtr("Patterns", NULL, &settings->show_vdp_patterns, true);
            igMenuItem_BoolPtr("Planes", NULL, &settings->show_vdp_planes, true);
            igMenuItem_BoolPtr("Sprites", NULL, &settings->show_vdp_sprites, true);
            igSeparator();
            igMenuItem_BoolPtr("VRAM", NULL, &settings->show_vram, true);
            igMenuItem_BoolPtr("VSRAM", NULL, &settings->show_vsram, true);
            igMenuItem_BoolPtr("CRAM", NULL, &settings->show_cram, true);
            igEndMenu();
        }

        if (igBeginMenu("Audio", true))
        {
            igMenuItem_BoolPtr("Z80 registers", NULL, &settings->show_z80_registers, true);
            igMenuItem_BoolPtr("Z80 disassembly", NULL, &settings->show_z80_disassembly, true);
            igMenuItem_BoolPtr("Z80 log", NULL, &settings->show_z80_log, true);
            igSeparator();
            igMenuItem_BoolPtr("PSG registers", NULL, &settings->show_psg_registers, true);
            igMenuItem_BoolPtr("YM2612 registers", NULL, &settings->show_ym2612_registers, true);
            igEndMenu();
        }

        igEndMainMenuBar();
    }

    // M68K registers
    if (settings->show_m68k_registers)
    {
        igSetNextWindowSize((ImVec2_c){440, 270}, ImGuiCond_FirstUseEver);
        igBegin("CPU registers", &settings->show_m68k_registers, 0);
        igColumns(2, NULL, true);

        for (int i = 0; i < 8; ++i)
        {
            igText("D%d: %08X", i, genesis->m68k->data_registers[i]);
        }

        igNextColumn();

        for (int i = 0; i < 8; ++i)
        {
            igText("A%d: %08X", i, genesis->m68k->address_registers[i]);
        }

        igColumns(1, NULL, false);
        igSeparator();

        igText("Status: %04X", genesis->m68k->status);

#define STATUS_BIT(label, bit)    \
    igText(label);                \
    igCheckbox("##" label, &bit); \
    igNextColumn()

        bool extended = EXTENDED(genesis->m68k);
        bool negative = NEGATIVE(genesis->m68k);
        bool zero = ZERO(genesis->m68k);
        bool overflow = OVERFLOW(genesis->m68k);
        bool carry = CARRY(genesis->m68k);

        igPushStyleColor_Vec4(ImGuiCol_CheckMark, color_accent);
        igColumns(5, NULL, false);
        STATUS_BIT("X", extended);
        STATUS_BIT("N", negative);
        STATUS_BIT("Z", zero);
        STATUS_BIT("V", overflow);
        STATUS_BIT("C", carry);
        igPopStyleColor(1);

        igColumns(1, NULL, false);
        igSeparator();

        igText("PC:     %08X", genesis->m68k->pc);
        igText("Cycles: %08X", genesis->m68k->cycles);

        igEnd();
    }

    // M68K disassembly
    if (settings->show_m68k_disassembly)
    {
        igSetNextWindowSize((ImVec2_c){520, 320}, ImGuiCond_FirstUseEver);
        igBegin("CPU Disassembly", &settings->show_m68k_disassembly, 0);

        igColumns(4, NULL, false);
        igSetColumnOffset(1, 20);
        igSetColumnOffset(2, 100);

        igNextColumn();
        igText("Address");
        igNextColumn();
        igText("Instruction");
        igNextColumn();
        igText("Opcode");
        igNextColumn();
        igSeparator();

        uint32_t address = genesis->m68k->pc;
        for (int i = 0; i < DISASSEMBLY_LENGTH; ++i)
        {
            DecodedInstruction* instr = m68k_decode(genesis->m68k, address);

            // The memory may not contain valid opcodes, especially after branching instructions
            if (instr == NULL)
            {
                igColumns(1, NULL, false);
                igTextColored(color_dimmed, "Cannot decode opcode at %06X", address);

                // Fill the disassembler with empty lines to keep the same height
                for (int j = i + 1; j < DISASSEMBLY_LENGTH; ++j)
                {
                    igText("");
                }

                break;
            }

            // Draw a bubble on lines with a breakpoint
            if (debugger_get_breakpoint(genesis->debugger, address) != NULL)
            {
                struct ImDrawList* draw_list = igGetWindowDrawList();

                ImVec2_c window = igGetWindowPos();
                ImVec2_c cursor = igGetCursorPos();
                ImVec2_c bubble = (ImVec2_c){window.x + cursor.x + 5, window.y + cursor.y + 7};

                ImDrawList_AddCircleFilled(draw_list, bubble, 4, igGetColorU32_Vec4(color_accent), 32);
            }
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, "%06X", address);
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, instr->mnemonics);

            // Toggle breakpoint when the instruction is clicked
            // TODO would be nice to have a hover feedback
            // TODO would be better to click the whole row but grouping seems to be interrupted by columns :(
            if (igIsItemClicked(0))
            {
                debugger_toggle_breakpoint(genesis->debugger, address);
            }

            igNextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                igTextColored(color_dimmed, "%02X ", m68k_read_b(genesis->m68k, address + byte));
                igSameLine(0, 0);
            }
            igNextColumn();

            address += instr->length;
            decoded_instruction_free(instr);
        }

        igColumns(1, NULL, false);
        igSeparator();

        igText("Breakpoints");

        for (uint8_t i = 0; i < BREAKPOINTS_COUNT; ++i)
        {
            Breakpoint* b = &genesis->debugger->breakpoints[i];

            char name_buffer[100];

            sprintf(name_buffer, "##be%d", i);
            igCheckbox(name_buffer, &b->enabled);

            igSameLine(0, 10);

            sprintf(name_buffer, "##ba%d", i);
            igInputInt(name_buffer, (int*)&b->address, 1, 2, ImGuiInputTextFlags_CharsHexadecimal);
        }

        igEnd();
    }

    // Z80 registers
    if (settings->show_z80_registers)
    {
        Z80* z = genesis->z80;

        igSetNextWindowSize((ImVec2_c){440, 230}, ImGuiCond_FirstUseEver);
        igBegin("Z80 registers", &settings->show_z80_registers, 0);
        igColumns(2, NULL, true);

        igText("AF: %04X", z->af);
        igText("BC: %04X", z->bc);
        igText("DE: %04X", z->de);
        igText("HL: %04X", z->hl);
        igText("IX: %04X", z->ix);
        igText("I:    %02X", z->i);
        igText("R:    %02X", z->r);

        igNextColumn();

        igText("AF': %04X", z->af_);
        igText("BC': %04X", z->bc_);
        igText("DE': %04X", z->de_);
        igText("HL': %04X", z->hl_);
        igText("IY:  %04X", z->iy);
        igText("PC:  %04X", genesis->z80->pc);
        igText("SP:  %04X", genesis->z80->sp);

        igColumns(1, NULL, false);
        igSeparator();

        igPushStyleColor_Vec4(ImGuiCol_CheckMark, color_accent);
        igColumns(2, NULL, false);
        igCheckbox("running", &z->running);
        igNextColumn();
        igCheckbox("resetting", &z->resetting);
        igNextColumn();

        bool sf = z->sf;
        bool zf = z->zf;
        bool yf = z->yf;
        bool hf = z->hf;
        bool xf = z->xf;
        bool pf = z->pf;
        bool nf = z->nf;
        bool cf = z->cf;

        igColumns(8, NULL, false);
        STATUS_BIT("S", sf);
        STATUS_BIT("Z", zf);
        STATUS_BIT("Y", yf);
        STATUS_BIT("H", hf);
        STATUS_BIT("X", xf);
        STATUS_BIT("P", pf);
        STATUS_BIT("N", nf);
        STATUS_BIT("C", cf);
        igPopStyleColor(1);

        igEnd();
    }

    // M68K program log
    if (settings->show_m68k_log)
    {
        igSetNextWindowSize((ImVec2_c){380, 460}, ImGuiCond_FirstUseEver);
        igBegin("CPU log", &settings->show_m68k_log, 0);

        Debugger* d = genesis->debugger;
        for (int i = 0; i < M68K_LOG_LENGTH; ++i)
        {
            uint32_t address = d->m68k_log_addresses[(d->m68k_log_cursor + 1 + i) % M68K_LOG_LENGTH];
            DecodedInstruction* instr = m68k_decode(genesis->m68k, address);
            if (instr != NULL)
            {
                igText("%04X   %s", address, instr->mnemonics);
                decoded_instruction_free(instr);
            }
        }

        igSetScrollHereY(0); // scroll to bottom
        // igButton("clear", )
        igEnd();
    }

    // Z80 disassembly
    if (settings->show_z80_disassembly)
    {
        igSetNextWindowSize((ImVec2_c){400, 230}, ImGuiCond_FirstUseEver);
        igBegin("Z80 Disassembly", &settings->show_z80_disassembly, 0);

        igColumns(4, NULL, false);
        igSetColumnOffset(1, 0);
        igSetColumnOffset(2, 60);
        igSetColumnOffset(3, 250);

        igNextColumn();
        igText("Address");
        igNextColumn();
        igText("Instruction");
        igNextColumn();
        igText("Opcode");
        igNextColumn();
        igSeparator();

        uint32_t address = genesis->z80->pc;
        for (int i = 0; i < DISASSEMBLY_LENGTH; ++i)
        {
            FullyDecodedZ80Instruction* instr = z80_decode(genesis->z80, address);

            // The memory may not contain valid opcodes, especially after branching instructions
            if (instr == NULL)
            {
                igColumns(1, NULL, false);
                igTextColored(color_dimmed, "Cannot decode opcode at %04X", address);

                // Fill the disassembler with empty lines to keep the same height
                for (int j = i + 1; j < DISASSEMBLY_LENGTH; ++j)
                {
                    igText("");
                }

                break;
            }
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, "%04X", address);
            igNextColumn();

            igTextColored(i == 0 ? color_accent : color_white, instr->mnemonics);
            igNextColumn();

            for (int byte = 0; byte < instr->length; ++byte)
            {
                igTextColored(color_dimmed, "%02X ", z80_read(genesis->z80, address + byte));
                igSameLine(0, 0);
            }
            igNextColumn();

            address += instr->length;

            fully_decoded_z80_instruction_free(instr);
        }

        igEnd();
    }

    // Z80 program log
    if (settings->show_z80_log)
    {
        igSetNextWindowSize((ImVec2_c){260, 460}, ImGuiCond_FirstUseEver);
        igBegin("Z80 log", &settings->show_z80_log, 0);

        Debugger* d = genesis->debugger;
        for (int i = 0; i < Z80_LOG_LENGTH; ++i)
        {
            LoggedZ80Instruction* instr = &d->z80_log_instrs[(d->z80_log_cursor + 1 + i) % Z80_LOG_LENGTH];
            igText("%04X   %s", instr->address, instr->mnemonics);
        }

        igSetScrollHereY(0); // scroll to bottom
        igEnd();
    }

    // ROM
    if (settings->show_rom)
    {
        memory_viewer("ROM", &settings->show_rom, genesis->rom, Byte, 0x400000, &ui->rom_target_address);
    }

    // RAM
    if (settings->show_ram)
    {
        memory_viewer("RAM", &settings->show_ram, genesis->ram, Byte, 0x10000, &ui->ram_target_address);
    }

    // VDP registers
    if (settings->show_vdp_registers)
    {
        igSetNextWindowSize((ImVec2_c){720, 330}, ImGuiCond_FirstUseEver);
        igBegin("VDP registers", &settings->show_vdp_registers, 0);
        igColumns(3, NULL, false);

        Vdp* vdp = genesis->vdp;

#define REGISTER_SECTION(reg) igTextColored(color_title, "Register %0X [%02X]", reg, vdp->register_raw_values[reg])

        igBeginChild_Str("column1", vec_zero, ImGuiChildFlags_None, 0);

        REGISTER_SECTION(0);
        igBullet();
        text_on_off(vdp->hblank_interrupt_enabled, "H-blank interrupt:");
        igBullet();
        text_on_off(vdp->hv_counter_latched, "HV-counter latched:");
        igSeparator();

        REGISTER_SECTION(1);
        igBullet();
        text_on_off(vdp->display_enabled, "Display enabled:");
        igBullet();
        text_on_off(vdp->vblank_interrupt_enabled, "V-blank interrupt:");
        igBullet();
        text_on_off(vdp->dma_enabled, "DMA enabled:");
        igBullet();
        igText("Display height: %0X", vdp->display_height);
        igSeparator();

        REGISTER_SECTION(2);
        igBullet();
        igText("Plane A location: %0X", vdp->plane_a_nametable);
        igSeparator();

        REGISTER_SECTION(3);
        igBullet();
        igText("Window location: %0X", vdp->window_nametable);
        igSeparator();

        REGISTER_SECTION(4);
        igBullet();
        igText("Plane B location: %0X", vdp->plane_b_nametable);
        igSeparator();

        REGISTER_SECTION(5);
        igBullet();
        igText("Sprite table location: %0X", vdp->sprites_attribute_table);

        igEndChild();
        igNextColumn();
        igBeginChild_Str("column2", vec_zero, ImGuiChildFlags_None, 0);

        REGISTER_SECTION(7);
        igBullet();
        igText("Background palette: %0X", vdp->background_color_palette);
        igBullet();
        igText("Background color: %0X", vdp->background_color_entry);
        igSeparator();

        REGISTER_SECTION(0xA);
        igBullet();
        igText("H-blank counter: %0X", vdp->hblank_line);
        igSeparator();

        REGISTER_SECTION(0xB);
        char* vertical_scrolling_mode_names[] = {"Screen", "two columns"};
        igBullet();
        igText("Vertical scrolling: %s", vertical_scrolling_mode_names[vdp->vertical_scrolling_mode]);
        char* horizontal_scrolling_mode_names[] = {"Screen", "Invalid", "Row", "Line"};
        igBullet();
        igText("Horizontal scrolling: %s", horizontal_scrolling_mode_names[vdp->horizontal_scrolling_mode]);

        REGISTER_SECTION(0xC);
        igBullet();
        igText("Display width: %0X", vdp->display_width);
        igBullet();
        text_on_off(vdp->shadow_highlight_enabled, "Shadow/Highlight:");
        igBullet();
        igText("Interlace mode: %0X", vdp->interlace_mode);
        igSeparator();

        REGISTER_SECTION(0xD);
        igBullet();
        igText("Horizontal scroll table: %0X", vdp->horizontal_scrolltable);
        igSeparator();

        REGISTER_SECTION(0xF);
        igBullet();
        igText("Auto-increment: %0X", vdp->auto_increment);

        igEndChild();
        igNextColumn();
        igBeginChild_Str("column3", vec_zero, ImGuiChildFlags_None, 0);

        REGISTER_SECTION(0x10);
        igBullet();
        igText("Plane height: %0X", vdp->plane_width);
        igBullet();
        igText("Plane width: %0X", vdp->plane_height);
        igSeparator();

        REGISTER_SECTION(0x11);
        igBullet();
        igText("Window horizontal direction: %s", vdp->window_plane_horizontal_direction ? "left" : "right");
        igBullet();
        igText("Window horizontal position: %0X", vdp->window_plane_horizontal_offset);
        igSeparator();

        REGISTER_SECTION(0x12);
        igBullet();
        igText("Window vertical direction: %s", vdp->window_plane_vertical_direction ? "up" : "down");
        igBullet();
        igText("Window vertical position: %0X", vdp->window_plane_vertical_offset);
        igSeparator();

        REGISTER_SECTION(0x13);
        REGISTER_SECTION(0x14);
        igBullet();
        igText("DMA length: %0X", vdp->dma_length);
        igSeparator();

        REGISTER_SECTION(0x15);
        REGISTER_SECTION(0x16);
        REGISTER_SECTION(0x17);
        igBullet();
        igText("DMA source: %0X", vdp->dma_source_address_hi << 16 | vdp->dma_source_address_lo);

        igEndChild();
        igColumns(1, NULL, false);
        igEnd();
    }

    // VDP palettes
    if (settings->show_vdp_palettes)
    {
        igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP palettes", &settings->show_vdp_palettes, ImGuiWindowFlags_AlwaysAutoResize);

        struct ImDrawList* draw_list = igGetWindowDrawList();

        ImVec2_c window_position = igGetCursorScreenPos();

        ImVec2_c cell_top_left, cell_bottom_right;

        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 16; ++col)
            {
                cell_top_left.x = window_position.x + col * PALETTE_ENTRY_WIDTH;
                cell_top_left.y = window_position.y + row * PALETTE_ENTRY_WIDTH;
                cell_bottom_right.x = cell_top_left.x + PALETTE_ENTRY_WIDTH;
                cell_bottom_right.y = cell_top_left.y + PALETTE_ENTRY_WIDTH;

                Color color = genesis->vdp->cram[row * 16 + col];
                ImDrawList_AddRectFilled(draw_list, cell_top_left, cell_bottom_right, 255u << 24 | color.b << 16 | color.g << 8 | color.r, 0, 0);
            }
        }

        igDummy((ImVec2_c){16 * PALETTE_ENTRY_WIDTH, 4 * PALETTE_ENTRY_WIDTH});

        igEnd();
        igPopStyleVar(1);
    }

    // VDP patterns
    if (settings->show_vdp_patterns)
    {
        uint16_t patterns_width = PATTERNS_COLUMNS * 8;
        uint16_t patterns_height = PATTERNS_COUNT / PATTERNS_COLUMNS * 8;

        igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP patterns", &settings->show_vdp_patterns, ImGuiWindowFlags_AlwaysAutoResize);

        // Update the pattern texture with the VRAM contents

        uint8_t patterns_buffer[PATTERNS_COUNT * 64 * 3]; // n patterns * 64 pixels * 3 color components

        for (int pattern = 0; pattern < PATTERNS_COUNT; ++pattern)
        {
            int x = pattern % PATTERNS_COLUMNS * 8;
            int y = pattern / PATTERNS_COLUMNS * 8;
            vdp_draw_pattern(genesis->vdp, pattern, debug_palette, patterns_buffer, patterns_width, x, y, false, false);
        }

        glBindTexture(GL_TEXTURE_2D, ui->patterns_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, patterns_width, patterns_height, 0, GL_RGB, GL_UNSIGNED_BYTE, patterns_buffer);

        igImageWithBg(texture_ref((ImTextureID)(intptr_t)ui->patterns_texture), (ImVec2_c){patterns_width, patterns_height}, vec_zero, vec_one, color_black, color_white);

        // If a pattern is hovered, show a tooltip with a magnified view
        if (igIsItemHovered(ImGuiHoveredFlags_None))
        {
            ImVec2_c pattern_pos = get_mouse_wrt_window();
            uint16_t pattern_index = (int)pattern_pos.y / 8 * PATTERNS_COLUMNS + (int)pattern_pos.x / 8;

            uint8_t magnified_pattern_buffer[64 * 3];
            vdp_draw_pattern(genesis->vdp, pattern_index, debug_palette, magnified_pattern_buffer, 8, 0, 0, false, false);

            glBindTexture(GL_TEXTURE_2D, ui->magnified_pattern_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, magnified_pattern_buffer);

            igBeginTooltip();
            igText("Pattern #%d", pattern_index);
            igImageWithBg(texture_ref((ImTextureID)(intptr_t)ui->magnified_pattern_texture), (ImVec2_c){8 * PATTERN_MAGNIFICATION_FACTOR, 8 * PATTERN_MAGNIFICATION_FACTOR}, vec_zero, vec_one, color_white, color_white);
            igEndTooltip();
        }

        igEnd();
        igPopStyleVar(1);
    }

    // VDP planes
    if (settings->show_vdp_planes)
    {
        uint16_t plane_width = 64 * 8;
        uint16_t plane_height = 64 * 8;

        igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP planes", &settings->show_vdp_planes, ImGuiWindowFlags_AlwaysAutoResize);

        // Update the plane texture with the selected plane

        memset(ui->plane_buffer, 0, 64 * 8 * 64 * 8 * 3 * sizeof(uint8_t));
        vdp_draw_plane(genesis->vdp, ui->selected_plane, ui->plane_buffer, 512);

        glBindTexture(GL_TEXTURE_2D, ui->planes_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, plane_width, plane_height, 0, GL_RGB, GL_UNSIGNED_BYTE, ui->plane_buffer);

        igImageWithBg(texture_ref((ImTextureID)(intptr_t)ui->planes_texture), (ImVec2_c){plane_width, plane_height}, vec_zero, vec_one, color_black, color_white);

        // If a cell is hovered, show a tooltip with details
        if (igIsItemHovered(ImGuiHoveredFlags_None))
        {
            // Get the data of the hovered cell

            ImVec2_c cell_pos = get_mouse_wrt_window();
            uint16_t cell_index = (int)cell_pos.y / 8 * genesis->vdp->plane_height + (int)cell_pos.x / 8;

            uint16_t pattern_index, palette_index;
            bool priority, horizontal_flip, vertical_flip;
            vdp_get_plane_cell_data(genesis->vdp, ui->selected_plane, cell_index, &pattern_index, &palette_index, &priority, &horizontal_flip, &vertical_flip);

            // Draw a border around the hovered cell

            ImVec2_c window = get_cursor();
            ImVec2_c top_left = {window.x + (int)(cell_pos.x / 8) * 8 + 1, window.y + (int)(cell_pos.y / 8) * 8 + 1};
            ImVec2_c bottom_right = {top_left.x + 8, top_left.y + 8};

            struct ImDrawList* draw_list = igGetWindowDrawList();
            ImDrawList_AddRect(draw_list, top_left, bottom_right, igGetColorU32_Vec4(color_accent), 0, 0, 1);

            // Draw a magnified version of the pattern

            Color* palette = genesis->vdp->cram + palette_index * 16;

            uint8_t magnified_pattern_buffer[64 * 3];
            vdp_draw_pattern(genesis->vdp, pattern_index, palette, magnified_pattern_buffer, 8, 0, 0, horizontal_flip, vertical_flip);

            glBindTexture(GL_TEXTURE_2D, ui->magnified_pattern_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, magnified_pattern_buffer);

            igBeginTooltip();
            igText("Cell (%d, %d)", (int)cell_pos.x / 8, (int)cell_pos.y / 8);
            igText("Pattern #%d", pattern_index);

            igText("%-18s", "Horizontal flip: ");
            igSameLine(0, 0);
            igTextColored(horizontal_flip ? color_accent : color_dimmed, horizontal_flip ? "on" : "off");

            igText("%-18s", "Vertical flip: ");
            igSameLine(0, 0);
            igTextColored(vertical_flip ? color_accent : color_dimmed, vertical_flip ? "on" : "off");

            igText("%-18s", "Priority: ");
            igSameLine(0, 0);
            igTextColored(priority ? color_accent : color_dimmed, priority ? "on" : "off");

            igImageWithBg(texture_ref((ImTextureID)(intptr_t)ui->magnified_pattern_texture), (ImVec2_c){8 * PATTERN_MAGNIFICATION_FACTOR, 8 * PATTERN_MAGNIFICATION_FACTOR}, vec_zero, vec_one, color_white, color_white);
            igEndTooltip();
        }

        igColumns(3, NULL, false);
        igRadioButton_IntPtr("Plane A", (int*)&ui->selected_plane, Plane_A);
        igNextColumn();
        igRadioButton_IntPtr("Plane B", (int*)&ui->selected_plane, Plane_B);
        igNextColumn();
        igRadioButton_IntPtr("Window", (int*)&ui->selected_plane, Plane_Window);
        igColumns(1, NULL, false);

        igEnd();
        igPopStyleVar(1);
    }

    // VDP sprites
    if (settings->show_vdp_sprites)
    {
        igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, vec_zero);
        igBegin("VDP sprites", &settings->show_vdp_sprites, ImGuiWindowFlags_AlwaysAutoResize);

        ImVec2_c pos = get_cursor();

        // Update the sprite texture

        memset(ui->sprites_buffer, 0, 552 * 552 * 3 * sizeof(uint8_t));
        vdp_draw_sprites(genesis->vdp, ui->sprites_buffer, 552);

        glBindTexture(GL_TEXTURE_2D, ui->sprites_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 552, 552, 0, GL_RGB, GL_UNSIGNED_BYTE, ui->sprites_buffer);

        igImageWithBg(texture_ref((ImTextureID)(intptr_t)ui->sprites_texture), (ImVec2_c){552, 552}, vec_zero, vec_one, color_black, color_white);

        // Draw the screen border

        ImVec2_c a = {pos.x + 128, pos.y + 128};
        ImVec2_c b = {a.x + genesis->vdp->display_width * 8, a.y + genesis->vdp->display_height * 8};

        struct ImDrawList* draw_list = igGetWindowDrawList();
        ImDrawList_AddRect(draw_list, a, b, igGetColorU32_Vec4(color_accent), 0, 0, 1);

        igEnd();
        igPopStyleVar(1);
    }

    // VRAM
    if (settings->show_vram)
    {
        memory_viewer("VRAM", &settings->show_vram, genesis->vdp->vram, Byte, 0x10000, NULL);
    }

    // VSRAM
    if (settings->show_vsram)
    {
        memory_viewer("VSRAM", &settings->show_vsram, genesis->vdp->vsram, Word, 0x40, NULL);
    }

    // CRAM
    if (settings->show_cram)
    {
        // Because we do not store raw CRAM data in the VDP implementation, convert
        // the decoded colors back to words for the debug view
        uint16_t raw_cram[0x40];
        for (int c = 0; c < 0x40; ++c)
        {
            raw_cram[c] = COLOR_STRUCT_TO_11(genesis->vdp->cram[c]);
        }

        memory_viewer("CRAM", &settings->show_cram, raw_cram, Word, 0x40, NULL);
    }

    // Status bubble when the emulation is paused/rewinding
    // TODO would be better to always keep this overlay on top of the other windows but I don't think it's currently possible
    if (genesis->status == Status_Pause || genesis->status == Status_Rewinding)
    {
        igSetNextWindowPos((ImVec2_c){10, 50}, ImGuiCond_None, (ImVec2_c){0, 0});
        igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4_c){1.0f, 1.0f, 1.0f, 0.10f});
        igBegin("Example: Fixed Overlay", &dummy_flag, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        igTextColored(color_accent, genesis->status == Status_Pause ? "Paused" : "Rewinding");
        igEnd();
        igPopStyleColor(1);
    }

    double now = glfwGetTime();
    double dt = now - ui->last_time;
    ui->last_time = now;

    metric_push(ui->tpf, dt * 1000);
    metric_push(ui->audio_buffer_queue, audio_queued_samples(audio));

    ui->metrics_refresh_counter += dt;
    if (ui->metrics_refresh_counter > 1)
    {
        ui->metrics_refresh_counter = 0;
        metric_average(ui->tpf);
        metric_average(ui->audio_buffer_queue);
    }

    if (settings->show_metrics)
    {
        igSetNextWindowSize((ImVec2_c){620, 230}, ImGuiCond_FirstUseEver);
        igBegin("Metrics", &settings->show_metrics, 0);
        char buf[50];

        // Time per frame, in milliseconds
        snprintf(buf, sizeof(buf), "tpf (ms)\navg: %.2f\nfps: %.2f",
                 ui->tpf->average, ui->tpf->average > 0 ? 1000.0 / ui->tpf->average : 0);
        plot_metric(ui->tpf, buf);

        // (M68k) instructions per frame, in kilos
        snprintf(buf, sizeof buf, "audio queue (samples)\navg: %.2f", ui->audio_buffer_queue->average);
        plot_metric(ui->audio_buffer_queue, buf);

        igTextColored(color_title, "Remaining audio time: ");
        igSameLine(0, 0);
        igText("%.6fms", audio->remaining_time * 1000);

        igTextColored(color_title, "Remaining master cycles");
        igText("M68k:   %d", genesis->m68k->remaining_master_cycles);
        igText("Z80:    %d", genesis->z80->remaining_master_cycles);
        igText("PSG:    %d", genesis->psg->remaining_master_cycles);
        igText("YM2612: %d", genesis->ym2612->remaining_master_cycles);

        igEnd();
    }

    // PSG registers
    if (settings->show_psg_registers)
    {
        igBegin("PSG registers", &settings->show_psg_registers, ImGuiWindowFlags_AlwaysAutoResize);

        PSG* p = genesis->psg;

        for (int i = 0; i < 3; ++i)
        {
            igTextColored(color_title, "Square %d", i);
            if (p->square[i].volume == 0xF)
            {
                igSameLine(0, 0);
                igTextColored(color_dimmed, " [silent]");
            }
            igText("volume:  %0X", p->square[i].volume);
            igText("tone:    %0X [%.2fHz]", p->square[i].tone, square_tone_in_hertz(&p->square[i]));
            igText("counter: %0X", p->square[i].counter);
            igText("output:  %d", square_output(&p->square[i]));
        }

        igTextColored(color_title, "Noise");
        if (p->noise.volume == 0xF)
        {
            igSameLine(0, 0);
            igTextColored(color_dimmed, " [silent]");
        }
        igText("volume:     %0X", p->noise.volume);
        igText("mode:       %0X [%s]", p->noise.mode, p->noise.mode ? "white" : "periodic");
        igText("shift rate: %0X", p->noise.shift_rate);
        igText("counter:    %0X", p->noise.counter);
        igText("lfsr:       %0X", p->noise.lfsr);
        igText("output:     %d", noise_output(&p->noise));
        igEnd();
    }

    // YM2612 registers
    if (settings->show_ym2612_registers)
    {
        igSetNextWindowSize((ImVec2_c){700, 360}, ImGuiCond_FirstUseEver);
        igBegin("YM2612 registers", &settings->show_ym2612_registers, 0);

        YM2612* y = genesis->ym2612;

        // Global registers
        igTextColored(color_title, "Global registers");
        igColumns(3, NULL, false);

        igText("LFO enabled:   %s", y->lfo_enabled ? "true" : "false");
        igText("LFO frequency: %0X", y->lfo_frequency_index);

        igNextColumn();

        igText("timer A: %0X", y->timer_a);
        igText("timer B: %0X", y->timer_b);

        igNextColumn();

        igText("DAC enabled: %0X", y->dac_enabled);
        igText("DAC data:    %0X", y->dac_data);

        igSeparator();

        // Channels

        igColumns(7, NULL, false);

        ImVec2_c dummy = {0, 0}; // Don't know what it does, but needed
                                 // as argument to IgSelectable
        igTextColored(color_title, "Channel");
        igSelectable_Bool("enabled", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("freq number", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("frequency", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("feedback", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("algorithm", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("stereo", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("amp modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
        igSelectable_Bool("freq modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);

        igNextColumn();

        for (int i = 0; i < 6; ++i)
        {
            igTextColored(color_title, "%d", i + 1);

            // FIXME: the checkbox is too tall for the line, shifting all the
            // lines in the table after it.
            igPushStyleColor_Vec4(ImGuiCol_CheckMark, color_accent);
            igSameLine(20, 0);
            char name_buffer[10];
            sprintf(name_buffer, "##chan%d", i);
            igCheckbox(name_buffer, &y->channels[i].muted);
            igPopStyleColor(1);

            if ((i + 1) % 3 == 0)
            {
                uint8_t channel_mode = i == 3 ? y->channel3_mode : y->channel6_mode;

                char* mode_string = "normal";
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

                igSameLine(0, 0);
                igText(" (%s)", mode_string);
            }

            igText("%d", y->channels[i].enabled);
            igText("%d:%d", y->channels[i].frequency.block, y->channels[i].frequency.freq);
            igText("%.2fHz", channel_frequency_in_hertz(&y->channels[i], genesis_master_frequency(genesis)));
            igText("%0X", y->channels[i].feedback);
            igText("%0X", y->channels[i].algorithm);
            igText("%s%s", y->channels[i].left_output ? "L" : " ", y->channels[i].right_output ? "R" : " ");
            igText("%0X", y->channels[i].amplitude_modulation_sensitivity);
            igText("%0X", y->channels[i].frequency_modulation_sensitivity);

            igNextColumn();
        }

        igSeparator();

        // Operators
        for (int i = 0; i < 6; ++i)
        {
            igColumns(1, NULL, false);
            if (igTreeNode_Ptr((void*)(intptr_t)i, "Operators for channel %d", i + 1))
            {

                igColumns(5, NULL, false);

                igTextColored(color_title, "Operator");
                igSelectable_Bool("detune", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("multiple", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("total level", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("attack rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("decay rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("sustain level", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("sustain rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("release rate", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("rate scaling", false, ImGuiSelectableFlags_SpanAllColumns, dummy);
                igSelectable_Bool("amp modulation", false, ImGuiSelectableFlags_SpanAllColumns, dummy);

                igNextColumn();

                for (int j = 0; j < 4; ++j)
                {
                    igTextColored(color_title, "%d", j + 1);

                    igText("%0X", y->channels[i].operators[j].detune);
                    igText("%0X", y->channels[i].operators[j].multiple);
                    igText("%0X", y->channels[i].operators[j].total_level);
                    igText("%0X", y->channels[i].operators[j].attack_rate);
                    igText("%0X", y->channels[i].operators[j].decay_rate);
                    igText("%0X", y->channels[i].operators[j].sustain_level);
                    igText("%0X", y->channels[i].operators[j].sustain_rate);
                    igText("%0X", y->channels[i].operators[j].release_rate);
                    igText("%0X", y->channels[i].operators[j].rate_scaling);
                    igText("%0X", y->channels[i].operators[j].amplitude_modulation_enabled);

                    igNextColumn();
                }

                igTreePop();
            }
        }

        igEnd();
    }

    // bool a = true;
    // igShowTestWindow(&a);

    return actions;
}

Ui* ui_make()
{
    Ui* ui = calloc(1, sizeof(Ui));
    ui->plane_buffer = calloc(64 * 8 * 64 * 8 * 3, sizeof(uint8_t));
    ui->sprites_buffer = calloc(552 * 552 * 3, sizeof(uint8_t));

    ui->tpf = metric_make(128);
    ui->audio_buffer_queue = metric_make(128);

    igGetStyle()->WindowRounding = 0.0f;

    gen_texture(&ui->patterns_texture);
    gen_texture(&ui->magnified_pattern_texture);
    gen_texture(&ui->planes_texture);
    gen_texture(&ui->sprites_texture);

    return ui;
}

void ui_free(Ui* ui)
{
    if (ui == NULL)
    {
        return;
    }

    glDeleteTextures(1, &ui->patterns_texture);
    glDeleteTextures(1, &ui->magnified_pattern_texture);
    glDeleteTextures(1, &ui->planes_texture);
    glDeleteTextures(1, &ui->sprites_texture);

    free(ui->plane_buffer);
    free(ui->sprites_buffer);
    metric_free(ui->tpf);
    metric_free(ui->audio_buffer_queue);

    for (int i = 0; i < SNAPSHOT_SLOTS; ++i)
    {
        snapshot_metadata_free(ui->snapshots[i]);
    }

    free(ui);
}

void ui_load_snapshots(Ui* ui, Genesis* genesis)
{
    for (int i = 0; i < SNAPSHOT_SLOTS; ++i)
    {
        snapshot_metadata_free(ui->snapshots[i]);
    }

    snapshots_preload(genesis, ui->snapshots);
}
