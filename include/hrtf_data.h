#ifndef HRTF_DATA_H_
#define HRTF_DATA_H_

#include <string>

#ifdef MIT_KEMAR
#include "mit_kemar_hrtf_data.h"
#endif

struct HRTFDataT {
  // Data set identifier
  const std::string identifier;

  // Number of HRTF sample points in polar coordinates
  const int num_hrtfs;

  // Length of HRTF FIRs
  const int fir_length;

  const int sample_rate;

  // Orientations as {elevation, azimuth} pairs.
  const int (*direction)[2];

  // Distance of sound source in meters.
  const float distance;

  // HRTF data
  const short (*data)[2][kHRTFFilterLen];
};

static const HRTFDataT kHRTFDataSet = { kHRTFDataIdentifier, kHRTFNum,
    kHRTFFilterLen, kHRTFSampleRate, kHRTFDirection, kDistanceMeter, kHRTFData };

#endif
