#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Config Config;

typedef struct Pixel {
    unsigned short x;
    unsigned short y;
} Pixel;

typedef struct Region {
    Pixel start;
    Pixel end;
    unsigned short width;
    unsigned short height;
} Region;

typedef struct RegionPair {
    Region one;
    Region two;
} RegionPair;

typedef struct Regions {
    RegionPair *region_pair;
    int size;
} Regions;

// Region management functions
void push(Regions *region_data, const bool isPair, unsigned short width, unsigned short height,
    unsigned short start_x1, unsigned short start_y1, unsigned short end_x1, unsigned short end_y1,
    unsigned short start_x2, unsigned short start_y2, unsigned short end_x2, unsigned short end_y2);

void pop(Regions *region_data);

RegionPair* top(const Regions *region_data);

bool is_empty(const Regions *region_data);

void cleanup_regions(Regions *region_data);

static void allocate_buffer(Config *data, const size_t buffer_size);
static void copy_region_pixels(uint8_t *buffer, const uint8_t *pixel, const int linesize, const Pixel *region_start,
                                   const Pixel *region_end);

// Region manipulation functions
void swap_pixels(Config* data, uint8_t *pixel, int linesize, const Pixel *region1_start, const Pixel *region1_end,
    const Pixel *region2_start, const Pixel *region2_end);

void scale_pixels(Config *data, uint8_t *pixel, int linesize, const float scale_factor, const Pixel *region_start,
                  const Pixel *region_end);

void move_pixels(Config *data, uint8_t *pixel, int linesize, const int width, const int height,
                 const Pixel *region_start, const Pixel *region_end);

static bool overlap(unsigned short start_x1, unsigned short start_y1, unsigned short end_x1, unsigned short end_y1,
    unsigned short start_x2, unsigned short start_y2, unsigned short end_x2, unsigned short end_y2);

static void get_random_move_val(int *move_x, int *move_y);

static void get_random_dimensions(const int width, const int height, unsigned short *region_width,
                                         unsigned short *region_height);

static void select_random_operation(Regions *region_data, bool isPair, unsigned short region_width,
                                    unsigned short region_height, unsigned short start_x1,
                                    unsigned short start_y1, unsigned short end_x1, unsigned short end_y1,
                                    unsigned short start_x2, unsigned short start_y2, unsigned short end_x2,
                                    unsigned short end_y2);

void randomize(Regions *region_data, int width, int height);

void randomize_single_region(Regions *region_data, const int width, const int height);