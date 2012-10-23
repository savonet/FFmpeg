/*
 * raw TAK muxer
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

#include "avformat.h"
#include "avio_internal.h"
#include "apetag.h"
#include "libavcodec/tak.h"

typedef struct TAXMuxContext {
    int     size;
    int64_t offset;
} TAKMuxContext;

static int tak_write_header(struct AVFormatContext *s)
{
    AVCodecContext *codec = s->streams[0]->codec;

    if (s->nb_streams > 1) {
        av_log(s, AV_LOG_ERROR, "only one stream is supported\n");
        return AVERROR(EINVAL);
    }
    if (codec->codec_id != AV_CODEC_ID_TAK) {
        av_log(s, AV_LOG_ERROR, "unsupported codec\n");
        return AVERROR(EINVAL);
    }
    if (!codec->extradata || codec->extradata_size < TAK_MIN_STREAMINFO_BYTES + 3) {
        av_log(codec, AV_LOG_ERROR, "extradata NULL or too small.\n");
        return AVERROR(EINVAL);
    }

    ffio_wfourcc(s->pb, "tBaK");

    avio_w8(s->pb, TAK_METADATA_STREAMINFO);
    avio_wl24(s->pb, codec->extradata_size);
    avio_write(s->pb, codec->extradata, codec->extradata_size);

    avio_w8(s->pb, TAK_METADATA_END);
    avio_wl24(s->pb, 0);

    return 0;
}

static int tak_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    TAKMuxContext *tc = s->priv_data;

    avio_write(s->pb, pkt->data, pkt->size);
    avio_flush(s->pb);
    tc->size = pkt->size;

    return 0;
}

static int tak_write_trailer(struct AVFormatContext *s)
{
    AVCodecContext *codec = s->streams[0]->codec;

    ff_ape_write(s);

    if (s->pb->seekable) {
        int64_t file_size;

        /* rewrite the STREAMINFO metadata */
        file_size = avio_tell(s->pb);
        avio_seek(s->pb, 8, SEEK_SET);
        avio_write(s->pb, codec->extradata, codec->extradata_size);
        avio_seek(s->pb, file_size, SEEK_SET);
    }

    avio_flush(s->pb);

    return 0;
}

AVOutputFormat ff_tak_muxer = {
    .name           = "tak",
    .long_name      = NULL_IF_CONFIG_SMALL("raw TAK"),
    .priv_data_size = sizeof(TAKMuxContext),
    .audio_codec    = AV_CODEC_ID_TAK,
    .video_codec    = AV_CODEC_ID_NONE,
    .write_header   = tak_write_header,
    .write_packet   = tak_write_packet,
    .write_trailer  = tak_write_trailer,
    .flags          = AVFMT_NOTIMESTAMPS,
    .extensions     = "tak",
};
