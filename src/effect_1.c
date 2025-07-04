#include "effect.h"
#include "video-effects.h"
#include "region/region.h"

// Region Scaling
void apply_effect_1(uint8_t *pixel, const int linesize, const int width, const int height, Config *data) {

    randomize_single_region(data->region_data, width, height);

    for (int i = 0; i < data->region_data->size; i++) {
        const Region *current = &data->region_data->region_pair[i].one;
        scale_pixels(data, pixel, linesize, data->scale_factor, &current->start, &current->end);
    }

}