#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "kiss_fftr.h"

using namespace std;

TEST(FFTTest, DiracImpulseTest) {
  int fft_len = 64;

  kiss_fftr_cfg forward_fft = kiss_fftr_alloc(fft_len, 0, 0, 0);
  kiss_fftr_cfg inverse_fft = kiss_fftr_alloc(fft_len, 1, 0, 0);

  vector<float> test_signal(fft_len, 0.0f);
  test_signal[fft_len / 2] = 1.0f;

  vector<kiss_fft_scalar> fft_time_domain_buffer(fft_len);
  vector<kiss_fft_cpx> fft_freq_domain_buffer(fft_len / 2 + 1);

  for (int i = 0; i < fft_len; ++i) {
    fft_time_domain_buffer[i] = i;
  }

  kiss_fftr(forward_fft, &fft_time_domain_buffer[0],
            &fft_freq_domain_buffer[0]);
  kiss_fftri(inverse_fft, &fft_freq_domain_buffer[0],
             &fft_time_domain_buffer[0]);

  // Invert FFT scaling
  for (int i = 0; i < fft_len; ++i) {
    fft_time_domain_buffer[i] /= fft_len;
  }

  for (int i = 0; i < fft_len; ++i) {
    EXPECT_NEAR(fft_time_domain_buffer[i], i, 1e-4);
  }

  kiss_fft_free(forward_fft);
  kiss_fft_free(inverse_fft);
}
