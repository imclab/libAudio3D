#ifndef CONFIG_H_
#define CONFIG_H_

struct Audio3DConfigT {
  int sample_rate;
  int block_size;
  float distance_resolution_meter;
  float max_distance_meter;
};
static const Audio3DConfigT config = {
    44100,  // sample rate
    2048,   // block size
    0.1,    // distance resolution
    25.0    // maximum distance
};


#endif CONFIG_H_
