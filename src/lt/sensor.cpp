#include <lt/sensor.h>

namespace LT_NAMESPACE {

/////////////////////
// Sensor Factory
///////////////////

template<>
Factory<Sensor>::CreatorRegistry& Factory<Sensor>::registry()
{
    static Factory<Sensor>::CreatorRegistry registry{
        { "Sensor", std::make_shared<Sensor> },
        { "VarianceSensor", std::make_shared<VarianceSensor> },
        { "HemisphereSensor", std::make_shared<HemisphereSensor> }
    };
    return registry;
}

void Sensor::init() {
    value.resize(w * h);
    acculumator.resize(w * h);
    count.resize(w * h);
    u = linspace<Float>(-1, 1, w);
    v = linspace<Float>(1, -1, h);
    sum_counts = 0;
}

/**
    * @brief Resets the sensor data.
    *
    * This function resets the accumulator, value, and count arrays of the
    * sensor.
    */
void Sensor::reset()
{
    memset(acculumator.data(), 0, sizeof(Spectrum) * acculumator.size());
    memset(value.data(), 0, sizeof(Spectrum) * value.size());
    memset(count.data(), 0, sizeof(uint16_t) * count.size());
    sum_counts = 0;
}

/**
    * @brief Adds a sample to the sensor data.
    *
    * @param x The x-coordinate of the sample.
    * @param y The y-coordinate of the sample.
    * @param s The spectrum of the sample.
    */
void Sensor::add(const uint32_t& x, const uint32_t& y, Spectrum s)
{
    uint32_t idx = y * w + x;
    acculumator[idx] += s;
    count[idx]++;
    sum_counts++;
    set_value(idx,y);
}

/**
    * @brief Sets a sample in the sensor data.
    *
    * @param x The x-coordinate of the sample.
    * @param y The y-coordinate of the sample.
    * @param s The spectrum of the sample.
    */
void Sensor::set(const uint32_t& x, const uint32_t& y, Spectrum s)
{
    uint32_t idx = y * w + x;
    acculumator[idx] = s;
    count[idx] = 1;
    set_value(idx,y);
}

Spectrum Sensor::get(const uint32_t& x, const uint32_t& y) {
    return value[y * w + x];
}

/**
    * @brief Gets the number of samples at a specific pixel.
    *
    * @param x The x-coordinate of the pixel.
    * @param y The y-coordinate of the pixel.
    * @return The number of samples at the specified pixel.
    */
uint16_t Sensor::n_sample(const uint32_t& x, const uint32_t& y)
{
    return count[y * w + x];
}

void Sensor::set_value(const uint32_t& idx, const uint32_t& x){
    value[idx] = acculumator[idx] / (Float)count[idx];
    assert(value[idx] == value[idx]);
}



void HemisphereSensor::init() {
    Sensor::init();

    dtheta = 0.5 * pi / Float(h);
    dphi = 2 * pi / Float(w);

    std::vector<float> th = lt::linspace<float>(0, 0.5 * lt::pi, h);

    solid_angle.resize(h);
    for (int i = 0; i < h; i++)
        solid_angle[i] = std::sin(th[i]) * dtheta * dphi;

}

void HemisphereSensor::set_value(const uint32_t& idx, const uint32_t& y) {
    Float norm = sum_counts * solid_angle[y];
    value[idx] = acculumator[idx] / norm;
}

}
