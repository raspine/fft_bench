#include "catch.hpp"
#include <complex>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <chrono>
#include <fftw3.h>
#include "kissfft/kiss_fft.h"
#include "ffts/src/ffts_attributes.h"
#include "ffts/include/ffts.h"

namespace {

	bool EnablePrint = false; // enable to get printouts

	// Sums sines of freqs[] into a composite sinussoid
	template <typename T, size_t size>
	std::array<std::complex<T>, size>
	createSinusoid(const std::vector<int>& freqs) {

		std::array<std::complex<T>, size> ret;
		std::array<std::complex<T>, size> tmp;

		for(size_t i=0; i < size; i++) {
			ret[i] = std::complex<T>(0, 0);
			for(size_t j = 0; j < freqs.size(); j++) {
				tmp[i] += std::complex<T>(cos(2*M_PI*freqs[j] * i/size),
						sin(2*M_PI*freqs[j] * i/size));
			}
			ret[i] = tmp[i];
		}
		return ret;
	}

	void printComplex(const std::complex<float>* const val, const std::string& desc) {

		if (EnablePrint) {
			std::cout << desc << std::endl;
			for (int i = 0; i < 16; i++) {
				std::cout <<
					"Re: " << std::round(val[i].real() * 5) / 5 << "\t" <<
					" Im: " << std::round(val[i].imag() * 5) / 5  << "\t" <<
					std::endl;
			}
		}
	}
}

TEST_CASE("kiss_fft16 float", "[fft]") {
	std::vector<int> freqs = {2, 5};
	auto sinusoid = createSinusoid<float, 16>(freqs);
	decltype(sinusoid) outBuf;

	auto plan = kiss_fft_alloc(16, 0, NULL, NULL);

	kiss_fft(plan,
			reinterpret_cast<const kiss_fft_cpx*>(sinusoid.begin()),
			reinterpret_cast<kiss_fft_cpx*>(outBuf.begin()));

	printComplex(outBuf.begin(), "KissFft float result");
	kiss_fft_free(plan);
}

TEST_CASE("FFTW16 float", "[fft]") {
	std::vector<int> freqs = {2, 5};
	auto sinusoid = createSinusoid<float, 16>(freqs);
	printComplex(sinusoid.begin(), "sinusoid");

	std::complex<float>* pFFTBuf;
	pFFTBuf = reinterpret_cast<std::complex<float>*>(
						fftwf_malloc(sizeof (std::complex<float>) * 16));
	auto plan = fftwf_plan_dft_1d(16,
							reinterpret_cast <fftwf_complex *>(pFFTBuf),
							reinterpret_cast <fftwf_complex *>(pFFTBuf),
							FFTW_FORWARD, FFTW_ESTIMATE);
	std::memcpy(pFFTBuf, sinusoid.begin(), sizeof(std::complex<float>) * 16);

	fftwf_execute(plan);
	printComplex(pFFTBuf, "FFTW8 float result");
	fftwf_destroy_plan(plan);

	plan = fftwf_plan_dft_1d(16,
							reinterpret_cast <fftwf_complex *>(pFFTBuf),
							reinterpret_cast <fftwf_complex *>(pFFTBuf),
							FFTW_BACKWARD, FFTW_ESTIMATE);

	fftwf_execute(plan);
	printComplex(pFFTBuf, "FFTW16 float result");
	fftwf_destroy_plan(plan);
	fftwf_free(pFFTBuf);
}

TEST_CASE("ffts16 float", "[fft]") {
	std::vector<int> freqs = {2, 5};
	auto sinusoid = createSinusoid<float, 16>(freqs);
	decltype(sinusoid) outBuf;

	float FFTS_ALIGN(32) *pFFTBuf = reinterpret_cast<float*>(valloc(2 * 16 * sizeof(float)));
	// auto pFFTBuf = av_malloc(sizeof(std::complex<float>) * sinusoid.size());
	std::memcpy(pFFTBuf, sinusoid.begin(), sizeof(std::complex<float>) * sinusoid.size());
	auto plan = ffts_init_1d(sinusoid.size(), FFTS_FORWARD);
	ffts_execute(plan, reinterpret_cast<void*>(pFFTBuf), reinterpret_cast<void*>(outBuf.begin()));

	printComplex(reinterpret_cast<std::complex<float>*>(outBuf.begin()), "ffts16 float result");
	free(pFFTBuf);
	ffts_free(plan);
}

