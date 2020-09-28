#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include <SDL2/SDL.h>

#include "glad/glad.h"

#include "opengl.h"

struct media {
    AVFormatContext *format_context;
    AVCodec *video_codec;
    AVCodec *audio_codec;
    AVCodecContext *video_codec_context;
    AVCodecContext *audio_codec_context;
    AVStream *video_stream;
    AVStream *audio_stream;
    int video_index;
    int audio_index;

    struct SwsContext *sws_context;
};
typedef struct media media_t;

int fill_stream_info(AVStream *stream, AVCodec **codec, AVCodecContext **codec_context)
{
    *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!*codec) { return -1; }

    *codec_context = avcodec_alloc_context3(*codec);
    if (!*codec_context) { return -1; }

    if (avcodec_parameters_to_context(*codec_context, stream->codecpar) < 0) { return -1; }
    if (avcodec_open2(*codec_context, *codec, NULL) < 0) { return -1; }
}

int find_streams(media_t *media)
{
    for (int i = 0; i < media->format_context->nb_streams; ++i) {
        if (media->format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            media->video_index = i;
            media->video_stream = media->format_context->streams[i];

            if (fill_stream_info(media->video_stream, &media->video_codec, &media->video_codec_context) < 0 ) { return -1; }
        }
    }

    return 0;
}

int media_load_file(media_t *media, const char* path)
{
    media->format_context = avformat_alloc_context();
    if (!media->format_context) { return -1; }
    if (avformat_open_input(&media->format_context, path, NULL, NULL) != 0) { return -1; }

    if (avformat_find_stream_info(media->format_context, NULL) < 0) { return -1; }

    if (find_streams(media) < 0) { return -1; }

    media->sws_context = sws_getContext(
            media->video_codec_context->width,
            media->video_codec_context->height,
            media->video_codec_context->pix_fmt,
            media->video_codec_context->width,
            media->video_codec_context->height,
            AV_PIX_FMT_RGB24,
            SWS_BICUBIC,
            NULL, NULL, NULL
            );
}

/*
 * decoding
 */

int media_decode_video_frame(media_t *media, AVFrame *frame, AVPacket *packet)
{
    int ret = 0;

    while (av_read_frame(media->format_context, packet) >= 0) {
        if (packet->stream_index == media->video_index) {

            ret = avcodec_send_packet(media->video_codec_context, packet);
            if (ret < 0) {
                return -1;
            }

            ret = avcodec_receive_frame(media->video_codec_context, frame);

            if (ret == AVERROR(EAGAIN)) {
                continue;
            } else if (ret == 0) {
                break;
            } else {
                return ret;
            }
        }

        av_packet_unref(packet);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Please input one file!\n");
        return 0;
    }

    media_t *media = (media_t*)calloc(1, sizeof(media_t));

    media_load_file(media, argv[1]);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;

    SDL_Window *window = SDL_CreateWindow("media_player",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            media->video_codec_context->width, media->video_codec_context->height,
            SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_Event event;

    // opengl functions
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    printf("GL_VENDOR:   %s\n", glGetString(GL_VENDOR));
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
    printf("GL_VERSION:  %s\n", glGetString(GL_VERSION));
    printf("video: %ix%i\n", media->video_codec_context->width, media->video_codec_context->height);

    SDL_GL_SetSwapInterval(1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // decode first video frame
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    AVFrame *gl_frame = av_frame_alloc();

    int size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, media->video_codec_context->width, media->video_codec_context->height, 32);
    uint8_t *buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
    av_image_fill_arrays(gl_frame->data, gl_frame->linesize, buffer, AV_PIX_FMT_RGB24, media->video_codec_context->width, media->video_codec_context->height, 32);

    gl_init(media->video_codec_context->width, media->video_codec_context->height);

    // main loop
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
        }

        media_decode_video_frame(media, frame, packet);
        sws_scale(media->sws_context, (uint8_t const * const *)frame->data, frame->linesize, 0, media->video_codec_context->height, gl_frame->data, gl_frame->linesize);

        gl_update_texture(gl_frame->data[0]);
        gl_new_frame();

        SDL_Delay(1000 * (1.0/(double)av_q2d(media->format_context->streams[media->video_index]->r_frame_rate)) - 10);
        SDL_GL_SwapWindow(window);
    }


    return 0;
}
