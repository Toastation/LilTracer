#include "test.h"

namespace LT_NAMESPACE {


/////////////////////
// TestBrdf 
///////////////////
Spectrum TestBrdf::eval(vec3 wi, vec3 wo, Sampler& sampler)
{
    return Spectrum(wo.z);
}


} // namespace LT_NAMESPACE