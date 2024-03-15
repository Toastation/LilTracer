/**
 * @file sensor.h
 * @brief Defines the Sensor class for handling sensor data.
 */

#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {

/**
 * @brief Class for handling sensor data.
 *
 * The Sensor class represents a sensor used to capture light data in a scene.
 * It provides functionality for resetting the sensor, adding samples, setting
 * samples, and retrieving the number of samples at a specific pixel.
 */
class Sensor {
public:
    /**
     * @brief Default constructor.
     */
    Sensor()
        : w(0)
        , h(0) {
        sum_counts = 0;
    };

    /**
     * @brief Parameterized constructor.
     * @param w Width of the sensor.
     * @param h Height of the sensor.
     */
    Sensor(const uint32_t& w, const uint32_t& h)
        : w(w)
        , h(h)
    {
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
    void reset()
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
    void add(const uint32_t& x, const uint32_t& y, Spectrum s)
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
    void set(const uint32_t& x, const uint32_t& y, Spectrum s)
    {
        uint32_t idx = y * w + x;
        acculumator[idx] = s;
        count[idx] = 1;
        set_value(idx,y);
    }

    Spectrum get(const uint32_t& x, const uint32_t& y) {
        return value[y * w + x];
    }

    /**
     * @brief Gets the number of samples at a specific pixel.
     *
     * @param x The x-coordinate of the pixel.
     * @param y The y-coordinate of the pixel.
     * @return The number of samples at the specified pixel.
     */
    uint16_t n_sample(const uint32_t& x, const uint32_t& y)
    {
        return count[y * w + x];
    }

    virtual void set_value(const uint32_t& idx, const uint32_t& x){
        value[idx] = acculumator[idx] / (Float)count[idx];
        //  Gamma correction
        value[idx] = glm::pow(value[idx], Spectrum(0.4545));
    }

    uint32_t w; /**< Width of the sensor. */
    uint32_t h; /**< Height of the sensor. */
    std::vector<Spectrum> acculumator; /**< Accumulator array for sensor samples. */
    std::vector<Spectrum> value; /**< Value array for sensor samples. */
    std::vector<uint16_t> count; /**< Count array for the number of samples at each pixel. */
    std::atomic<uint32_t> sum_counts;
    std::vector<Float> u; /**< Vector representing the u-coordinates of the sensor pixels. */
    std::vector<Float> v; /**< Vector representing the v-coordinates of the sensor pixels. */
};


class HemisphereSensor : public Sensor
{
public:

    HemisphereSensor() : Sensor() {
        dtheta = -1.;
        dphi = -1.;
    };
    HemisphereSensor(const uint32_t& w, const uint32_t& h) : Sensor(w, h) {
        dtheta = 0.5 * pi / Float(h);
        dphi = 2 * pi / Float(w);

        std::vector<float> th = lt::linspace<float>(0, 0.5 * lt::pi, h);
        
        solid_angle.resize(h);
        for (int i = 0; i < h; i++)
            solid_angle[i] = std::sin(th[i]) * dtheta * dphi;

    };

    void set_value(const uint32_t& idx, const uint32_t& y) {
        Float norm = sum_counts * solid_angle[y];
        value[idx] = acculumator[idx] / norm;
        //  Gamma correction
        value[idx] = glm::pow(value[idx], Spectrum(0.4545));
    }

    std::vector<Float> solid_angle; /**< Vector representing the u-coordinates of the sensor pixels. */
    Float dtheta;
    Float dphi;

};


} // namespace LT_NAMESPACE