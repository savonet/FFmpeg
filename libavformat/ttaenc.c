#include "libavcodec/tta.h"
#include "avformat.h"
#include "avio_internal.h"

// FIXME
static int tta_write_seek_table(AVIOContext *pb)
{
    avio_wl32(pb, 0); // XXX seek point
    avio_wl32(pb, 0); // XXX CRC32
    return 0;
}

static int tta_write_header(struct AVFormatContext *s)
{
    uint32_t channels, bps, samplerate, samples;
    AVCodecContext *codec = s->streams[0]->codec;
    AVIOContext *pb = s->pb;

    switch (codec->sample_fmt) {
    case AV_SAMPLE_FMT_U8:
        bps = 1;
        break;
    case AV_SAMPLE_FMT_S16:
        bps = 2;
        break;
    case AV_SAMPLE_FMT_S32:
        bps = 3;
        break;
    default:
        av_log (s, AV_LOG_ERROR, "invalid/unsupported sample format\n");
        return AVERROR_INVALIDDATA;
    }

    samplerate = codec->sample_rate;
    if (samplerate <= 0 || samplerate > 1000000) {
        av_log(s, AV_LOG_ERROR, "nonsense samplerate\n");
        return -1;
    }

    channels = codec->channels;
    if (channels == 0 || channels > 8) {
        av_log(s, AV_LOG_ERROR, "invalid/unsupported number of channels\n");
        return -1;
    }

    samples = avio_size(pb) / (channels * bps);
    if (samples == 0) {
        av_log(s, AV_LOG_ERROR, "no samples\n");
        return -1;
    }

    ffio_wfourcc(pb, "TTA1");
    avio_wl16(pb, FORMAT_SIMPLE);
    avio_wl16(pb, channels);
    avio_wl16(pb, bps);
    avio_wl32(pb, samplerate);
    avio_wl32(pb, samples); // XXX
    avio_wl32(pb, 0); // XXX CRC32

    tta_write_seek_table(pb);
    return 0;
}

static int tta_write_trailer(struct AVFormatContext *s)
{
    avio_flush(s->pb);
    return 0;
}

static int tta_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    avio_write(s->pb, pkt->data, pkt->size);
    return 0;
}

AVOutputFormat ff_tta_muxer = {
    .name              = "tta",
    .long_name         = NULL_IF_CONFIG_SMALL("True Audio"),
    .mime_type         = "audio/x-tta",
    .extensions        = "tta",
    .audio_codec       = CODEC_ID_TTA,
    .video_codec       = CODEC_ID_NONE,
    .write_header      = tta_write_header,
    .write_packet      = tta_write_packet,
    .write_trailer     = tta_write_trailer,
    .flags= AVFMT_NOTIMESTAMPS,
};
