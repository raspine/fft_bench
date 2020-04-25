#include <cstring>
#include <cassert>
#include <cmath>
#include <fftw3.h>
#include "kissfft/kiss_fft.h"
#include "ffts/include/ffts.h"
#include "ffts/src/ffts_attributes.h"
#define __STDC_CONSTANT_MACROS

#include "FftWrapper.hpp"
namespace {
	const size_t FftSize = 2048;
}

FftWrapper::FftWrapper(const FftProcessor Fft) {
	fftProcessor_ = Fft;
	if (Fft == FftProcessor::KISS) {
		m_plan = kiss_fft_alloc(FftSize, 0, NULL, NULL);
	}
	else if (Fft == FftProcessor::FFTW) {
		m_pFFTBuf = reinterpret_cast<std::complex<float>*>(
					fftwf_malloc(sizeof (std::complex<float>) * FftSize));
		m_plan = fftwf_plan_dft_1d(FftSize,
                                reinterpret_cast <fftwf_complex *>(m_pFFTBuf),
                                reinterpret_cast <fftwf_complex *>(m_pFFTBuf),
	                            FFTW_FORWARD, FFTW_ESTIMATE);

	}
	else if (Fft == FftProcessor::FFTS) {
		float FFTS_ALIGN(32) *input = reinterpret_cast<float*>(valloc(2 * FftSize * sizeof(float)));
		m_pFFTBuf = reinterpret_cast<std::complex<float>*>(input);
		float FFTS_ALIGN(32) *output = reinterpret_cast<float*>(valloc(2 * FftSize * sizeof(float)));
		m_pFFTOutBuf = reinterpret_cast<std::complex<float>*>(output);
		m_plan = ffts_init_1d(FftSize, FFTS_FORWARD);
	}
}

FftWrapper::~FftWrapper () {
	if (fftProcessor_ == FftProcessor::KISS) {
		kiss_fft_free(m_plan);
	}
	else if (fftProcessor_ == FftProcessor::FFTW) {
	   fftwf_destroy_plan(reinterpret_cast<fftwf_plan>(m_plan));
	   fftwf_free(m_pFFTBuf);
	}
	else if (fftProcessor_ == FftProcessor::FFTS) {
		ffts_free(reinterpret_cast<ffts_plan_t*>(m_plan));
		free(m_pFFTBuf);
		free(m_pFFTOutBuf);
	}
}

void FftWrapper::doFftFloat(const std::complex<float>* const pInBuf,
		std::complex<float>* const pOutBuf) {
	if (fftProcessor_ == FftProcessor::KISS) {
		kiss_fft(
				reinterpret_cast<kiss_fft_cfg>(m_plan),
				reinterpret_cast<const kiss_fft_cpx*>(pInBuf),
				reinterpret_cast<kiss_fft_cpx*>(pOutBuf)
				);
	}
	else if (fftProcessor_ == FftProcessor::FFTW) {
		std::memcpy(m_pFFTBuf, pInBuf, sizeof(std::complex<float>) * FftSize);
		fftwf_execute(reinterpret_cast<fftwf_plan>(m_plan));
		std::memcpy(pOutBuf, m_pFFTBuf, sizeof(std::complex<float>) * FftSize);
	}
	else if (fftProcessor_ == FftProcessor::FFTS) {
		std::memcpy(m_pFFTBuf, pInBuf, sizeof(std::complex<float>) * FftSize);
		ffts_execute(reinterpret_cast<ffts_plan_t*>(m_plan), reinterpret_cast<void*>(m_pFFTBuf), reinterpret_cast<void*>(m_pFFTOutBuf));
	}
	else throw std::runtime_error("not implemented");
}


IFftWrapper::IFftWrapper(const FftProcessor Fft) {
	fftProcessor_ = Fft;
	if (Fft == FftProcessor::KISS) {
		m_plan = kiss_fft_alloc(FftSize, 1, NULL, NULL);
	}
	else if (Fft == FftProcessor::FFTW) {
		m_pFFTBuf = reinterpret_cast<std::complex<float>*>(
					fftwf_malloc(sizeof (std::complex<float>) * FftSize));
		m_plan = fftwf_plan_dft_1d(FftSize,
                                reinterpret_cast <fftwf_complex *>(m_pFFTBuf),
                                reinterpret_cast <fftwf_complex *>(m_pFFTBuf),
	                            FFTW_BACKWARD, FFTW_ESTIMATE);

	}
	else if (Fft == FftProcessor::FFTS) {
		float FFTS_ALIGN(32) *input = reinterpret_cast<float*>(valloc(2 * FftSize * sizeof(float)));
		m_pFFTBuf = reinterpret_cast<std::complex<float>*>(input);
		float FFTS_ALIGN(32) *output = reinterpret_cast<float*>(valloc(2 * FftSize * sizeof(float)));
		m_pFFTOutBuf = reinterpret_cast<std::complex<float>*>(output);
		m_plan = ffts_init_1d(FftSize, FFTS_BACKWARD);
	}
}

IFftWrapper::~IFftWrapper () {
	if (fftProcessor_ == FftProcessor::KISS) {
		kiss_fft_free(m_plan);
	}
	else if (fftProcessor_ == FftProcessor::FFTW) {
	   fftwf_destroy_plan(reinterpret_cast<fftwf_plan>(m_plan));
	   fftwf_free(m_pFFTBuf);
	}
	else if (fftProcessor_ == FftProcessor::FFTS) {
		ffts_free(reinterpret_cast<ffts_plan_t*>(m_plan));
		free(m_pFFTBuf);
		free(m_pFFTOutBuf);
	}
}

void IFftWrapper::doIfftFloat(const std::complex<float>* const pInBuf,
		std::complex<float>* const pOutBuf) {

	if (fftProcessor_ == FftProcessor::KISS) {
		kiss_fft(
				reinterpret_cast<kiss_fft_cfg>(m_plan),
				reinterpret_cast<const kiss_fft_cpx*>(pInBuf),
				reinterpret_cast<kiss_fft_cpx*>(pOutBuf)
				);
	}
	else if (fftProcessor_ == FftProcessor::FFTW) {
		std::memcpy(m_pFFTBuf, pInBuf, sizeof(std::complex<float>) * FftSize);
		fftwf_execute(reinterpret_cast<fftwf_plan>(m_plan));
		std::memcpy(pOutBuf, m_pFFTBuf, sizeof(std::complex<float>) * FftSize);
	}
	else if (fftProcessor_ == FftProcessor::FFTS) {
		std::memcpy(m_pFFTBuf, pInBuf, sizeof(std::complex<float>) * FftSize);
		ffts_execute(reinterpret_cast<ffts_plan_t*>(m_plan), reinterpret_cast<void*>(m_pFFTBuf), reinterpret_cast<void*>(m_pFFTOutBuf));
	}
	else throw std::runtime_error("not implemented");

	// scale
	const float factor = 1.0f / FftSize;
	for (size_t i = 0; i < FftSize; i++) {
	   pOutBuf[i] *= factor;
	}
}

