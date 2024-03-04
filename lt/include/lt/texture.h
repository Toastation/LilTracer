#pragma once
#include <lt/lt_common.h>
#include <lt/surface_interaction.h>

namespace LT_NAMESPACE {

struct Texture {
	bool init;
	size_t w;
	size_t h;
	Spectrum* data;

	Texture() : init(false), data(nullptr), w(0), h(0) {}
	Texture(const size_t& w, const size_t& h) :
		init(false),
		data(nullptr),
		w(w),
		h(h)
	{
		initialize();
	}

	~Texture() {
		if (init) {
			delete[] data;
			data = nullptr;
		}
	}

	void initialize() {
		data = new Spectrum[w * h];
		init = true;
	}

	void set(const size_t& x, const size_t& y, const Spectrum& s) {
		data[x * h + y] = s;
	}

	Spectrum get(const size_t& x, const size_t& y) {
		return data[x * h + y];
	}

	Spectrum eval(const SurfaceInteraction& si) {
		size_t x = std::max(std::min(size_t(si.u * (Float)w), w - 1), (size_t)0);
		size_t y = std::max(std::min(size_t(si.v * (Float)h), h - 1), (size_t)0);
		return get(x,y);
	}

};

}  // namespace LT_NAMESPACE