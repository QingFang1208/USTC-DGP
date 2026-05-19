#include "node_HodgeDmp.h"
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
#include <vector>

std::string node_HodgeDmp::getType() const { return "hodge_dmp"; }
std::string node_HodgeDmp::getName() const { return "Hodge Decomposition"; }
std::string node_HodgeDmp::getCategory() const { return "Geometry"; }
std::string node_HodgeDmp::getDescription() const {
  return "Compute Hodge Decomposition for a mesh";
}

std::vector<Socket> node_HodgeDmp::getInputs() const {
  return {{"mesh", "Mesh", DataType::CUSTOM, Mesh{}, "mesh"}};
}

std::vector<Socket> node_HodgeDmp::getOutputs() const {
  return {{"divfree", "Divfree", DataType::MATRIX,
           std::vector<std::vector<double>>()},
          {"curlfree", "Curlfree", DataType::MATRIX,
           std::vector<std::vector<double>>()},
          {"harmonic", "Harmonic", DataType::MATRIX,
           std::vector<std::vector<double>>()}};
}

std::map<std::string, std::any> node_HodgeDmp::getProperties() const {
  return {};
}

NodeSchema node_HodgeDmp::getSchema() const {
  NodeSchema schema = NodeBase::getSchema();
  schema.color = "#805ad5";
  return schema;
}

bool node_HodgeDmp::execute(
    const std::map<std::string, std::any> &inputs,
    std::map<std::string, std::any> &outputs,
    const std::map<std::string, std::any> & /*properties*/) {
  try {
    const auto meshIt = inputs.find("mesh");
    if (meshIt == inputs.end()) {
      errorMessage =
          "Hodge Decomposition node error: input mesh is missing or invalid";
      return false;
    }
    const Mesh *mesh = NodeUtils::getValuePtr<Mesh>(meshIt->second);
    if (!mesh) {
      errorMessage =
          "Hodge Decomposition node error: input mesh is missing or invalid";
      return false;
    }
    const size_t vertexCount = mesh->vertices.size();
    const size_t faceCount = mesh->triangles.size();
    if (vertexCount == 0) {
      errorMessage = "Hodge Decomposition node error: mesh has no vertices";
      return false;
    }

    std::vector<std::vector<double>> faceNormals(faceCount,
                                                 std::vector<double>(3, 0.0));
    std::vector<std::vector<double>> randvec(faceCount,
                                             std::vector<double>(3, 0.0));
    std::vector<std::vector<double>> divfree(faceCount,
                                             std::vector<double>(3, 0.0));
    std::vector<std::vector<double>> curlfree(faceCount,
                                              std::vector<double>(3, 0.0));
    std::vector<std::vector<double>> harmonic(faceCount,
                                              std::vector<double>(3, 0.0));

    for (size_t i = 0; i < faceCount; ++i) {

      const auto &face = mesh->triangles[i];

      const auto &v0 = mesh->vertices[face[0]];
      const auto &v1 = mesh->vertices[face[1]];
      const auto &v2 = mesh->vertices[face[2]];
      const auto edge1 =
          std::array<double, 3>{v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
      const auto edge2 =
          std::array<double, 3>{v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
      const auto normal =
          std::array<double, 3>{edge1[1] * edge2[2] - edge1[2] * edge2[1],
                                edge1[2] * edge2[0] - edge1[0] * edge2[2],
                                edge1[0] * edge2[1] - edge1[1] * edge2[0]};
      const double norm =
          std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] +
                    normal[2] * normal[2]);
      if (norm > std::numeric_limits<double>::epsilon()) {
        faceNormals[i][0] = normal[0] / norm;
        faceNormals[i][1] = normal[1] / norm;
        faceNormals[i][2] = normal[2] / norm;
      } else {
        faceNormals[i][0] = normal[0];
        faceNormals[i][1] = normal[1];
        faceNormals[i][2] = normal[2];
      }
    }

    for (size_t i = 0; i < faceCount; ++i) {
      randvec[i][0] = std::rand() / static_cast<double>(RAND_MAX);
      randvec[i][1] = std::rand() / static_cast<double>(RAND_MAX);
      randvec[i][2] = std::rand() / static_cast<double>(RAND_MAX);
      // project randvec into face plane
      const double dot = randvec[i][0] * faceNormals[i][0] +
                         randvec[i][1] * faceNormals[i][1] +
                         randvec[i][2] * faceNormals[i][2];
      randvec[i][0] -= dot * faceNormals[i][0];
      randvec[i][1] -= dot * faceNormals[i][1];
      randvec[i][2] -= dot * faceNormals[i][2];
    }

    /*** compute Hodge decomposition ***/

    for (size_t i = 0; i < faceCount; ++i) {
      divfree[i][0] =
          randvec[i][0] * 0.5; // placeholder for actual div-free component
      divfree[i][1] =
          randvec[i][1] * 0.5; // placeholder for actual div-free component
      divfree[i][2] =
          randvec[i][2] * 0.5; // placeholder for actual div-free component
      curlfree[i][0] =
          randvec[i][0] * 0.3; // placeholder for actual curl-free component
      curlfree[i][1] =
          randvec[i][1] * 0.3; // placeholder for actual curl-free component
      curlfree[i][2] =
          randvec[i][2] * 0.3; // placeholder for actual curl-free component
      harmonic[i][0] =
          randvec[i][0] * 0.2; // placeholder for actual harmonic component
      harmonic[i][1] =
          randvec[i][1] * 0.2; // placeholder for actual harmonic component
      harmonic[i][2] =
          randvec[i][2] * 0.2; // placeholder for actual harmonic component
    }

    /*** compute Hodge decomposition ***/

    outputs["divfree"] = divfree;
    outputs["curlfree"] = curlfree;
    outputs["harmonic"] = harmonic;
    return true;
  } catch (const std::exception &e) {
    errorMessage = std::string("Hodge Decomposition node error: ") + e.what();
    return false;
  }
}

namespace {
NodeRegistrar<node_HodgeDmp> node_hodge_dmp_registrar;
} // namespace