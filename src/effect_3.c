#include "effect.h"
#include "video-effects.h"
#include "region/region.h"

// Region Move
void apply_effect_3(uint8_t *pixel, const int linesize, const int width, const int height, Config *data) {

    randomize_single_region(data->region_data, width, height);

    for (int i = 0; i < data->region_data->size; i++) {
        const Region *current = &data->region_data->region_pair[i].one;
        move_pixels(data, pixel, linesize, width, height, &current->start, &current->end);
    }

}