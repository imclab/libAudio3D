#include <cmath>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "fft_filter.h"

using namespace std;

TEST(FFTFilterTest, DiracImpulseTest) {
  int filter_size = 8;
  int signal_size = filter_size * 4;

  FFTFilter fft_filter(filter_size);

  vector<float> kernel(filter_size, 0.0f);
  // Construct dirac impulse.
  kernel[filter_size / 2] = 1.0;

  fft_filter.SetTimeDomainKernel(kernel);

  // Test signal
  float signal_val = 0.0f;

  vector<float> signal_block;
  signal_block.reserve(filter_size);

  vector<float> filtered_block;
  signal_block.reserve(filter_size);

  vector<float> filtered_signal;
  signal_block.reserve(signal_size);

  for (int i = 0; i < signal_size; ++i) {
    signal_block.push_back(signal_val);
    signal_val += 1.0f;
    if (signal_block.size() == filter_size) {
      fft_filter.AddSignalBlock(signal_block);
      fft_filter.GetResult(&filtered_block);
      filtered_signal.insert(filtered_signal.end(), filtered_block.begin(),
                             filtered_block.end());
      signal_block.clear();
    }
  }

  for (int i = 0; i < filter_size / 2; ++i) {
    // Dirac-based shift contains zeros.
    EXPECT_NEAR(filtered_signal[i], 0.0f, 1e-5);
  }
  for (int i = filter_size / 2; i < signal_size; ++i) {
    // Dirac-based shift contains zeros.
    EXPECT_NEAR(filtered_signal[i], i-filter_size/2, 1e-5);
  }
}

TEST(FFTFilterTest, TransformTest) {
  int filter_size = 10;
  int fft_size = 16;

  FFTFilter fft_filter(fft_size);

  vector<float> time_domain_kernel(filter_size);
  for (int i=0; i<time_domain_kernel.size(); ++i) {
	  time_domain_kernel[i] = sin(i);  // some floats
  }
  vector<float> freq_domain_kernel;
  fft_filter.ForwardTransform(time_domain_kernel, &freq_domain_kernel);
  vector<float> inv_domain_kernel;
  fft_filter.InverseTransform(freq_domain_kernel, &inv_domain_kernel);

  EXPECT_EQ(fft_size * 2, inv_domain_kernel.size());
  for (int i = 0; i < filter_size; ++i) {
    EXPECT_NEAR(inv_domain_kernel[i], sin(i), 1e-5);
  }
}

