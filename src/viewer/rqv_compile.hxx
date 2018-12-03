#pragma once
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/viewer/rqv_node_base.hxx"

#include <memory>
#include <vector>
#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace rqv {

using NodeList = std::vector<std::shared_ptr<NodeBase>>;

std::tuple<bool, NodeList> compile(const JsonValue& usernodes, rglv::MeshStore& meshstore);

bool link(NodeList& nodes);

}  // close package namespace
}  // close enterprise namespace
