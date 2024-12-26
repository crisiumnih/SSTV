#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <jpeglib.h>
#include <string.h>
#include <assert.h>
/*       IMAGE         */  

#define HEIGHT 240
#define WIDTH 320

typedef float Image[HEIGHT][WIDTH][3];

Image image;
Image freq_image;

/*       IMAGE         */  



/*       AUDIO         */  

struct wav_header
{
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

struct wav_header wavh;

#define sample_rate 11025
#define duration_seconds 114.285
#define buffer_size  (int)(sample_rate * duration_seconds)
short int *buffer= NULL;

const int header_length = sizeof(struct wav_header);

/*       AUDIO         */  

void ReadImage(char *filename, Image img){
    FILE *origin;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int stride, x, y, c, idx;
    JSAMPARRAY buffer;

    if ((origin= fopen(filename, "rb"))==NULL){
        fprintf(stderr, "Cannot open %s\n", filename);
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, origin);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);

    if (cinfo.image_width != WIDTH || cinfo.image_height != HEIGHT) {
	      fprintf(stderr, "image size is %dx%d, should be %dx%d.\n",
	      cinfo.image_width, cinfo.image_height, WIDTH, HEIGHT) ;
	      
    }

    assert(cinfo.output_components == 3) ;

    stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        y = cinfo.output_scanline ;
	      (void) jpeg_read_scanlines(&cinfo, buffer, 1);
	      /* copy out the data into the image */
        for (x=0, idx=0; x<cinfo.output_width; x++) {
	          for(c=0; c<3; c++, idx++){
                img[y][x][c] = buffer[0][idx]/(float)MAXJSAMPLE ;
                //printf("Raw data: %f\n", img[y][x][c]);
            }
        }
    }
    

    
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(origin);
}

static int buffer_position = 0;

void pulse(double freq, float time_ms) {
    int num_samples = (int)(sample_rate * time_ms / 1000.0);
    
    if (buffer_position + num_samples >= buffer_size) {
        printf("Warning: Buffer overflow prevented at position %d\n", buffer_position);
        return;
    }
    
    // Generate samples at the current buffer position
    for (int i = 0; i < num_samples; i++) {
        double sample = cos(2 * M_PI * freq * i / sample_rate) * 1000 ;  
        
        buffer[buffer_position + i] = (short int)sample;
    }
    
    buffer_position += num_samples; 
}
void VIS(){
    pulse(1900, 300);
    pulse(1200, 10);
    pulse(1900, 300);
    pulse(1200, 30);   // Start bit
    // Code for Martin M1 - 10101100 
    // Encoding starts from LSB
    pulse(1300, 10);   // 0 
    pulse(1300, 10);   // 0
    pulse(1100, 10);   // 1 
    pulse(1100, 10);   // 1 
    pulse(1300, 10);   // 0
    pulse(1100, 10);   // 1 
    pulse(1300, 10);   // 0
    pulse(1100, 10);   // 1  Parity , Even in case of SSTV
    pulse(1200, 10);   // Stop bit
    
    printf("VIS done. \n");
}

void process_image_to_freq(Image img){
    int fullblack=1500;
    for (int i=0; i<HEIGHT; i++){
        for (int j=0; j<WIDTH; j++){
            for(int k=0;k<3;k++){
                
                float pixel_value = fmax(0.0, fmin(1.0, img[i][j][k]));
                
                // Calculate frequency with bounds
                float freq = pixel_value * 800.0 + 1500.0;
                
                freq_image[i][j][k] = fmin(freq, 2300.0);

            }  
        }
    }
}

void image_signal(){

    double pixel_duration = 146.432 / WIDTH;

       
    for (int i = 0; i < HEIGHT; i++) {
        // Sync signal before each channel (1200 Hz pulse for 4.862ms)
        pulse(1200, 4.862);  // Sync signal 

        for (int j = 0; j < WIDTH; j++) {
            double freq = freq_image[i][j][1];  
            pulse(freq, pixel_duration); 
        }
        
        // Gap (1500 Hz, 0.572 ms)
        pulse(1500, 0.572);

        for (int j = 0; j < WIDTH; j++) {
            double freq = freq_image[i][j][2];  
            pulse(freq, pixel_duration); 
        }

        pulse(1500, 0.572);

        for (int j = 0; j < WIDTH; j++) {
            double freq = freq_image[i][j][0];  
            pulse(freq, pixel_duration); 
        }

    } 

    printf("Done image signal.\n");

}
void main(){

    buffer = (short int*)calloc(buffer_size, sizeof(short int));
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate buffer\n");
        return;
    }



    strncpy(wavh.riff, "RIFF", 4);
    strncpy(wavh.wave, "WAVE", 4);
    strncpy(wavh.fmt, "fmt ", 4);
    strncpy(wavh.data, "data", 4);

    wavh.chunk_size = 16;
    wavh.format_tag = 1;
    wavh.num_chans = 1;
    wavh.srate = sample_rate;
    wavh.bits_per_samp = 16;
    wavh.bytes_per_sec = wavh.srate * wavh.bits_per_samp / 8 * wavh.num_chans;
    wavh.bytes_per_samp = wavh.bits_per_samp / 8 * wavh.num_chans;

    buffer_position = 0;  // Reset position before starting
    memset(buffer, 0, buffer_size * sizeof(short int));
    
    ReadImage("sample.jpeg", image);
    /*for (int i=0; i<HEIGHT; i++){*/
    /*    for (int j=0; j<WIDTH; j++){*/
    /*        for(int k=0;k<3;k++){*/
    /**/
    /*            printf("Raw data : %f\n", image[i][j][k]);*/
    /*        }  */
    /*    }*/
    /*}*/

    process_image_to_freq(image);
    VIS();
    image_signal();
    

    wavh.dlength = buffer_size * wavh.bytes_per_samp;
    wavh.flength = wavh.dlength + header_length;

    FILE *fp = fopen("test.wav", "w");
    fwrite(&wavh, 1, header_length, fp);
    fwrite(buffer, 2, buffer_size, fp);

    free(buffer);
}
