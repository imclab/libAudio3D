#ifndef CONFIG_H_
#define CONFIG_H_

struct Audio3DConfigT {
  int sample_rate;
  int block_size;
  float distance_resolution_meter;
  float max_distance_meter;
  int reflection_order;
};
static const Audio3DConfigT config = {
    44100,  // sample rate
    2048,   // block size
    0.1,    // distance resolution
    25.0,   // maximum distance
    2       //reflection_order
};


#endif CONFIG_H_
