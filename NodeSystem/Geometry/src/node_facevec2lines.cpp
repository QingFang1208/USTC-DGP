#include "node_facevec2lines.h"
#include "node_lines.h"
#include "node_mesh.h"
#include <array>

std::string node_facevec2lines::getType() const { return "facevec2lines"; }

std::string node_facevec2lines::getName() const {
  return "Face Vector to Lines";
}

std::string node_facevec2lines::getCategory() const { return "Method"; }

std::string node_facevec2lines::getDescription() const {
  return "Convert face vectors to lines for a mesh.";
}

std::vector<Socket> node_facevec2lines::getInputs() const {
  return {
      {"mesh", "Mesh", DataType::CUSTOM, Mesh{}, "mesh"},
      {"facevecs", "Facevecs", DataType::MATRIX,
       std::vector<std::vector<double>>(), "matrix"},
  };
}

std::vector<Socket> node_facevec2lines::getOutputs() const {
  return {{"lines", "Lines", DataType::CUSTOM, LineSet{}, "lines"}};
}

std::map<std::string, std::any> node_facevec2lines::getProperties() const {
  return {};
}

std::map<std::string, std::vector<std::string>>
node_facevec2lines::getPropertyOptions() const {
  return {};
}

NodeSchema node_facevec2lines::getSchema() const {
  NodeSchema schema = NodeBase::getSchema();
  schema.color = "#805ad5";
  return schema;
}

bool node_facevec2lines::execute(
    const std::map<std::string, std::any> &inputs,
    std::map<std::string, std::any> &outputs,
    const std::map<std::string, std::any> & /*properties*/) {
  try {
    auto meshIt = inputs.find("mesh");
    if (meshIt == inputs.end()) {
      errorMessage = "Facevec2lines node error: input mesh is missing";
      return false;
    }
    const Mesh *mesh = NodeUtils::getValuePtr<Mesh>(meshIt->second);
    if (!mesh) {
      errorMessage = "Facevec2lines node error: input mesh is missing";
      return false;
    }

    auto facevecsIt = inputs.find("facevecs");
    if (facevecsIt == inputs.end()) {
      errorMessage = "Facevec2lines node error: input facevecs is missing";
      return false;
    }
    const auto facevecs =
        NodeUtils::getValuePtr<std::vector<std::vector<double>>>(
            facevecsIt->second);
    if (!facevecs) {
      errorMessage = "Facevec2lines node error: input facevecs is missing";
      return false;
    }

    if (facevecs->size() != mesh->triangles.size() ||
        facevecs->at(0).size() != 3) {
      errorMessage =
          "Facevec2lines node error: input facevecs has incorrect dimensions";
      return false;
    }

    auto result = std::make_shared<LineSet>();
    result->directed = false;

    std::vector<std::array<double, 3>> mesh_facects;
    std::vector<double> mesh_faceradius;
    mesh_facects.resize(mesh->triangles.size());
    mesh_faceradius.resize(mesh->triangles.size());

    for (size_t i = 0; i < mesh->triangles.size(); ++i) {
      const auto &viArr =
          mesh->vertices[static_cast<std::size_t>(mesh->triangles[i][0])];
      const auto &vjArr =
          mesh->vertices[static_cast<std::size_t>(mesh->triangles[i][1])];
      const auto &vkArr =
          mesh->vertices[static_cast<std::size_t>(mesh->triangles[i][2])];
      const auto vi = std::array<double, 3>{viArr[0], viArr[1], viArr[2]};
      const auto vj = std::array<double, 3>{vjArr[0], vjArr[1], vjArr[2]};
      const auto vk = std::array<double, 3>{vkArr[0], vkArr[1], vkArr[2]};
      const auto ct = std::array<double, 3>{(vi[0] + vj[0] + vk[0]) / 3.0,
                                            (vi[1] + vj[1] + vk[1]) / 3.0,
                                            (vi[2] + vj[2] + vk[2]) / 3.0};
      mesh_faceradius[i] = (std::sqrt((vi[0] - ct[0]) * (vi[0] - ct[0]) +
                                      (vi[1] - ct[1]) * (vi[1] - ct[1]) +
                                      (vi[2] - ct[2]) * (vi[2] - ct[2])) +
                            std::sqrt((vj[0] - ct[0]) * (vj[0] - ct[0]) +
                                      (vj[1] - ct[1]) * (vj[1] - ct[1]) +
                                      (vj[2] - ct[2]) * (vj[2] - ct[2])) +
                            std::sqrt((vk[0] - ct[0]) * (vk[0] - ct[0]) +
                                      (vk[1] - ct[1]) * (vk[1] - ct[1]) +
                                      (vk[2] - ct[2]) * (vk[2] - ct[2]))) /
                           3.0;

      const auto edge1 =
          std::array<double, 3>{vj[0] - vi[0], vj[1] - vi[1], vj[2] - vi[2]};
      const auto edge2 =
          std::array<double, 3>{vk[0] - vi[0], vk[1] - vi[1], vk[2] - vi[2]};
      auto normal =
          std::array<double, 3>{edge1[1] * edge2[2] - edge1[2] * edge2[1],
                                edge1[2] * edge2[0] - edge1[0] * edge2[2],
                                edge1[0] * edge2[1] - edge1[1] * edge2[0]};
      const double norm =
          std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] +
                    normal[2] * normal[2]);
      if (norm > std::numeric_limits<double>::epsilon()) {
        normal[0] /= norm;
        normal[1] /= norm;
        normal[2] /= norm;
      }
      mesh_facects[i][0] = ct[0] + normal[0] * (1e-3 * mesh_faceradius[i]);
      mesh_facects[i][1] = ct[1] + normal[1] * (1e-3 * mesh_faceradius[i]);
      mesh_facects[i][2] = ct[2] + normal[2] * (1e-3 * mesh_faceradius[i]);
    }

    result->points.resize(2 * mesh_facects.size());
    result->segments.resize(mesh_facects.size());
    double mean_len = 0.0;
    double mean_radius = 0.0;
    for (size_t i = 0; i < mesh_facects.size(); ++i) {
      result->segments[i][0] = 2 * i;
      result->segments[i][1] = 2 * i + 1;
      mean_len += std::sqrt(facevecs->at(i)[0] * facevecs->at(i)[0] +
                            facevecs->at(i)[1] * facevecs->at(i)[1] +
                            facevecs->at(i)[2] * facevecs->at(i)[2]);
      mean_radius += mesh_faceradius[i];
    }
    if (!mesh_facects.empty()) {
      mean_len /= mesh_facects.size();
      mean_radius /= mesh_facects.size();
    }
    double mean_scale = 0.5 * mean_radius / mean_len;

    for (size_t i = 0; i < mesh_facects.size(); ++i) {
      result->points[2 * i][0] = mesh_facects[i][0];
      result->points[2 * i][1] = mesh_facects[i][1];
      result->points[2 * i][2] = mesh_facects[i][2];

      const auto &v = facevecs->at(i);
      result->points[2 * i + 1][0] = v[0] * mean_scale + mesh_facects[i][0];
      result->points[2 * i + 1][1] = v[1] * mean_scale + mesh_facects[i][1];
      result->points[2 * i + 1][2] = v[2] * mean_scale + mesh_facects[i][2];
    }

    outputs["lines"] = result;
    return true;
  } catch (const std::exception &e) {
    errorMessage = std::string("Facevec2lines node error: ") + e.what();
    return false;
  }
}

namespace {
NodeRegistrar<node_facevec2lines> node_facevec2lines_registrar;
}