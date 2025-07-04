#pragma once

#include "cmdline.h"

typedef struct Config Config;

void apply_effect_1(uint8_t *pixel, const int linesize, const int width, const int height, Config *data);

void apply_effect_2(uint8_t *pixel, const int linesize, const int width, const int height, Config *data);

void apply_effect_3(uint8_t *pixel, const int linesize, const int width, const int height, Config *data);