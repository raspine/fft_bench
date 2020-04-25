#pragma once

#include <complex>

enum class FftProcessor { KISS, FFTW, FFTS };

class FftWrapper final {
public:
	explicit FftWrapper(const FftProcessor Fft);
	~FftWrapper(void);
	void doFftFloat(const std::complex<float>* const pInBuf,
			std::complex<float>* const pOutBuf);

private:
	std::complex<float>* m_pFFTBuf = nullptr;
	std::complex<float>* m_pFFTOutBuf = nullptr;
	FftProcessor fftProcessor_;
	void* m_plan;
};

class IFftWrapper
{
public:
	IFftWrapper(const FftProcessor Fft);
	~IFftWrapper(void);
	void doIfftFloat(const std::complex<float>* const pInBuf,
			std::complex<float>* const pOutBuf);

private:
	std::complex<float>* m_pFFTBuf = nullptr;
	std::complex<float>* m_pFFTOutBuf = nullptr;
	FftProcessor fftProcessor_;
	void* m_plan;
};

