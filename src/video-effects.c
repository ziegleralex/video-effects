#include "video-effects.h"
#include "cmdline.h"
#include "effect.h"

#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

static const unsigned int max_error_message_size = 64;

void check_av_error_positive(int err, const char *file_name, const char *function_name, int line) {
    if (err < 0) {
        char err_message[max_error_message_size];
        fprintf(stderr, 
                "AVError returned %d: %s in file %s %s line %d\n", 
                err, 
                av_make_error_string(err_message, max_error_message_size, err),
                file_name,
                function_name,
                line);
        exit(-1);
	}
}

void check_not_null(const void* ptr, const char *file_name, const char *function_name, int line) {
    if (ptr == NULL) {
        fprintf(stderr, 
                "Nullpointer in file %s in function %s line %d\n",
                file_name,
                function_name,
                line);
        exit(-1);
    }
}

void process_video(const char *input_file_path, const char *output_file_path, Config *data) {
    
    av_log_set_level(AV_LOG_ERROR);
    AVFormatContext *input_format_context = NULL;
    AVFormatContext *output_format_context = NULL;
     
    AV_NOT_NEGATIVE(avformat_open_input(&input_format_context, input_file_path, NULL, NULL));
    AV_NOT_NEGATIVE(avformat_find_stream_info(input_format_context, NULL));
    
    const int video_stream_index = av_find_best_stream(input_format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AV_NOT_NEGATIVE(video_stream_index);
    
    AVStream *video_stream = input_format_context->streams[video_stream_index];
    
    const AVCodec *video_decoder = avcodec_find_decoder(video_stream->codecpar->codec_id);
    NOT_NULL(video_decoder);

    AVCodecContext *decoder_context = avcodec_alloc_context3(video_decoder);
    NOT_NULL(decoder_context);
    AV_NOT_NEGATIVE(avcodec_parameters_to_context(decoder_context, video_stream->codecpar));
    AV_NOT_NEGATIVE(avcodec_open2(decoder_context, video_decoder, NULL));


    AV_NOT_NEGATIVE(avformat_alloc_output_context2(&output_format_context, NULL, NULL, output_file_path));
    
    AVStream *out_video_stream = NULL;
    for (int i = 0; i < input_format_context->nb_streams; ++i){
        AVStream *out_stream = avformat_new_stream(output_format_context, NULL);
        NOT_NULL(out_stream);
        if (input_format_context->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
                AV_NOT_NEGATIVE(
                    avcodec_parameters_copy(out_stream->codecpar, input_format_context->streams[i]->codecpar));
        }
        else {
            out_video_stream = out_stream;
        }
    }
    
    NOT_NULL(out_video_stream);

    const AVCodec *video_encoder = avcodec_find_encoder(video_stream->codecpar->codec_id);
    NOT_NULL(video_encoder);

    AVCodecContext *encoder_context = avcodec_alloc_context3(video_encoder);
    NOT_NULL(encoder_context);
    
    encoder_context->height = decoder_context->height;
    encoder_context->width = decoder_context->width;
    encoder_context->pix_fmt = video_encoder->pix_fmts[0];
    encoder_context->time_base = video_stream->time_base;

    AV_NOT_NEGATIVE(avcodec_open2(encoder_context, video_encoder, NULL));

    AV_NOT_NEGATIVE(avcodec_parameters_from_context(out_video_stream->codecpar, encoder_context));
    out_video_stream->time_base = encoder_context->time_base;

    AV_NOT_NEGATIVE(avio_open(&output_format_context->pb, output_file_path, AVIO_FLAG_WRITE));
    AV_NOT_NEGATIVE(avformat_write_header(output_format_context, NULL));

    struct SwsContext *input_format_to_rgb_sws_context = NULL;
    struct SwsContext *rgb_to_output_format_sws_context = NULL;
    input_format_to_rgb_sws_context = sws_getCachedContext(input_format_to_rgb_sws_context, 
                                                           decoder_context->width,
                                                           decoder_context->height,
                                                           decoder_context->pix_fmt,
                                                           encoder_context->width,
                                                           encoder_context->height,
                                                           AV_PIX_FMT_RGB24,
                                                           SWS_BICUBIC, NULL, NULL, NULL);
    NOT_NULL(input_format_to_rgb_sws_context);

    AVStream *output_video_stream = output_format_context->streams[video_stream_index];
    rgb_to_output_format_sws_context = sws_getCachedContext(rgb_to_output_format_sws_context,
                                                            decoder_context->width,
                                                            decoder_context->height,
                                                            AV_PIX_FMT_RGB24,
                                                            encoder_context->width,
                                                            encoder_context->height,
                                                            encoder_context->pix_fmt,
                                                            SWS_BICUBIC, NULL, NULL, NULL);
    NOT_NULL(rgb_to_output_format_sws_context);
    
    AVFrame *input_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    AVFrame *output_frame = av_frame_alloc();

    int buff_size = av_image_alloc(rgb_frame->data, rgb_frame->linesize, video_stream->codecpar->width,
                                   video_stream->codecpar->height, AV_PIX_FMT_RGB24, 1);
    AV_NOT_NEGATIVE(buff_size);
    
    buff_size = av_image_alloc(output_frame->data, output_frame->linesize, encoder_context->width,
                               encoder_context->height, encoder_context->pix_fmt, 1);
    AV_NOT_NEGATIVE(buff_size);
    
    output_frame->format = encoder_context->pix_fmt;
    output_frame->width  = encoder_context->width;
    output_frame->height = encoder_context->height;

    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = encoder_context->width;
    rgb_frame->height = encoder_context->height;


    AVPacket packet;
    int ret;
    while((ret = av_read_frame(input_format_context, &packet)) >= 0) {
        if (packet.stream_index == video_stream_index) {
            AV_NOT_NEGATIVE(avcodec_send_packet(decoder_context, &packet));
            
            while (avcodec_receive_frame(decoder_context, input_frame) >= 0) {
                AV_NOT_NEGATIVE(sws_scale(input_format_to_rgb_sws_context, 
                                          (const uint8_t * const *)input_frame->data, 
                                          input_frame->linesize,
                                          0,
                                          decoder_context->height, 
                                          rgb_frame->data, 
                                          rgb_frame->linesize));
                rgb_frame->pts = packet.pts;
                process_frame(rgb_frame, data);
                AV_NOT_NEGATIVE(sws_scale(rgb_to_output_format_sws_context, 
                                          (const uint8_t * const *)rgb_frame->data, 
                                          rgb_frame->linesize,
                                          0, 
                                          rgb_frame->height, 
                                          output_frame->data, 
                                          output_frame->linesize));

                //output_frame->pts = av_rescale_q(input_frame->pts, video_stream->time_base,
                //                                 out_video_stream->time_base);
                output_frame->pts = input_frame->pts;
                AV_NOT_NEGATIVE(avcodec_send_frame(encoder_context, output_frame));
                AVPacket encoded_packet = {};
                while (avcodec_receive_packet(encoder_context, &encoded_packet) >= 0) {
                    encoded_packet.stream_index = out_video_stream->index;
                    av_packet_rescale_ts(&encoded_packet, encoder_context->time_base, out_video_stream->time_base);
                    AV_NOT_NEGATIVE(av_interleaved_write_frame(output_format_context, &encoded_packet));
                    av_packet_unref(&encoded_packet);
                }
            }
        }
        else {
            AV_NOT_NEGATIVE(av_interleaved_write_frame(output_format_context, &packet));
        }
        av_packet_unref(&packet);
    }

    AV_NOT_NEGATIVE(avcodec_send_frame(encoder_context, NULL));
    while (avcodec_receive_packet(encoder_context, &packet) >= 0) {
        packet.stream_index = out_video_stream->index;
        av_packet_rescale_ts(&packet, encoder_context->time_base, out_video_stream->time_base);
        AV_NOT_NEGATIVE(av_interleaved_write_frame(output_format_context, &packet));
        av_packet_unref(&packet);
    }
    

    AV_NOT_NEGATIVE(av_write_trailer(output_format_context));

    avcodec_free_context(&decoder_context);
    avcodec_free_context(&encoder_context);
    
    av_freep(&rgb_frame->data[0]);
    av_freep(&output_frame->data[0]);

    av_frame_free(&input_frame);
    av_frame_free(&rgb_frame);
    av_frame_free(&output_frame);
    
    sws_freeContext(input_format_to_rgb_sws_context);
    sws_freeContext(rgb_to_output_format_sws_context);
    
    avformat_close_input(&input_format_context);
    avformat_free_context(output_format_context);
}

void process_frame(AVFrame *rgb_frame, void *user_data) {

    Config *data = user_data;

    uint8_t *pixel = rgb_frame->data[0];

    const int linesize = rgb_frame->linesize[0];
    const int width = rgb_frame->width;
    const int height = rgb_frame->height;

    switch (data->effect_id) {
        case EFFECT_ONE:
            apply_effect_1(pixel, linesize, width, height, data);
            break;
        case EFFECT_TWO:
            apply_effect_2(pixel, linesize, width, height, data);
            break;
        case EFFECT_THREE:
            apply_effect_3(pixel, linesize, width, height, data);
            break;
        default:
            fprintf(stderr, "[ERROR] Unknown effect: %s\n", get_filter_name(data->effect_id));
    }
}

void set_rgb_value(uint8_t *pixel, const int offset, const uint8_t r, const bool update_r, const uint8_t g,
                   const bool update_g, const uint8_t b, const bool update_b) {
    if (update_r)
        *(pixel + offset) = r;
    if (update_g)
        *(pixel + offset + 1) = g;
    if (update_b)
        *(pixel + offset + 2) = b;
}