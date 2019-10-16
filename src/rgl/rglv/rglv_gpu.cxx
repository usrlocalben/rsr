#include "src/rgl/rglv/rglv_gpu.hxx"

namespace rqdq {
namespace rglv {

/**
 * global toggle for double-buffer operation of _all_ gpu
 * instances.
 *
 * may only be updated when all gpu instances are idle.
 */
bool doubleBuffer{true};


}  // namespace rglv
}  // namespace rqdq
