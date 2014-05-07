
#include <cstdint>
#include <vector>
class FFTFilter;
class HRTF;

class Audio3DSource {
public:
	Audio3DSource(int sample_rate, int block_size);
	virtual ~Audio3DSource();

	void SetPosition(int x, int y, int z);
	void SetDirection(float elevation_deg, float azimuth_deg, float distance);

	void ProcessBlock(const std::vector<float>&input,
			std::vector<float>* output_left,
			std::vector<float>* output_right);
private:
	void CalculateXFadeWindow();
	void ApplyXFadeWindow(const std::vector<float>& block_a,
			const std::vector<float>& block_b, std::vector<float>* output);

	static void ApplyDamping(float damping_factor, std::vector<float>* block);
	const int sample_rate_;
	const int block_size_;
	float elevation_deg_;
	float azimuth_deg_;
	float distance_;

	float damping_;

	std::vector<float> xfade_window_;
	std::vector<float> prev_signal_block_;

	HRTF* hrtf_;
		FFTFilter* left_fft_filter_;
		FFTFilter* right_fft_filter_;
};

/*
 * Hann window*
 * for (int i = 0; i < 2048; i++) {
    double multiplier = 0.5 * (1 - cos(2*PI*i/2047));
    dataOut[i] = multiplier * dataIn[i];
}
 */
