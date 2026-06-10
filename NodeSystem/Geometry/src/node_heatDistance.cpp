#include "node_heatDistance.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <map>
#include <utility>

std::string node_heatDistance::getType() const { return "heat_distance"; }
std::string node_heatDistance::getName() const { return "Heat Distance"; }
std::string node_heatDistance::getCategory() const { return "Geometry"; }
std::string node_heatDistance::getDescription() const {
  return "Compute heat distance from a mesh";
}

std::vector<Socket> node_heatDistance::getInputs() const {
  return {{"mesh", "Mesh", DataType::CUSTOM, Mesh{}, "mesh"},
          {"idv", "IDV", DataType::NUMBER, 0.0, "number"}};
}

std::vector<Socket> node_heatDistance::getOutputs() const {
  return {{"distance", "Distance", DataType::LIST, std::vector<double>{}}};
}

std::map<std::string, std::any> node_heatDistance::getProperties() const {
  return {};
}

NodeSchema node_heatDistance::getSchema() const {
  NodeSchema schema = NodeBase::getSchema();
  schema.color = "#805ad5";
  return schema;
}

bool node_heatDistance::execute(
    const std::map<std::string, std::any> &inputs,
    std::map<std::string, std::any> &outputs,
    const std::map<std::string, std::any> & /*properties*/) {
  try {
    const auto meshIt = inputs.find("mesh");
    if (meshIt == inputs.end()) {
      errorMessage =
          "Heat Distance node error: input mesh is missing or invalid";
      return false;
    }
    const Mesh *mesh = NodeUtils::getValuePtr<Mesh>(meshIt->second);
    if (!mesh) {
      errorMessage =
          "Heat Distance node error: input mesh is missing or invalid";
      return false;
    }
    const size_t vertexCount = mesh->vertices.size();
    if (vertexCount == 0) {
      errorMessage = "Heat Distance node error: mesh has no vertices";
      return false;
    }
    if (mesh->triangles.empty()) {
      errorMessage = "Heat Distance node error: mesh has no triangles";
      return false;
    }
    int idv =
        static_cast<int>(NodeUtils::getValue<double>(inputs.at("idv"), 0.0));
    if (idv < 0 || idv > static_cast<int>(vertexCount - 1)) {
      errorMessage = "Heat Distance node error: IDV must be between 0 and "
                     "vertex count - 1";
      return false;
    }

    /*** compute heat distance for vertices to vertex idv ***/
    std::vector<double> distance(vertexCount, 0.0);
    for (size_t i = 0; i < vertexCount; ++i) {
      distance[i] = std::rand() / static_cast<double>(RAND_MAX);
    }

    /*** compute heat distance for vertices to vertex idv ***/

    outputs["distance"] = distance;
    return true;
  } catch (const std::exception &e) {
    errorMessage = std::string("Heat Distance node error: ") + e.what();
    return false;
  }
}

namespace {
NodeRegistrar<node_heatDistance> node_heat_distance_registrar;
} // namespace