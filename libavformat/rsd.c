/*
 * RSD File Demuxer
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

static const AVCodecTag codec_rsd_tags[] = {
    { AV_CODEC_ID_PCM_S16BE,   MKTAG('P','C','M','B') },
    { AV_CODEC_ID_PCM_S16LE,   MKTAG('P','C','M',' ') },
    { AV_CODEC_ID_ADPCM_GADP,  MKTAG('G','A','D','P') },
    { AV_CODEC_ID_NONE,    0 },
};

static int rsd_probe(AVProbeData *p)
{
    if (!memcmp(p->buf, "RSD", 3) &&
        p->buf[3] >= '2' && p->buf[3] <= '6')
        return AVPROBE_SCORE_MAX / 2;
    return 0;
}

static int rsd_read_header(AVFormatContext *s)
{
    AVStream *st;
    unsigned int tag;
    uint32_t start_offset;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);

    avio_skip(s->pb, 4);
    tag = avio_rl32(s->pb);

    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id   = ff_codec_get_id(codec_rsd_tags, tag);
    if (st->codec->codec_id == AV_CODEC_ID_NONE) {
        av_log_ask_for_sample(s, "unknown tag %X\n", tag);
        return AVERROR_PATCHWELCOME;
    }
    st->codec->channels = avio_rl32(s->pb);
    avio_skip(s->pb, 4); // output bit depth
    st->codec->sample_rate = avio_rl32(s->pb);
    avio_skip(s->pb, 4); // block align
    start_offset = avio_rl32(s->pb);

    if (st->codec->codec_id == AV_CODEC_ID_ADPCM_GADP) {
        st->codec->extradata_size = 32 * st->codec->channels;
        st->codec->extradata = av_malloc(32 * st->codec->channels);
        avio_skip(s->pb, 1);
        avio_read(s->pb, st->codec->extradata, 32 * st->codec->channels);
    }

    if (start_offset < avio_tell(s->pb))
        return AVERROR_INVALIDDATA;

    avio_skip(s->pb, start_offset - avio_tell(s->pb));

    avpriv_set_pts_info(st, 64, 1, st->codec->sample_rate);

    return 0;
}

static int rsd_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    return av_get_packet(s->pb, pkt, 1024);
}

AVInputFormat ff_rsd_demuxer = {
    .name           = "rsd",
    .long_name      = NULL_IF_CONFIG_SMALL("RSD"),
    .read_probe     = rsd_probe,
    .read_header    = rsd_read_header,
    .read_packet    = rsd_read_packet,
};
