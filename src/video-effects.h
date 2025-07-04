#pragma once

#include <stdbool.h>
#include <libavutil/frame.h>

#include "cmdline.h"

#define AV_NOT_NEGATIVE(ret) check_av_error_positive(ret, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define NOT_NULL(ptr) check_not_null(ptr, __FILE__, __PRETTY_FUNCTION__,  __LINE__);

void check_av_error_positive(int err, const char *file_name, const char *function_name, int line);
void check_not_null(const void* ptr, const char *file_name, const char *function_name, int line);

void process_frame(AVFrame *rgb_frame, void *user_data);

void process_video(const char *input_file_path, const char *output_file_path, Config *data);

void set_rgb_value(uint8_t *pixel, int offset, uint8_t r, bool update_r, uint8_t g, bool update_g, uint8_t b,
                   bool update_b);