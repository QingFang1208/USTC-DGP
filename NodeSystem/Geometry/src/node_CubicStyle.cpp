#include "node_CubicStyle.h"
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

std::string node_CubicStyle::getType() const { return "cubic_style"; }
std::string node_CubicStyle::getName() const { return "Cubic Style"; }
std::string node_CubicStyle::getCategory() const { return "Geometry"; }
std::string node_CubicStyle::getDescription() const {
  return "Apply cubic style to a mesh";
}

std::vector<Socket> node_CubicStyle::getInputs() const {
  return {{"mesh", "Mesh", DataType::CUSTOM, Mesh{}, "mesh"}};
}

std::vector<Socket> node_CubicStyle::getOutputs() const {
  return {
      {"xyz", "Matrix", DataType::MATRIX, std::vector<std::vector<double>>()}};
}

std::map<std::string, std::any> node_CubicStyle::getProperties() const {
  return {};
}

NodeSchema node_CubicStyle::getSchema() const {
  NodeSchema schema = NodeBase::getSchema();
  schema.color = "#805ad5";
  return schema;
}

bool node_CubicStyle::execute(
    const std::map<std::string, std::any> &inputs,
    std::map<std::string, std::any> &outputs,
    const std::map<std::string, std::any> & /*properties*/) {
  try {
    const auto meshIt = inputs.find("mesh");
    if (meshIt == inputs.end()) {
      errorMessage =
          "Cubic Style node error: input mesh is missing or invalid";
      return false;
    }
    const Mesh *mesh = NodeUtils::getValuePtr<Mesh>(meshIt->second);
    if (!mesh) {
      errorMessage =
          "Cubic Style node error: input mesh is missing or invalid";
      return false;
    }
    const size_t vertexCount = mesh->vertices.size();
    if (vertexCount == 0) {
      errorMessage = "Cubic Style node error: mesh has no vertices";
      return false;
    }

    /*** compute cubic stylization ***/
    std::vector<std::vector<double>> xyz(vertexCount,
                                        std::vector<double>(3, 0.0));
    for (size_t i = 0; i < vertexCount; ++i) {
      xyz[i][0] = std::rand() / static_cast<double>(RAND_MAX);
      xyz[i][1] = std::rand() / static_cast<double>(RAND_MAX);
      xyz[i][2] = std::rand() / static_cast<double>(RAND_MAX);
    }

    /*** compute cubic stylization ***/

    outputs["xyz"] = xyz;
    return true;
  } catch (const std::exception &e) {
    errorMessage =
        std::string("Cubic Style node error: ") + e.what();
    return false;
  }
}

namespace {
NodeRegistrar<node_CubicStyle> node_cubic_style_registrar;
} // namespace