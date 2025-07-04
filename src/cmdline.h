#pragma once

#include "region/region.h"
#include <stdint.h>

typedef struct Regions Regions;

typedef enum {

    NONE = 0,
    EFFECT_ONE = 1,
    EFFECT_TWO = 2,
    EFFECT_THREE = 3

} EffectType;

typedef struct Config {

    Regions *region_data;
    EffectType effect_id;

    float scale_factor;
    uint8_t *buffer;

    char *input_file;
    char *output_file;

} Config;

int parse_cmdline(int argc, char **argv, Config *data);
int validate_arguments(const Config *data);
char *get_filter_name(EffectType id);