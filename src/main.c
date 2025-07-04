#include "video-effects.h"
#include "cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {

    Regions region_data = {
        .region_pair = NULL,
        .size = 0
    };

    Config data = {
        .region_data = &region_data,
        .effect_id = NONE,
        .scale_factor = 0.0f,
        .buffer = NULL,
        .input_file = NULL,
        .output_file = NULL
    };

    parse_cmdline(argc, argv, &data);

    if (validate_arguments(&data) == EXIT_FAILURE)
        exit(EXIT_FAILURE);

    srand(time(NULL));

    process_video(data.input_file, data.output_file, &data);

    cleanup_regions(data.region_data);
    free(data.buffer);

    printf("[INFO] The filter '%s' was successfully applied to '%s' and saved as '%s'\n",
           get_filter_name(data.effect_id), data.input_file, data.output_file);

    return EXIT_SUCCESS;
}