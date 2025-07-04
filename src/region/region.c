#include "region.h"
#include "cmdline.h"
#include "video-effects.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void push(Regions *region_data, const bool isPair, const unsigned short width, const unsigned short height,
          const unsigned short start_x1, const unsigned short start_y1, const unsigned short end_x1,
          const unsigned short end_y1, const unsigned short start_x2, const unsigned short start_y2,
          const unsigned short end_x2, const unsigned short end_y2) {
    RegionPair *buffer;
    if (is_empty(region_data)) {
        buffer = malloc(sizeof(RegionPair));
    } else {
        buffer = realloc(region_data->region_pair, sizeof(RegionPair) * (region_data->size + 1));
    }

    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    region_data->region_pair = buffer;

    RegionPair *new_pair = &region_data->region_pair[region_data->size];

    if (isPair) {
        new_pair->one.width = width;
        new_pair->one.height = height;
        new_pair->one.start.x= start_x1;
        new_pair->one.start.y = start_y1;
        new_pair->one.end.x = end_x1;
        new_pair->one.end.y = end_y1;

        new_pair->two.width = width;
        new_pair->two.height = height;
        new_pair->two.start.x = start_x2;
        new_pair->two.start.y = start_y2;
        new_pair->two.end.x = end_x2;
        new_pair->two.end.y = end_y2;
    } else {
        new_pair->one.width = width;
        new_pair->one.height = height;
        new_pair->one.start.x= start_x1;
        new_pair->one.start.y = start_y1;
        new_pair->one.end.x = end_x1;
        new_pair->one.end.y = end_y1;
    }

    region_data->size++;
}

void pop(Regions *region_data) {
    if (is_empty(region_data)) {
        fprintf(stderr, "[ERROR] No regions to remove.\n");
        exit(EXIT_FAILURE);
    }
    region_data->size--;

    if (region_data->size > 0) {
        RegionPair *buffer = realloc(region_data->region_pair, sizeof(RegionPair) * (region_data->size));
        if (buffer == NULL) {
            fprintf(stderr, "[ERROR] Failed to allocate memory.\n");
            exit(EXIT_FAILURE);
        }
        region_data->region_pair = buffer;
    } else {
        free(region_data->region_pair);
        region_data->region_pair = NULL;
    }
}

RegionPair* top(const Regions *region_data) {
    if (is_empty(region_data)) {
        fprintf(stderr, "[ERROR] No regions available.\n");
        exit(EXIT_FAILURE);
    }

    return &region_data->region_pair[region_data->size - 1];
}

bool is_empty(const Regions *region_data) {
    return (region_data->size == 0);
}

void cleanup_regions(Regions *region_data) {
    if (region_data) {
        free(region_data->region_pair);
        region_data->region_pair = NULL;
        region_data->size = 0;
    }
}

static void allocate_buffer(Config *data, const size_t buffer_size) {

    if (data->buffer == NULL) {
        data->buffer = malloc(buffer_size);
    } else {
        uint8_t *new_buffer = realloc(data->buffer, buffer_size);
        if (new_buffer == NULL) {
            fprintf(stderr, "[ERROR] Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        data->buffer = new_buffer;
    }

}

static void copy_region_pixels(uint8_t *buffer, const uint8_t *pixel, const int linesize, const Pixel *region_start,
                                   const Pixel *region_end) {

    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Buffer is NULL\n");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    for (unsigned short y = region_start->y; y < region_end->y; y++) {
        for (unsigned short x = region_start->x; x < region_end->x; x++) {
            const int offset = (y * linesize) + (x * 3);
            buffer[i++] = pixel[offset];
            buffer[i++] = pixel[offset + 1];
            buffer[i++] = pixel[offset + 2];
        }
    }

}

void swap_pixels(Config *data, uint8_t *pixel, const int linesize, const Pixel *region1_start, const Pixel *region1_end,
                 const Pixel *region2_start, const Pixel *region2_end) {

    const unsigned short region1_width = region1_end->x - region1_start->x;
    const unsigned short region1_height = region1_end->y - region1_start->y;

    const size_t buffer_size = region1_width * region1_height * 3;

    allocate_buffer(data, buffer_size);

    copy_region_pixels(data->buffer, pixel, linesize, region1_start, region1_end);

    // Swap the pixels between the two regions
    for (unsigned short y = region1_start->y; y < region1_end->y; y++) {
        const unsigned short rel_pos_y = y - region1_start->y;
        for (unsigned short x = region1_start->x; x < region1_end->x; x++) {
            const unsigned short rel_pos_x = x - region1_start->x;

            const int offset_region1 = (y * linesize) + (x * 3);
            const int offset_region2 = ((region2_start->y + rel_pos_y) * linesize) + (
                                           (region2_start->x + rel_pos_x) * 3);

            pixel[offset_region1] = pixel[offset_region2];
            pixel[offset_region1 + 1] = pixel[offset_region2 + 1];
            pixel[offset_region1 + 2] = pixel[offset_region2 + 2];
        }
    }

    int i = 0;

    // Write the pixel data from the buffer back to the second region
    for (unsigned short y = region2_start->y; y < region2_end->y; y++) {
        for (unsigned short x = region2_start->x; x < region2_end->x; x++) {
            const int offset = (y * linesize) + (x * 3);

            pixel[offset] = data->buffer[i++];
            pixel[offset + 1] = data->buffer[i++];
            pixel[offset + 2] = data->buffer[i++];
        }
    }

}

// Nearest Neighbor Interpolation, algorithm from https://kwojcicki.github.io/blog/NEAREST-NEIGHBOUR
void scale_pixels(Config *data, uint8_t *pixel, int linesize, const float scale_factor, const Pixel *region_start,
                  const Pixel *region_end) {

    const float scale_ratio = 1.0f / scale_factor;

    const int region_width = region_end->x - region_start->x;
    const int region_height = region_end->y - region_start->y;

    const size_t buffer_size = region_width * region_height * 3;

    allocate_buffer(data, buffer_size);

    copy_region_pixels(data->buffer, pixel, linesize, region_start, region_end);

    for (unsigned short y = region_start->y; y < region_end->y; y++) {
        for (unsigned short x = region_start->x; x < region_end->x; x++) {
            // Calculate the position of the pixel in the original region
            // (x - region_start->x) = the relative position of the pixel in the region
            // scale_ratio = inverse of the scale factor
            const int source_x = (int) roundf((x - region_start->x) * scale_ratio);
            const int source_y = (int) roundf((y - region_start->y) * scale_ratio);

            if (source_x < region_width && source_y < region_height) {
                const int buff_offset = (source_y * region_width + source_x) * 3;
                const int dest_offset = (y * linesize) + (x * 3);

                pixel[dest_offset] = data->buffer[buff_offset];
                pixel[dest_offset + 1] = data->buffer[buff_offset + 1];
                pixel[dest_offset + 2] = data->buffer[buff_offset + 2];

            }
        }
    }

}

void move_pixels(Config *data, uint8_t *pixel, int linesize, const int width, const int height, const Pixel *region_start, const Pixel *region_end) {

    const int region_width = region_end->x - region_start->x;
    const int region_height = region_end->y - region_start->y;

    int move_x, move_y;
    get_random_move_val(&move_x, &move_y);

    const int new_start_x = region_start->x + move_x;
    const int new_start_y = region_start->y + move_y;
    const int new_end_x = region_end->x + move_x;
    const int new_end_y = region_end->y + move_y;

    if (new_start_x < 0 || new_start_y < 0 || new_end_x > width || new_end_y > height)
        return;

    const size_t buffer_size = region_width * region_height * 3;

    allocate_buffer(data, buffer_size);

    copy_region_pixels(data->buffer, pixel, linesize, region_start, region_end);

    for (unsigned short y = region_start->y; y < region_end->y; y++) {
        for (unsigned short x = region_start->x; x < region_end->x; x++) {
            const int offset = (y * linesize) + (x * 3);
            set_rgb_value(pixel, offset, 0, true, 0, true, 0, true);
        }
    }

    int i = 0;
    for (int y = new_start_y; y < new_end_y; y++) {
        for (int x = new_start_x; x < new_end_x; x++) {
            const int offset = (y * linesize) + (x * 3);
            pixel[offset] = data->buffer[i++];
            pixel[offset + 1] = data->buffer[i++];
            pixel[offset + 2] = data->buffer[i++];
        }
    }

}

static bool overlap(const unsigned short start_x1, const unsigned short start_y1, const unsigned short end_x1,
             const unsigned short end_y1, const unsigned short start_x2, const unsigned short start_y2,
             const unsigned short end_x2, const unsigned short end_y2) {

    if (end_x1 < start_x2 || end_x2 < start_x1 || end_y1 < start_y2 || end_y2 < start_y1)
        return false;

    return true;
}

static void get_random_move_val(int *move_x, int *move_y) {
    // between -100 and +100 pixels
    *move_x = (rand() % 201) - 100;
    *move_y = (rand() % 201) - 100;
}

static void get_random_dimensions(const int width, const int height, unsigned short *region_width,
                                         unsigned short *region_height) {
    // between 10% and 30% of the image
    *region_width = (width * (10 + (rand() % 20))) / 100;
    *region_height = (height * (10 + (rand() % 20))) / 100;
}

static void select_random_operation(Regions *region_data, bool isPair, unsigned short region_width,
                                    unsigned short region_height, unsigned short start_x1,
                                    unsigned short start_y1, unsigned short end_x1, unsigned short end_y1,
                                    unsigned short start_x2, unsigned short start_y2, unsigned short end_x2,
                                    unsigned short end_y2) {
    const int i = rand() % 3;
    switch (i) {
        case 0:
            if (!isPair) {
                push(region_data, false, region_width, region_height, start_x1, start_y1, end_x1, end_y1, 0, 0, 0, 0);
            } else {
                push(region_data, true, region_width, region_height,
                start_x1, start_y1, end_x1, end_y1,
                start_x2, start_y2, end_x2, end_y2);
            }
            break;
        case 1:
            if (!is_empty(region_data))
                pop(region_data);
            break;
        default:
            break;
    }
}

void randomize_single_region(Regions *region_data, const int width, const int height) {

    unsigned short region_width, region_height;
    get_random_dimensions(width, height, &region_width, &region_height);

    const unsigned short start_x1 = rand() % width;
    const unsigned short start_y1 = rand() % height;
    const unsigned short end_x1 = fmin(start_x1 + region_width, width);
    const unsigned short end_y1 = fmin(start_y1 + region_height, height);

    select_random_operation(region_data, false, region_width, region_height, start_x1, start_y1, end_x1, end_y1, 0, 0,
                            0, 0);

}

void randomize(Regions *region_data, const int width, const int height) {

    unsigned short region_width, region_height;
    get_random_dimensions(width, height, &region_width, &region_height);

    unsigned short start_x1, start_y1, end_x1, end_y1, start_x2, start_y2, end_x2, end_y2;

    do {
        start_x1 = rand() % (width - region_width);
        start_y1 = rand() % (height - region_height);
        end_x1 = start_x1 + region_width;
        end_y1 = start_y1 + region_height;

        start_x2 = rand() % (width - region_width);
        start_y2 = rand() % (height - region_height);
        end_x2 = start_x2 + region_width;
        end_y2 = start_y2 + region_height;
    } while (overlap(start_x1, start_y1, end_x1, end_y1, start_x2, start_y2, end_x2, end_y2));

    select_random_operation(region_data, true, region_width, region_height, start_x1, start_y1, end_x1, end_y1,
                            start_x2, start_y2, end_x2, end_y2);

}