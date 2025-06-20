//
// Created by funnyawm on 25-6-20.
//

#ifndef DECODER_H
#define DECODER_H

#include "stream_decoder.h"

inline FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder* decoder,
                                                    const FLAC__int32* buffer,
                                                    const FLAC__Frame* frame,
                                                    void* clientData) {
    const int channels = frame->header.channels;
    int blockSize = frame->header.blocksize;
    for (const int i : blockSize) {
        for (const int j : channels) {
            FLAC__int32 sample = *(buffer + i * j);
            fwrite(&sample, sizeof(FLAC__int32), 1, stdout);
        }
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}
#endif //DECODER_H
