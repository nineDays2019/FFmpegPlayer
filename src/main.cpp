#include <iostream>
#include <opencv2/opencv.hpp>
#include <SDL/SDL.h>
#include <stdio.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

// Full Screen
#define SHOW_FULLSCREEN 0
// Output YUV420p
#define OUTPUT_YUV420P 0

typedef SDL_Surface *pSurface;
using namespace cv;
using namespace std;

int main() {
    // FFmpeg
    AVFormatContext *pFormatCtx;
    int i, video_index;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    AVPacket *packet;
    struct SwsContext *img_convert_ctx;

    // SDL
    int screen_w, screen_h;
    SDL_Surface *screen;
    SDL_VideoInfo *vi;
    SDL_Overlay *bmp;
    SDL_Rect rect;

    FILE *fp_yuv = nullptr;
    int ret, got_picture;
    char filepath[] = "/home/juhezi/Desktop/demo.flv";

    av_register_all();
    avformat_network_init();

    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, filepath, nullptr, nullptr) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        printf("Couldn't find stream information.\n");
    }
    video_index = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        }
    }
    if (video_index == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[video_index]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr) {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not init SDL - %s\n", SDL_GetError());
        return -1;
    }

#if SHOW_FULLSCREEN
    vi = const_cast<SDL_VideoInfo *>(SDL_GetVideoInfo());
    screen_w = vi->current_w;
    screen_h = vi->current_h;
    screen = SDL_SetVideoMode(screen_w, screen_h, 0, SDL_FULLSCREEN);
#else
    screen_w = pCodecCtx->width;
    screen_h = pCodecCtx->height;
    screen = SDL_SetVideoMode(screen_w, screen_h, 0, 0);
#endif

    if (!screen) {
        printf("SDL: could not set video mode - exiting: %s\n", SDL_GetError());
        return -1;
    }

    bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height, SDL_YV12_OVERLAY, screen);

    rect.x = 0;
    rect.y = 0;
    rect.w = static_cast<Uint16>(screen_w);
    rect.h = static_cast<Uint16>(screen_h);
    // SDL End

    packet = static_cast<AVPacket *>(av_malloc(sizeof(AVPacket)));
    printf("-----File Info-----\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    printf("\n-------------------\n");

#if OUTPUT_YUV420P
    fp_yuv = fopen("output.yuv", "wb+");
#endif

    SDL_WM_SetCaption("Simple FFmpeg Player", nullptr);

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                     nullptr, nullptr, nullptr);

    // todo 160
    while (av_read_frame(pFormatCtx, packet) >= 0) {

        if (packet->stream_index == video_index) {
            // Decode
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                printf("Decode Error.\n");
                return -1;
            }
            if (got_picture) {
                SDL_LockYUVOverlay(bmp);
                pFrameYUV->data[0] = bmp->pixels[0];
                pFrameYUV->data[1] = bmp->pixels[2];
                pFrameYUV->data[2] = bmp->pixels[1];
                pFrameYUV->linesize[0] = bmp->pitches[0];
                pFrameYUV->linesize[1] = bmp->pitches[2];
                pFrameYUV->linesize[2] = bmp->pitches[1];
                sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

#if OUTPUT_YUV420P
                int y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, static_cast<size_t>(y_size), fp_yuv);         // Y
                fwrite(pFrameYUV->data[1], 1, static_cast<size_t>(y_size / 4), fp_yuv);     // U
                fwrite(pFrameYUV->data[2], 1, static_cast<size_t>(y_size / 4), fp_yuv);     // V
#endif

                SDL_UnlockYUVOverlay(bmp);

                SDL_DisplayYUVOverlay(bmp, &rect);

                // Delay 40ms
                SDL_Delay(40);
            }
        }
        av_free_packet(packet);

    }

    // FIX: Flush Frames remained in Codec
    while (1) {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0) {
            break;
        }
        if (!got_picture) {
            break;
        }
        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

        SDL_LockYUVOverlay(bmp);
        pFrameYUV->data[0] = bmp->pixels[0];
        pFrameYUV->data[1] = bmp->pixels[2];
        pFrameYUV->data[2] = bmp->pixels[1];
        pFrameYUV->linesize[0] = bmp->pitches[0];
        pFrameYUV->linesize[1] = bmp->pitches[2];
        pFrameYUV->linesize[2] = bmp->pitches[1];

#if OUTPUT_YUV420P
        int y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, static_cast<size_t>(y_size), fp_yuv);         // Y
                fwrite(pFrameYUV->data[1], 1, static_cast<size_t>(y_size / 4), fp_yuv);     // U
                fwrite(pFrameYUV->data[2], 1, static_cast<size_t>(y_size / 4), fp_yuv);     // V
#endif

        SDL_UnlockYUVOverlay(bmp);

        SDL_DisplayYUVOverlay(bmp, &rect);

        // Delay 40ms
        SDL_Delay(40);

    }

    sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P
    fclose(fp_yuv);
#endif

    SDL_Quit();

    // av_free(out_buffer);
    av_free(pFrameYUV);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}