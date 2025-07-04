#include "effect.h"
#include "video-effects.h"
#include "region/region.h"

// Region Swap
void apply_effect_2(uint8_t *pixel, const int linesize, const int width, const int height, Config *data) {

    randomize(data->region_data, width, height);

    for (int i = 0; i < data->region_data->size; i++) {
        const RegionPair *current = &data->region_data->region_pair[i];
        swap_pixels(data, pixel, linesize, &current->one.start, &current->one.end, &current->two.start,
                    &current->two.end);
    }

}