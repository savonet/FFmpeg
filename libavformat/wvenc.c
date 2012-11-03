/*
 * WavPack muxer
 * Copyright (c) 2012 Paul B Mahol
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "avio_internal.h"
#include "apetag.h"

#define WV_EXTRA_SIZE 12
#define WV_END_BLOCK  0x1000

typedef struct{
    uint32_t duration;
    int off;
} WVMuxContext;

static int write_header(AVFormatContext *s)
{
    WVMuxContext *wc = s->priv_data;
    AVCodecContext *codec = s->streams[0]->codec;

    if (s->nb_streams > 1) {
        av_log(s, AV_LOG_ERROR, "only one stream is supported\n");
        return AVERROR(EINVAL);
    }
    if (codec->codec_id != AV_CODEC_ID_WAVPACK) {
        av_log(s, AV_LOG_ERROR, "unsupported codec\n");
        return AVERROR(EINVAL);
    }
    if (codec->extradata_size > 0) {
        av_log_missing_feature(s, "remuxing from matroska container", 0);
        return AVERROR_PATCHWELCOME;
    }
    wc->off = codec->channels > 2 ? 4 : 0;
    avpriv_set_pts_info(s->streams[0], 64, 1, codec->sample_rate);

    return 0;
}

static int write_packet(AVFormatContext *s, AVPacket *pkt)
{
    WVMuxContext *wc = s->priv_data;
    AVIOContext *read_pb;
    AVIOContext *pb = s->pb;
    uint32_t size, flags;
    uint32_t left = pkt->size;
    uint8_t *pos = pkt->data;

    read_pb = avio_alloc_context(pkt->data, pkt->size, 0, NULL, NULL, NULL, NULL);
    if (!read_pb)
        return AVERROR(ENOMEM);

    wc->duration += pkt->duration;
    ffio_wfourcc(pb, "wvpk");
    if (wc->off)
        size = AV_RL32(pkt->data) - 12;
    else
        size = pkt->size;
    if (size > left)
        return AVERROR(EINVAL);
    avio_wl32(pb, size + 12);
    avio_wl16(pb, 0x410);
    avio_w8(pb, 0);
    avio_w8(pb, 0);
    avio_wl32(pb, -1);
    avio_wl32(pb, pkt->pts);
    pos += wc->off;
    flags = AV_RL32(pos + 4);
    avio_write(pb, pos, size);
    pos  += size;
    left -= size;
    while (!(flags & WV_END_BLOCK)) {
        ffio_wfourcc(pb, "wvpk");
        size = AV_RL32(pos);
        av_log(s, AV_LOG_ERROR, "%d\n", size);
        pos += 4;
        avio_wl32(pb, size);
        avio_wl16(pb, 0x410);
        avio_w8(pb, 0);
        avio_w8(pb, 0);
        avio_wl32(pb, -1);
        avio_wl32(pb, pkt->pts);
        flags = AV_RL32(pos + 4);
        avio_write(pb, pos, WV_EXTRA_SIZE);
        pos += WV_EXTRA_SIZE;
        avio_write(pb, pos, size - 24);
        pos += size - 24;
    }
    avio_flush(pb);

    return 0;
}

static int write_trailer(AVFormatContext *s)
{
    WVMuxContext *wc = s->priv_data;
    AVIOContext *pb = s->pb;

    ff_ape_write(s);

    if (pb->seekable) {
        avio_seek(pb, 12, SEEK_SET);
        avio_wl32(pb, wc->duration);
        avio_flush(pb);
    }

    return 0;
}

AVOutputFormat ff_wv_muxer = {
    .name              = "wv",
    .long_name         = NULL_IF_CONFIG_SMALL("WavPack"),
    .priv_data_size    = sizeof(WVMuxContext),
    .extensions        = "wv",
    .audio_codec       = AV_CODEC_ID_WAVPACK,
    .video_codec       = AV_CODEC_ID_NONE,
    .write_header      = write_header,
    .write_packet      = write_packet,
    .write_trailer     = write_trailer,
};
