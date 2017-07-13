#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"

Settings* settings_make()
{
    // Try to load previous settings
    Settings* s = settings_load();
    if (s != NULL)
        return s;

    // If not possible, use default settings
    s = calloc(1, sizeof(Settings));
    s->video_scale = 1.0f;
    s->vsync = true;
    return s;
}

void settings_free(Settings* s)
{
    free(s->breakpoint_sets);
    free(s);
}

#define JSON_SET_FLOAT(name) json_object_object_add(json, #name, json_object_new_double((float)s->name));
#define JSON_SET_BOOL(name) json_object_object_add(json, #name, json_object_new_boolean(s->name));

void settings_save(Settings* s)
{
    json_object* json = json_object_new_object();

    json_object_object_add(json, "version", json_object_new_int(SETTINGS_FORMAT_VERSION));

    JSON_SET_FLOAT(video_scale);
    JSON_SET_BOOL(vsync);

    JSON_SET_BOOL(show_metrics);
    JSON_SET_BOOL(show_m68k_registers);
    JSON_SET_BOOL(show_m68k_disassembly);
    JSON_SET_BOOL(show_m68k_log);
    JSON_SET_BOOL(show_z80_registers);
    JSON_SET_BOOL(show_z80_disassembly);
    JSON_SET_BOOL(show_z80_log);
    JSON_SET_BOOL(show_vdp_registers);
    JSON_SET_BOOL(show_vdp_palettes);
    JSON_SET_BOOL(show_vdp_patterns);
    JSON_SET_BOOL(show_vdp_planes);
    JSON_SET_BOOL(show_vdp_sprites);
    JSON_SET_BOOL(show_rom);
    JSON_SET_BOOL(show_ram);
    JSON_SET_BOOL(show_vram);
    JSON_SET_BOOL(show_metrics);
    JSON_SET_BOOL(rewinding_enabled);

    // Save the breakpoints
    json_object* json_sets = json_object_new_array();
    for (int i = 0; i < s->breakpoint_sets_length; ++i)
    {
        json_object* json_set = json_object_new_object();
        json_object_object_add(json_set, "game", json_object_new_string(s->breakpoint_sets[i].game));

        json_object* json_breakpoints = json_object_new_array();
        for (int j = 0; j < BREAKPOINTS_COUNT; ++j)
        {
            json_object* json_breakpoint = json_object_new_object();
            json_object_object_add(json_breakpoint, "enabled", json_object_new_boolean(s->breakpoint_sets[i].breakpoints[j].enabled));
            json_object_object_add(json_breakpoint, "address", json_object_new_int(s->breakpoint_sets[i].breakpoints[j].address));
            json_object_array_put_idx(json_breakpoints, j, json_breakpoint);
        }
        json_object_object_add(json_set, "breakpoints", json_breakpoints);

        json_object_array_put_idx(json_sets, i, json_set);
    }
    json_object_object_add(json, "breakpoint_sets", json_sets);

    printf("Saving settings...\n");
    FILE* file = fopen(SETTINGS_FILE, "w");
    if (!file)
    {
        printf("Cannot open file \"%s\"\n", SETTINGS_FILE);
        return;
    }

    const char* json_string = json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED);
    fputs(json_string, file);
    fclose(file);

    json_object_put(json);
}

#define JSON_GET_FLOAT(name) s->name = (float) json_object_get_double(json_get(json, #name))
#define JSON_GET_BOOL(name) s->name = json_object_get_boolean(json_get(json, #name))

json_object* json_get(json_object* json, const char* key)
{
    json_object* obj = json_object_object_get(json, key);
    if (obj == NULL)
        printf("Cannot find key \"%s\"\n", key);
    return obj;
}

Settings* settings_load()
{
    printf("Loading settings...\n");
    FILE* file = fopen(SETTINGS_FILE, "r");
    if (!file)
    {
        printf("Cannot open file \"%s\"\n", SETTINGS_FILE);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    char* buffer = calloc(file_length + 1, sizeof(char));
    fseek(file, 0, SEEK_SET);
    fread(buffer, sizeof(char), file_length, file);
    buffer[file_length] = '\0';
    fclose(file);

    json_object* json = json_tokener_parse(buffer);
    free(buffer);

    // Check the version number
    int version = json_object_get_int(json_object_object_get(json, "version"));
    if (version != SETTINGS_FORMAT_VERSION)
    {
        printf("Settings are out of date (loaded version is %d, current version is %d)\n", version, SETTINGS_FORMAT_VERSION);
        return NULL;
    }

    Settings* s = calloc(1, sizeof(Settings));

    JSON_GET_FLOAT(video_scale);
    JSON_GET_BOOL(vsync);

    JSON_GET_BOOL(show_metrics);
    JSON_GET_BOOL(show_m68k_registers);
    JSON_GET_BOOL(show_m68k_disassembly);
    JSON_GET_BOOL(show_m68k_log);
    JSON_GET_BOOL(show_z80_registers);
    JSON_GET_BOOL(show_z80_disassembly);
    JSON_GET_BOOL(show_z80_log);
    JSON_GET_BOOL(show_vdp_registers);
    JSON_GET_BOOL(show_vdp_palettes);
    JSON_GET_BOOL(show_vdp_patterns);
    JSON_GET_BOOL(show_vdp_planes);
    JSON_GET_BOOL(show_vdp_sprites);
    JSON_GET_BOOL(show_rom);
    JSON_GET_BOOL(show_ram);
    JSON_GET_BOOL(show_vram);
    JSON_GET_BOOL(show_metrics);
    JSON_GET_BOOL(rewinding_enabled);

    // Load the breakpoints

    json_object* json_sets = json_get(json, "breakpoint_sets");
    if (json_sets != NULL)
    {
        s->breakpoint_sets_length = json_object_array_length(json_sets);
        s->breakpoint_sets = calloc(s->breakpoint_sets_length, sizeof(BreakpointSet));

        for (int i = 0; i < s->breakpoint_sets_length; ++i)
        {
            json_object* json_set = json_object_array_get_idx(json_sets, i);
            strcpy(s->breakpoint_sets[i].game, json_object_get_string(json_get(json_set, "game")));

            json_object* json_breakpoints = json_get(json_set, "breakpoints");
            if (json_breakpoints != NULL)
            {
                int breakpoints_length = json_object_array_length(json_breakpoints);
                for (int j = 0; j < breakpoints_length && j < BREAKPOINTS_COUNT; ++j)
                {
                    json_object* json_breakpoint = json_object_array_get_idx(json_breakpoints, j);;
                    s->breakpoint_sets[i].breakpoints[j].enabled = json_object_get_boolean(json_get(json_breakpoint, "enabled"));
                    s->breakpoint_sets[i].breakpoints[j].address = json_object_get_int(json_get(json_breakpoint, "address"));
                }
            }
        }
    }

    json_object_put(json);

    return s;
}

Breakpoint* settings_get_or_create_breakpoints(Settings* s, char* game)
{
    // Look for breakpoints for this game in the settings
    for (int i = 0; i < s->breakpoint_sets_length; ++i)
        if (strcmp(game, s->breakpoint_sets[i].game) == 0)
            return s->breakpoint_sets[i].breakpoints;

    // If there are none, create a new set
    s->breakpoint_sets_length++;
    s->breakpoint_sets = realloc(s->breakpoint_sets, s->breakpoint_sets_length * sizeof(BreakpointSet));

    // Initialize the new set
    strcpy(s->breakpoint_sets[s->breakpoint_sets_length - 1].game, game);
    memset(s->breakpoint_sets[s->breakpoint_sets_length - 1].breakpoints, 0, BREAKPOINTS_COUNT * sizeof(Breakpoint));

    return s->breakpoint_sets[s->breakpoint_sets_length - 1].breakpoints;
}
