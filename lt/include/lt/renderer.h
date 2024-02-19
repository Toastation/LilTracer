#pragma once 
#include <lt/lt_common.h>
#include <lt/sensor.h>
#include <lt/sampler.h>
#include <lt/camera.h>
#include <lt/integrator.h>

namespace LT_NAMESPACE {
	
	class Renderer
	{
	public:
		std::shared_ptr<Sampler> sampler;
		std::shared_ptr<Sensor> sensor;
		std::shared_ptr<Camera> camera;
		std::shared_ptr<Integrator> integrator;
	};


}