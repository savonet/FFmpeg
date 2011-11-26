#include "libavutil/opt.h"
#include "avcodec.h"
#include "tta.h"

typedef struct TTAEncodeContext {
    int channels;

    TTAFilter fst;
    TTARice rice;
    int last;
} TTAEncodeContext;

static void filter_init(TTAFilter *fs, int shift) {
    memset(fs, 0, sizeof(TTAFilter));
    fs->shift = shift;
    fs->round = 1 << (shift - 1);
}

static void rice_init(TTARice *rice, unsigned int k0, unsigned int k1)
{
    rice->k0 = k0;
    rice->k1 = k1;
    rice->sum0 = 0x1;
    rice->sum1 = 0x1;
}

static av_cold int tta_encode_init(AVCodecContext *avctx)
{
    TTAEncodeContext *s = avctx->priv_data;
    int i;
    
    for (i = 0; i < s->channels; i++) {
//        filter_init(&s[i].fst, flt_set[byte_size - 1]);
        rice_init(&s[i].rice, 10, 10);
        s[i].last = 0;
    }
    return 0;
}

static int tta_encode_frame(AVCodecContext *avctx, uint8_t *frame,
                            int buf_size, void *data)
{
    return 0;
}

static av_cold int tta_encode_close(AVCodecContext *avctx)
{
    TTAEncodeContext *s = avctx->priv_data;
    return 0;
}

static const AVOption options[] = {
};

AVCodec ff_tta_encoder = {
    .name           = "tta",
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = CODEC_ID_TTA,
    .priv_data_size = sizeof(TTAEncodeContext),
    .init           = tta_encode_init,
    .encode         = tta_encode_frame,
    .close          = tta_encode_close,
    .capabilities = CODEC_CAP_SMALL_LAST_FRAME | CODEC_CAP_LOSSLESS,
    .sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE},
    .long_name = NULL_IF_CONFIG_SMALL("True Audio (TTA)"),
};
