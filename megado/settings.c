#include <stdlib.h>
#include <stdio.h>
#include <json.h>

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
    free(s);
}

#define JSON_SET_FLOAT(name) json_object_object_add(json, #name, json_object_new_double((float)s-> ## name));
#define JSON_SET_BOOL(name) json_object_object_add(json, #name, json_object_new_boolean(s-> ## name));

void settings_save(Settings* s)
{
    json_object* json = json_object_new_object();

    json_object_object_add(json, "version", json_object_new_int(SETTINGS_FORMAT_VERSION));

    JSON_SET_FLOAT(video_scale);
    JSON_SET_BOOL(vsync);

    JSON_SET_BOOL(show_cpu_registers);
    JSON_SET_BOOL(show_cpu_disassembly);
    JSON_SET_BOOL(show_vdp_registers);
    JSON_SET_BOOL(show_vdp_palettes);
    JSON_SET_BOOL(show_vdp_patterns);
    JSON_SET_BOOL(show_vdp_planes);
    JSON_SET_BOOL(show_vdp_sprites);
    JSON_SET_BOOL(show_rom);
    JSON_SET_BOOL(show_ram);
    JSON_SET_BOOL(show_vram);

    // TODO breakpoints

    printf("Saving settings...\n");
    FILE* file = fopen(SETTINGS_FILE, "w");
    if (!file)
    {
        printf("Cannot open file \"%s\"", SETTINGS_FILE);
        return;
    }

    const char* json_string = json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED);
    fputs(json_string, file);
    fclose(file);
}

#define JSON_GET_FLOAT(name) s-> ## name = (float) json_object_get_double(json_get(json, #name))
#define JSON_GET_BOOL(name) s-> ## name = json_object_get_boolean(json_get(json, #name))

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
        printf("Cannot open file \"%s\"", SETTINGS_FILE);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    char* buffer = calloc(file_length, sizeof(char));
    fseek(file, 0, SEEK_SET);
    fread(buffer, sizeof(char), file_length, file);
    fclose(file);

    json_object* json = json_tokener_parse(buffer);
    free(buffer);

    // Check the version number
    int version = json_object_get_int(json_object_object_get(json, "version"));
    if (version != SETTINGS_FORMAT_VERSION)
    {
        printf("Settings are out of date (loaded version is %d, current version is %d)", version, SETTINGS_FORMAT_VERSION);
        return NULL;
    }

    Settings* s = calloc(1, sizeof(Settings));

    JSON_GET_FLOAT(video_scale);
    JSON_GET_BOOL(vsync);

    JSON_GET_BOOL(show_cpu_registers);
    JSON_GET_BOOL(show_cpu_disassembly);
    JSON_GET_BOOL(show_vdp_registers);
    JSON_GET_BOOL(show_vdp_palettes);
    JSON_GET_BOOL(show_vdp_patterns);
    JSON_GET_BOOL(show_vdp_planes);
    JSON_GET_BOOL(show_vdp_sprites);
    JSON_GET_BOOL(show_rom);
    JSON_GET_BOOL(show_ram);
    JSON_GET_BOOL(show_vram);

    return s;
}
