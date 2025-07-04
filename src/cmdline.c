#include "cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <stdint.h>

const char *argp_program_version = "video-effects 2025-beta1";
char doc[] = "A program that applies post-processing effects on a video";
char args_doc[] = "";

struct argp_option options[] = {
    {"input", 'i', "FILE", 0, "Input video file"},
    {"output", 'o', "FILE", 0, "Output video file"},
    {"filter", 'f', "NUMBER", 0, "Effect type: 1 = Region Scaling, 2 = Region Swap, 3 = Region Move"},
    {"scale", 's', "FLOAT", 0, "Scale factor (only for Region Scaling, between 0.1 and 3.0)"},
    {0}
};

error_t parse_options(int key, char *arg, struct argp_state *state) {

    Config *arguments = state->input;

    switch (key) {
        case 'i':
            arguments->input_file = arg;
            break;
        case 'o':
            arguments->output_file = arg;
            break;
        case 's':
            if (arg) {
                arguments->scale_factor = strtof(arg, NULL);
            }
            break;
        case 'f':
            if (arg) {
                const uint8_t id = (uint8_t) strtol(arg, NULL, 10);
                switch (id) {
                    case 1:
                        arguments->effect_id = EFFECT_ONE;
                        break;
                    case 2:
                        arguments->effect_id = EFFECT_TWO;
                        break;
                    case 3:
                        arguments->effect_id = EFFECT_THREE;
                        break;
                    default:
                        argp_error(state, "Invalid filter value. Expected: %d, %d or %d", EFFECT_ONE, EFFECT_TWO, EFFECT_THREE);
                }
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int parse_cmdline(int argc, char **argv, Config *data) {
    struct argp parser = { options, parse_options, args_doc, doc };
    const int res = argp_parse(&parser, argc, argv, 0, 0, data);

    if (argc == 1) {
        argp_help(&parser, stderr, ARGP_HELP_STD_ERR, *argv);
        exit(1);
    }

    return res;
}

// only being triggered when argp_parse() doesn't exit, e.g. --help
int validate_arguments(const Config *data) {

    uint8_t errors = 0;

    if (data->input_file == NULL) {
        fprintf(stderr, "[ERROR] Missing required argument: --input=<file> (-i <file>)\n");
        errors++;
    }
    if (data->output_file == NULL) {
        fprintf(stderr, "[ERROR] Missing required argument: --output=<file>\n");
        errors++;
    }
    if (data->effect_id == NONE) {
        fprintf(stderr, "[ERROR] Missing required argument: --filter=<number>\n");
        errors++;
    }
    if (data->effect_id == EFFECT_ONE && (data->scale_factor <= 0 || data->scale_factor > 3)) {
        fprintf(stderr, "[ERROR] Invalid scale factor: --scale=<float> must be greater than 0 or smaller than 3 for Region Scaling\n");
        errors++;
    }

    return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;

}

char *get_filter_name(const EffectType id) {

    if (id == EFFECT_ONE) return "Region Transform";
    if (id == EFFECT_TWO) return "Region Swap";
    if (id == EFFECT_THREE) return "Region Move";

    return "Unknown Effect";
}