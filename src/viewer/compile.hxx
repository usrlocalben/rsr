#pragma once
#include <memory>
#include <vector>

#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/viewer/node/base.hxx"

#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace rqv {

using NodeList = std::vector<std::shared_ptr<NodeBase>>;

std::tuple<bool, NodeList> compile(const JsonValue& usernodes, rglv::MeshStore& meshstore);

bool link(NodeList& nodes);


}  // namespace rqv
}  // namespace rqdq
