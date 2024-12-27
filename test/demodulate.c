#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#define sample_rate 11025

struct wav_header{
    char riff[4];           /* "RIFF"                                  */
    int32_t flength;        /* file length in bytes                    */
    char wave[4];           /* "WAVE"                                  */
    char fmt[4];            /* "fmt "                                  */
    int32_t chunk_size;     /* size of FMT chunk in bytes (usually 16) */
    int16_t format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
    int16_t num_chans;      /* 1=mono, 2=stereo                        */
    int32_t srate;          /* Sampling rate in samples per second     */
    int32_t bytes_per_sec;  /* bytes per second = srate*bytes_per_samp */
    int16_t bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
    int16_t bits_per_samp;  /* Number of bits per sample               */
    char data[4];           /* "data"                                  */
    int32_t dlength;        /* data length in bytes (filelength - 44)  */
};


typedef struct{
    uint32_t samples;
    int16_t *data;
} signal_t;

signal_t demodulate;

bool LoadWav(const char *filename, signal_t *signal){
    bool return_value = true;
    FILE *file;
        
    struct wav_header wavh;

    file = fopen(filename, "rb");
    if(file == NULL){
        printf("%s: Failed to open file.\n", filename);
        return false;
    }

    fread(wavh.riff, 1, 4, file);
    if(wavh.riff[0] != 'R' || wavh.riff[1] != 'I' || wavh.riff[2] != 'F' || wavh.riff[3] != 'F'){
        printf("%s: First 4 bytes should be \"RIFF\", are \"%4s\".\n", filename, wavh.riff);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(&wavh.flength, 4, 1, file);

    fread(wavh.wave, 1, 4, file);
    if(wavh.wave[0] != 'W' || wavh.wave[1] != 'A' || wavh.wave[2] != 'V' || wavh.wave[3] != 'E') {
        printf("%s: Next 4 bytes should be \"WAVE\", are \"%4s\".\n", filename, wavh.wave);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(wavh.fmt, 1, 4, file);
    if(wavh.fmt[0] != 'f' || wavh.fmt[1] != 'm' || wavh.fmt[2] != 't' || wavh.fmt[3] != ' ') {
        printf("%s: Next 4 bytes should be \"fmt \", are \"%4s\".\n", filename, wavh.fmt);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(&wavh.chunk_size, 4, 1, file);
    fread(&wavh.format_tag, 2, 1, file);
    if(wavh.format_tag != 1) {
        printf("%s: Format tag should be 1 (PCM), is %d.\n", filename, wavh.format_tag);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(&wavh.num_chans, 2, 1, file);
    if(wavh.num_chans != 1) {
        printf("%s: Number of channels should be 1 (mono), is %d.\n", filename, wavh.num_chans);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(&wavh.srate, 4, 1, file);
    if(wavh.srate != sample_rate) {
        printf("%s: Sample rate should be 44100, is %d.\n", filename, wavh.srate);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(&wavh.bytes_per_sec, 4, 1, file);
    fread(&wavh.bytes_per_samp, 2, 1, file);
    fread(&wavh.bits_per_samp, 2, 1, file);
    if(wavh.bits_per_samp != 16) {
        printf("%s: Bits per sample should be 16, is %d.\n", filename, wavh.bits_per_samp);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(wavh.data, 1, 4, file);
    if(wavh.data[0] != 'd' || wavh.data[1] != 'a' || wavh.data[2] != 't' || wavh.data[3] != 'a') {
        printf("%s: Next 4 bytes should be \"data\", are \"%4s\".\n", filename, wavh.data);
        return_value = false;
        goto CLOSE_FILE;
    }

    fread(&wavh.dlength, 4, 1, file);

    signal->data = malloc(wavh.dlength);
    if(signal->data == NULL) {
        printf("%s: Failed to allocate %d bytes for data.\n", filename, wavh.dlength);
        return_value = false;
        goto CLOSE_FILE;
    }

    if(fread(signal->data, 1, wavh.dlength, file) != wavh.dlength) {
        printf("%s: Failed to read data bytes.\n", filename);
        free(signal->data);
        return_value = false;
        goto CLOSE_FILE;
    }

    signal->samples = wavh.dlength / 2;



    CLOSE_FILE:
    fclose(file);
    
    return return_value;

}

void main(){
    if(LoadWav("test.wav", &demodulate)) {
        printf("Successfully loaded WAV file\n");
        printf("Number of samples: %u\n", demodulate.samples);
        free(demodulate.data);
    }

}
