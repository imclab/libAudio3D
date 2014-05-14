#ifndef FFT_FILTER_IMPL_H_
#define FFT_FILTER_IMPL_H_
#include <vector>

#include "kiss_fftr.h"

using std::vector;

class FFTFilterImpl {
public:
	FFTFilterImpl(int filter_len);
	virtual ~FFTFilterImpl();

	void SetTimeDomainKernel(const vector<float>& kernel);
	void AddTimeDomainKernel(const vector<float>& kernel);

	void SetFreqDomainKernel(const vector<float>& kernel);
	void AddFreqDomainKernel(const vector<float>& kernel);

	void ForwardTransform(const vector<float>& time_signal, vector<float>* freq_signal) const;
	void InverseTransform(const vector<float>& freq_signal, vector<float>* time_signal) const;

	void AddSignalBlock(const vector<float>& signal_block);

	void GetResult(vector<float>* signal_block);


private:
	void Init();

	void ComplexVectorProduct(const vector<kiss_fft_cpx>& input_a,
			const vector<kiss_fft_cpx>& input_b, vector<kiss_fft_cpx>* result ) const;

	void VectorCopyWithZeroPadding(const vector<kiss_fft_scalar>& input,
			vector<kiss_fft_scalar>* output) const;

	void InverseFFTScaling(vector<float>* signal) const;

	int max_kernel_len_;
	int fft_len_;

	vector<float> filter_state_;
	vector<float> window_;

	bool kernel_defined_;
	vector<kiss_fft_scalar> kernel_time_domain_buffer_;
	vector<kiss_fft_cpx> kernel_freq_domain_buffer_;

	int buffer_selector_;
	vector< vector<kiss_fft_scalar> > signal_time_domain_buffer_;
	vector< vector<kiss_fft_cpx> > signal_freq_domain_buffer_;

	vector<kiss_fft_cpx> filtered_freq_domain_buffer_;

	kiss_fftr_cfg forward_fft_;
	kiss_fftr_cfg inverse_fft_;
};

#endif  // FFT_FILTER_IMPL_H_
