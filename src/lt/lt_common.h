#pragma once

#include <math.h>
#include <stdint.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <vector>

#define LT_NAMESPACE lt

namespace LT_NAMESPACE {

    using vec3 = glm::vec3;
    using vec2 = glm::vec2;

using Spectrum = vec3;

using Float = float;

const Float pi = 3.14159265359;

template <typename T>
inline std::vector<T> linspace(T start, T end, size_t size, bool centered = true)
{
    std::vector<T> arr;
    arr.resize(size);
    if (size == 0)
        return arr;
    if (size == 1)
        return { start };

    //T delta = (end - start) / (size - 1);
    if (centered) {
        T delta = (end - start) / size;
        for (size_t i = 0; i < size; i++)
            arr[i] = start + delta * i + delta * 0.5;
    }
    else {
        T delta = (end - start) / (size - 1);
        for (size_t i = 0; i < size; i++)
            arr[i] = start + delta * i;
    }
    return arr;
}

inline vec3 polar_to_card(Float theta, Float phi)
{
    return vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

inline vec3 square_to_uniform_sphere(Float u1, Float u2)
{
    Float z = 1. - 2. * u1;
    Float r = std::sqrt(std::max(0., 1. - z * z));
    Float ph = 2. * pi * u2;
    return vec3(r * cos(ph), r * sin(ph), z);
}

inline Float square_to_uniform_sphere_pdf() { return 1. / (4. * pi); }

inline vec3 square_to_uniform_hemisphere(Float u1, Float u2)
{
    Float z = u1;
    Float r = std::sqrt(std::max(0., 1. - z * z));
    Float ph = 2. * pi * u2;
    return vec3(r * cos(ph), r * sin(ph), z);
}

inline Float square_to_uniform_hemisphere_pdf() { return 1. / (2. * pi); }

inline vec3 square_to_cosine_hemisphere(Float u1, Float u2)
{
#if 0
    Float cos_theta = std::sqrt(u1);
    Float sin_theta = std::sqrt(glm::clamp(1.f - cos_theta * cos_theta,0.f,0.99999f));
    Float phi = 2. * pi * u2;
    Float x = sin_theta * std::cos(phi);
    Float y = sin_theta * std::sin(phi);
    return vec3(x, y, cos_theta);
#endif
#if 1
    Float r = std::sqrt(u1);
    Float theta = 2. * pi * u2;
    Float dx = r * std::cos(theta);
    Float dy = r * std::sin(theta);
    Float z = std::sqrt(glm::clamp(1.f - dx * dx - dy * dy,0.00001f,1.f));
    if (z != z)
        std::cout << "ff" << std::endl;
    return vec3(dx, dy, z);
#endif
}

inline Float square_to_cosine_hemisphere_pdf(const vec3& w)
{
    return glm::clamp(w.z,0.f,1.f) / pi;
}

inline void orthonormal_basis(const vec3& n, vec3& t, vec3& b)
{
    if (n.z < -0.999999) {
        t = vec3(0, -1, 0);
        b = vec3(-1, 0, 0);
    } else {
        float c1 = 1. / (1. + n.z);
        float c2 = -n.x * n.y * c1;
        t = glm::normalize(vec3(1.f - n.x * n.x * c1, c2, -n.x));
        b = glm::normalize(vec3(c2, 1.f - n.y * n.y * c1, -n.y));
    }
}

inline Spectrum fresnelConductor(const Float& cosThetaI, const Spectrum& eta, const Spectrum& k) {
    /* Modified from "Optics" by K.D. Moeller, University Science Books, 1988 */

    Float cosThetaI2 = cosThetaI * cosThetaI,
        sinThetaI2 = 1 - cosThetaI2,
        sinThetaI4 = sinThetaI2 * sinThetaI2;

    Spectrum temp1 = eta * eta - k * k - sinThetaI2;
    Spectrum a2pb2 = glm::sqrt(glm::max(temp1 * temp1 + 4.f * k * k * eta * eta, 0.00001f));
    Spectrum a = glm::sqrt(glm::max(0.5f * (a2pb2 + temp1), 0.00001f));

    Spectrum term1 = a2pb2 + cosThetaI2;
    Spectrum term2 = 2.f * a * cosThetaI;

    Spectrum Rs2 = (term1 - term2) / (term1 + term2);

    Spectrum term3 = a2pb2 * cosThetaI2 + sinThetaI4;
    Spectrum term4 = term2 * sinThetaI2;

    Spectrum Rp2 = Rs2 * (term3 - term4) / (term3 + term4);

    return 0.5f * (Rp2 + Rs2);
}

inline glm::mat3 build_tbn_from_w(const vec3& w)
{
    vec3 normal = w;
    float sign = copysignf(1.0f, normal.z);
    const float a = -1.0f / (sign + normal.z);
    const float b = normal.x * normal.y * a;
    vec3 tangent = vec3(1.0f + sign * normal.x * normal.x * a, sign * b, -sign * normal.x);
    vec3 bitangent = vec3(b, sign + normal.y * normal.y * a, -normal.y);
    return glm::mat3(tangent, bitangent, normal);
}

//template <class T>
//int binary_search(const std::vector<T>& arr, const T& u) {
//
//    int left = 0;
//    int right = arr.size() - 1;
//    int i = 1000;
//
//    while ((left <= right) && (i > 0))
//    {
//        int mid = left + (right - left) / 2;
//
//        if (arr[mid] <= u) {
//            if (arr[mid + 1] >= u)
//            {
//                return mid;
//            }
//            else
//            {
//                left = mid + 1;
//            }
//        }
//        else
//        {
//            right = mid - 1;
//        }
//
//    }
//    return left;
//    //return -1;
//
//}



template <class T>
int binary_search(const T* arr, const T& val, const int& size) {
    // edge case: value of smaller than min or larger than max
    if (arr[0] >= val) return 0;
    if (arr[size - 1] <= val) return size - 1;

    int start = 0;
    int end = size - 1;

    //while (start <= end) {
    while (end - start >= 1) {
        int mid = (end + start) / 2;

        // value is in interval from previous to current element
        if (val >= arr[mid] && val <= arr[mid+1]) {
            return mid ;
        }
        else {
            if (arr[mid] < val) {
                start = mid;
            }
            else {
                end = mid;
            }
        }
    }
    return -1;
}

template <class T>
int binary_search(const std::vector<T>& arr, const T& val) {
    return binary_search<T>(arr.data(), val, arr.size());
}

} // namespace LT_NAMESPACE
