/**
 * @file FileInput.hpp
 *
 * @class FileInput
 *
 * @brief Define the input of media through a file
 *
 * Setup demuxers and codecs of the media inside of a file
 *
 * @license GNU GPL V3
 * 
 * Copyright (C) 2018 Caio Marcelo Campoy Guedes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Caio Marcelo Campoy Guedes <caiomcg@gmail.com>
 */

#pragma once

#include <string>
#include <iostream>
#include <stdexcept>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/hwcontext.h>
    #include <libavcodec/vaapi.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/hwcontext_vaapi.h>
}

#include "LibAVInputMedia.hpp"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static struct vaapi_context libva_ctx_;
static AVBufferRef* buffer_ref_; // hwaccel

typedef struct DecodeContext {
    AVBufferRef *hw_device_ref;
} DecodeContext;

static AVPixelFormat formatCallback(AVCodecContext* av_codec_ctx, const AVPixelFormat* fmt) {
    while (*fmt != AV_PIX_FMT_NONE) {
        if (*fmt == AV_PIX_FMT_VAAPI) {
            
            AVBufferRef* decode = (AVBufferRef*)av_codec_ctx->opaque;
            
            AVHWFramesContext  *frames_ctx;
            AVVAAPIFramesContext *frames_hwctx;
            /* create a pool of surfaces to be used by the decoder */
            av_codec_ctx->hw_frames_ctx = av_hwframe_ctx_alloc(buffer_ref_); // Should grab from decodecontext
            if (!av_codec_ctx->hw_frames_ctx)
                return AV_PIX_FMT_NONE;

            frames_ctx   = (AVHWFramesContext*)av_codec_ctx->hw_frames_ctx->data;
            frames_hwctx = (AVVAAPIFramesContext*)frames_ctx->hwctx;
            frames_ctx->format            = AV_PIX_FMT_VAAPI;
            frames_ctx->sw_format         = av_codec_ctx->sw_pix_fmt;
            frames_ctx->width             = FFALIGN(av_codec_ctx->coded_width,  32);
            frames_ctx->height            = FFALIGN(av_codec_ctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;
            //frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            int ret = av_hwframe_ctx_init(av_codec_ctx->hw_frames_ctx);
            if (ret < 0)
                return AV_PIX_FMT_NONE;

            return AV_PIX_FMT_VAAPI;
        }
        fmt++;
    }
    return AV_PIX_FMT_NONE;
}

class FileInput : public LibAVInputMedia {
public:
    virtual ~FileInput() override;
    void open(const std::string& file_name) override;
    AVPacket* read() override;
};