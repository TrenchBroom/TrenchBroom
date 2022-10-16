/*
 Copyright (C) 2022 Amara M. Kilic
 Copyright (C) 2022 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AssimpParser.h"

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "Logger.h"
#include "Model/BrushFaceAttributes.h"
#include "ReaderException.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <optional>
#include <utility>
#include <vector>

namespace TrenchBroom {
namespace IO {

namespace {

class AssimpIOStream : public Assimp::IOStream {
  friend class AssimpIOSystem;

private:
  const FileSystem& m_fs;
  std::shared_ptr<File> m_file;
  Reader m_reader;

protected:
  AssimpIOStream(const Path& path, const FileSystem& fs)
    : m_fs{fs}
    , m_file{m_fs.openFile(path)}
    , m_reader{m_file->reader()} {}

public:
  size_t Read(void* buffer, const size_t size, const size_t count) override {
    if (m_reader.canRead(size * count)) {
      m_reader.read(reinterpret_cast<char*>(buffer), size * count);
      return count;
    } else {
      return 0;
    }
  }

  size_t Write(
    const void* /* buffer */, const size_t /* size */, const size_t /* count */) override {
    return 0; // unsupported
  }

  aiReturn Seek(const size_t offset, const aiOrigin origin) override {
    try {
      switch (origin) {
        case aiOrigin_SET:
          m_reader.seekFromBegin(offset);
          return aiReturn_SUCCESS;
        case aiOrigin_CUR:
          m_reader.seekForward(offset);
          return aiReturn_SUCCESS;
        case aiOrigin_END:
          m_reader.seekFromEnd(offset);
          return aiReturn_SUCCESS;
        case _AI_ORIGIN_ENFORCE_ENUM_SIZE:
          break;
      }
    } catch (const ReaderException& /*e*/) { return aiReturn_FAILURE; }
    return aiReturn_FAILURE;
  }

  size_t Tell() const override { return m_reader.position(); }

  size_t FileSize() const override { return m_reader.size(); }

  // No writing.
  void Flush() override {}
};

class AssimpIOSystem : public Assimp::IOSystem {
private:
  const FileSystem& m_fs;

public:
  explicit AssimpIOSystem(const FileSystem& fs)
    : m_fs{fs} {}

  bool Exists(const char* path) const override { return m_fs.fileExists(Path{path}); }

  char getOsSeparator() const override { return Path::separator()[0]; }

  void Close(Assimp::IOStream* file) override { delete file; }

  Assimp::IOStream* Open(const char* path, const char* mode) override {
    if (mode[0] != 'r') {
      throw ParserException{"Assimp attempted to open a file not for reading."};
    }
    return new AssimpIOStream{Path{path}, m_fs};
  }
};

} // namespace

std::unique_ptr<Assets::EntityModel> AssimpParser::doInitializeModel(TrenchBroom::Logger& logger) {
  // Create model.
  auto model = std::make_unique<Assets::EntityModel>(
    m_path.asString(), Assets::PitchType::Normal, Assets::Orientation::Oriented);
  model->addFrame();
  auto& surface = model->addSurface(m_path.asString());

  m_positions.clear();
  m_vertices.clear();
  m_faces.clear();
  m_textures.clear();

  // Import the file as an Assimp scene and populate our vectors.
  auto importer = Assimp::Importer{};
  importer.SetIOHandler(new AssimpIOSystem{m_fs});

  const auto* scene = importer.ReadFile(
    m_path.asString(), aiProcess_Triangulate | aiProcess_FlipWindingOrder |
                         aiProcess_MakeLeftHanded | aiProcess_JoinIdenticalVertices |
                         aiProcess_SortByPType | aiProcess_FlipUVs);

  if (!scene) {
    throw ParserException{
      std::string{"Assimp couldn't import the file: "} + importer.GetErrorString()};
  }

  // Load materials as textures.
  processMaterials(*scene, logger);

  surface.setSkins(std::move(m_textures));

  // Assimp files import as y-up. Use this transform to change the model to z-up.
  auto axisTransform = aiMatrix4x4{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

  processNode(*scene->mRootNode, *scene, scene->mRootNode->mTransformation, axisTransform);

  // Build bounds.
  auto bounds = vm::bbox3f::builder{};
  if (m_positions.empty()) {
    // Passing empty bounds as bbox crashes the program, don't let it happen.
    throw ParserException{"Model has no vertices. (So no valid bounding box.)"};
  } else {
    bounds.add(std::begin(m_positions), std::end(m_positions));
  }

  // Begin model construction.
  // Part 1: Collation
  size_t totalVertexCount = 0;
  auto size = Renderer::TexturedIndexRangeMap::Size{};
  for (const auto& face : m_faces) {
    size.inc(surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
    totalVertexCount += face.m_vertices.size();
  }

  // Part 2: Building
  auto& frame = model->loadFrame(0, m_path.asString(), bounds.bounds());
  auto builder =
    Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type>{totalVertexCount, size};

  for (const auto& face : m_faces) {
    auto entityVertices = kdl::vec_transform(face.m_vertices, [&](const auto& index) {
      return Assets::EntityModelVertex{
        m_positions[m_vertices[index].m_position], m_vertices[index].m_texcoords};
    });
    builder.addPolygon(surface.skin(face.m_material), entityVertices);
  }

  surface.addTexturedMesh(frame, builder.vertices(), builder.indices());
  return model;
}

AssimpParser::AssimpParser(Path path, const FileSystem& fs)
  : m_path{std::move(path)}
  , m_fs{fs} {}

bool AssimpParser::canParse(const Path& path) {
  static const auto supportedExtensions = std::vector<std::string>{
    // Quake model formats have been omitted since Trenchbroom's got its own parsers already.
    "3mf",  "dae",      "xml",          "blend",    "bvh",       "3ds",  "ase",
    "lwo",  "lws",      "md5mesh",      "md5anim",  "md5camera", // Lightwave and Doom 3 formats.
    "gltf", "fbx",      "glb",          "ply",      "dxf",       "ifc",  "iqm",
    "nff",  "smd",      "vta", // .smd and .vta are uncompiled Source engine models.
    "mdc",  "x",        "q30",          "qrs",      "ter",       "raw",  "ac",
    "ac3d", "stl",      "dxf",          "irrmesh",  "irr",       "off",
    "obj", // .obj files will only be parsed by Assimp if the neverball importer isn't enabled.
    "mdl", // 3D GameStudio Model. It requires a palette file to load.
    "hmp",  "mesh.xml", "skeleton.xml", "material", "ogex",      "ms3d", "lxo",
    "csm",  "ply",      "cob",          "scn",      "xgl"};

  return kdl::vec_contains(supportedExtensions, kdl::str_to_lower(path.extension()));
}

void AssimpParser::processNode(
  const aiNode& node, const aiScene& scene, const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform) {
  for (unsigned int i = 0; i < node.mNumMeshes; i++) {
    const auto* mesh = scene.mMeshes[node.mMeshes[i]];
    processMesh(*mesh, transform, axisTransform);
  }
  for (unsigned int i = 0; i < node.mNumChildren; i++) {
    processNode(
      *node.mChildren[i], scene, transform * node.mChildren[i]->mTransformation, axisTransform);
  }
}

void AssimpParser::processMesh(
  const aiMesh& mesh, const aiMatrix4x4& transform, const aiMatrix4x4& axisTransform) {
  // Meshes have been sorted by primitive type, so we know for sure we'll ONLY get triangles in a
  // single mesh.
  if (mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
    const auto offset = m_vertices.size();
    // Add all the vertices of the mesh.
    for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
      const auto texcoords = mesh.mTextureCoords[0]
                               ? vm::vec2f{mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y}
                               : vm::vec2f{0.0f, 0.0f};

      m_vertices.emplace_back(m_positions.size(), texcoords);

      auto meshVertices = mesh.mVertices[i];
      meshVertices *= transform;
      meshVertices *= axisTransform;
      m_positions.emplace_back(meshVertices.x, meshVertices.y, meshVertices.z);
    }

    for (unsigned int i = 0; i < mesh.mNumFaces; i++) {
      auto verts = std::vector<size_t>{};
      verts.reserve(mesh.mFaces[i].mNumIndices);

      for (unsigned int j = 0; j < mesh.mFaces[i].mNumIndices; j++) {
        verts.push_back(mesh.mFaces[i].mIndices[j] + offset);
      }
      m_faces.emplace_back(mesh.mMaterialIndex, verts);
    }
  }
}

namespace {

Assets::Texture loadTextureFromFileSystem(const Path& path, const FileSystem& fs, Logger& logger) {
  auto imageReader = FreeImageTextureReader{TextureReader::StaticNameStrategy{""}, fs, logger};
  return imageReader.readTexture(fs.openFile(path));
}

Assets::Texture loadUncompressedEmbeddedTexture(
  const aiTexel* data, const std::string& name, const size_t width, const size_t height) {
  auto buffer = Assets::TextureBuffer{width * height * sizeof(aiTexel)};
  std::memcpy(buffer.data(), data, width * height * sizeof(aiTexel));

  const auto averageColor = FreeImageTextureReader::getAverageColor(buffer, GL_BGRA);
  return {
    name, width, height, averageColor, std::move(buffer), GL_BGRA, Assets::TextureType::Masked};
}

Assets::Texture loadCompressedEmbeddedTexture(
  const aiTexel* data, const std::string& name, const size_t width) {
  return FreeImageTextureReader::readTextureFromMemory(
    name, reinterpret_cast<const uint8_t*>(data), width);
}

std::optional<Assets::Texture> loadFallbackTexture(const FileSystem& fs, Logger& logger) {
  static const auto texturePaths = std::vector<Path>{
    Path{"textures"} + Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("png"),
    Path{"textures"} + Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("jpg"),
    Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("png"),
    Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("jpg"),
  };

  auto imageReader = FreeImageTextureReader{TextureReader::StaticNameStrategy(""), fs, logger};

  for (const auto& texturePath : texturePaths) {
    try {
      return imageReader.readTexture(fs.openFile(texturePath));
    } catch (const Exception& /*ex1*/) {
      // ignore and try the next texture path
    }
  }

  return std::nullopt;
}

} // namespace

void AssimpParser::processMaterials(const aiScene& scene, Logger& logger) {
  for (unsigned int i = 0; i < scene.mNumMaterials; i++) {
    try {
      // Is there even a single diffuse texture? If not, fail and load fallback material.
      if (scene.mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        throw Exception{"Material does not contain a texture."};
      }

      auto path = aiString{};
      scene.mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);

      const auto texturePath = Path{path.C_Str()};
      const auto* texture = scene.GetEmbeddedTexture(path.C_Str());
      if (!texture) {
        // The texture is not embedded. Load it using the file system.
        const auto filePath = m_path.deleteLastComponent() + texturePath;
        m_textures.push_back(loadTextureFromFileSystem(filePath, m_fs, logger));
      } else if (texture->mHeight != 0) {
        // The texture is uncompressed, load it directly.
        m_textures.push_back(loadUncompressedEmbeddedTexture(
          texture->pcData, texture->mFilename.C_Str(), texture->mWidth, texture->mHeight));
      } else {
        // The texture is embedded, but compressed. Let FreeImage load it from memory.
        m_textures.push_back(loadCompressedEmbeddedTexture(
          texture->pcData, texture->mFilename.C_Str(), texture->mWidth));
      }
    } catch (Exception& exception) {
      // Load fallback material in case we get any error.
      if (auto fallbackTexture = loadFallbackTexture(m_fs, logger)) {
        m_textures.push_back(std::move(*fallbackTexture));
      }

      // Materials aren't guaranteed to have a name.
      const auto materialName = scene.mMaterials[i]->GetName() != aiString{""}
                                  ? scene.mMaterials[i]->GetName().C_Str()
                                  : "nr. " + std::to_string(i + 1);
      logger.error(
        "Model " + m_path.asString() + ": Loading fallback material for material " + materialName +
        ": " + exception.what());
    }
  }
}

} // namespace IO
} // namespace TrenchBroom
