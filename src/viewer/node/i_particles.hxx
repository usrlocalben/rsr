#pragma once

#include "base.hxx"

namespace rqdq {
namespace rqv {

struct ParticlePtrs {
	int many;
	const float *px, *py, *pz;
	const float *sz;
	const float *lr; };


class IParticles : public virtual NodeBase {
public:
	virtual auto GetPtrs() const -> ParticlePtrs = 0; };


}  // close package namespace
}  // close enterprise namespace
