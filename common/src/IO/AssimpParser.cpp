/*
 Copyright (C) 2022 Amara M. Kilic

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

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <utility>
#include <vector>

namespace TrenchBroom {
namespace IO {
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
    , m_reader{m_file->reader()} {
    m_file = m_fs.openFile(path);
    m_reader = m_file->reader();
    if (m_file == nullptr) {
      throw(ParserException("Could not load file '" + path.asString() + "'."));
    }
  }

public:
  ~AssimpIOStream() override = default;
  size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override {
    if (m_reader.canRead(pSize * pCount)) {
      m_reader.read(reinterpret_cast<char*>(pvBuffer), pSize * pCount);
      return pCount;
    } else {
      return 0;
    }
  }

  size_t Write(const void* /* pvBuffer */, size_t /* pSize */, size_t /* pCount */) override {
    return 0; // As far as I can see you can't write with File, and with a parser we never should,
              // anyway.
  }

  aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override {
    try {
      switch (pOrigin) {
        case aiOrigin_SET:
          m_reader.seekFromBegin(pOffset);
          return aiReturn_SUCCESS;
        case aiOrigin_CUR:
          m_reader.seekForward(pOffset);
          return aiReturn_SUCCESS;
        case aiOrigin_END:
          m_reader.seekFromEnd(pOffset);
          return aiReturn_SUCCESS;
        case _AI_ORIGIN_ENFORCE_ENUM_SIZE:
          break;
      }
    } catch (ReaderException& /*e*/) { return aiReturn_FAILURE; }
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

  bool Exists(const char* pFile) const override { return m_fs.fileExists(Path{pFile}); }

  char getOsSeparator() const override { return Path::separator()[0]; }

  void Close(Assimp::IOStream* file) override { delete file; }

  Assimp::IOStream* Open(const char* pFile, const char* pMode) override {
    if (pMode[0] != 'r') {
      throw(ParserException{"Assimp attempted to open a file not for reading."});
    }
    return new AssimpIOStream{Path(pFile), m_fs};
  }
};

// Copied from FreeImageTextureReader.cpp
static Color getAverageColor(const Assets::TextureBuffer& buffer, const GLenum format) {
  ensure(format == GL_RGBA || format == GL_BGRA, "expected RGBA or BGRA");

  const unsigned char* const data = buffer.data();
  const std::size_t bufferSize = buffer.size();

  Color average;
  for (std::size_t i = 0; i < bufferSize; i += 4) {
    average = average + Color{data[i], data[i + 1], data[i + 2], data[i + 3]};
  }
  const std::size_t numPixels = bufferSize / 4;
  average = average / static_cast<float>(numPixels);

  return average;
}

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
  Assimp::Importer importer;
  importer.SetIOHandler(new AssimpIOSystem(m_fs));
  const aiScene* scene = importer.ReadFile(
    (m_path).asString(), aiProcess_Triangulate | aiProcess_FlipWindingOrder |
                           aiProcess_MakeLeftHanded | aiProcess_JoinIdenticalVertices |
                           aiProcess_SortByPType | aiProcess_FlipUVs);

  if (!scene) {
    throw ParserException{
      std::string{"Assimp couldn't import the file: "} + importer.GetErrorString()};
  }

  // Load materials as textures.
  processMaterials(scene, logger);

  surface.setSkins(std::move(m_textures));

  // Assimp files import as y-up. Use this transform to change the model to z-up.
  aiMatrix4x4 axisTransform{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

  processNode(scene->mRootNode, scene, scene->mRootNode->mTransformation, axisTransform);

  // Build bounds.
  auto bounds = vm::bbox3f::builder{};
  if (m_positions.empty()) {
    // Passing empty bounds as bbox crashes the program, don't let it happen.
    throw ParserException{("Model has no vertices. (So no valid bounding box.)")};
  } else {
    bounds.add(std::begin(m_positions), std::end(m_positions));
  }

  // Begin model construction.
  // Part 1: Collation
  size_t totalVertexCount = 0;
  Renderer::TexturedIndexRangeMap::Size size;
  for (const AssimpFace& face : m_faces) {
    size.inc(surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
    totalVertexCount += face.m_vertices.size();
  }

  // Part 2: Building
  auto& frame = model->loadFrame(0, m_path.asString(), bounds.bounds());
  Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type> builder{
    totalVertexCount, size};

  for (const AssimpFace& face : m_faces) {
    std::vector<Assets::EntityModelVertex> entityVertices;
    for (size_t index : face.m_vertices) {
      entityVertices.emplace_back(
        m_positions[m_vertices[index].m_position], m_vertices[index].m_texcoords);
    }
    builder.addPolygon(surface.skin(face.m_material), entityVertices);
  }
  surface.addTexturedMesh(frame, builder.vertices(), builder.indices());
  return model;
}

AssimpParser::AssimpParser(Path path, const FileSystem& fs)
  : m_path(std::move(path))
  , m_fs(fs) {}

void AssimpParser::processNode(
  aiNode* node, const aiScene* scene, aiMatrix4x4 transform, aiMatrix4x4& axisTransform) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(mesh, transform, axisTransform);
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(
      node->mChildren[i], scene, transform * node->mChildren[i]->mTransformation, axisTransform);
  }
}

void AssimpParser::processMesh(aiMesh* mesh, aiMatrix4x4& transform, aiMatrix4x4& axisTransform) {
  // Meshes have been sorted by primitive type, so we know for sure we'll ONLY get triangles in a
  // single mesh.
  if (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
    size_t offset = m_vertices.size();
    // Add all the vertices of the mesh.
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      vm::vec2f texcoords{0.0f, 0.0f};
      if (mesh->mTextureCoords[0]) {
        texcoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
      }
      m_vertices.emplace_back(m_positions.size(), texcoords);
      aiVector3D meshVertices = mesh->mVertices[i];
      meshVertices *= transform;
      meshVertices *= axisTransform;
      m_positions.emplace_back(meshVertices.x, meshVertices.y, meshVertices.z);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      std::vector<size_t> verts;
      for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
        verts.push_back(mesh->mFaces[i].mIndices[j] + offset);
      }
      m_faces.emplace_back(mesh->mMaterialIndex, verts);
    }
  }
}

void AssimpParser::processMaterials(const aiScene* scene, Logger& logger) {
  for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    try {
      // Is there even a single diffuse texture? If not, fail and load fallback material.
      if (scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        throw Exception{(std::string("Material does not contain a texture."))};
      }
      aiString path;
      scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
      std::string texturePath = std::string(path.C_Str());
      const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
      if (!texture) {
        // The texture is not embedded. Load it using the file system.
        IO::FreeImageTextureReader imageReader{
          IO::TextureReader::StaticNameStrategy(""), m_fs, logger};
        const Path filePath = m_path.deleteLastComponent() + Path(texturePath);
        auto file = m_fs.openFile(filePath);
        Assets::Texture textureAsset = imageReader.readTexture(file);
        m_textures.push_back(std::move(textureAsset));
        continue;
      }
      if (texture->mHeight != 0) {
        // The texture is uncompressed, load it directly.
        Assets::TextureBuffer buffer{(texture->mWidth * texture->mHeight * sizeof(aiTexel))};
        std::memcpy(
          buffer.data(), texture->pcData, texture->mWidth * texture->mHeight * sizeof(aiTexel));
        Color averageColor = getAverageColor(buffer, GL_BGRA);
        Assets::Texture textureAsset = Assets::Texture{
          texturePath,       texture->mWidth, texture->mHeight,           averageColor,
          std::move(buffer), GL_BGRA,         Assets::TextureType::Masked};
        m_textures.push_back(std::move(textureAsset));
        continue;
      }
      // The texture is embedded, but compressed. Let FreeImage load it from memory.
      IO::FreeImageTextureReader imageReader(
        IO::TextureReader::StaticNameStrategy(""), m_fs, logger);
      Assets::Texture textureAsset = imageReader.readTextureFromMemory(
        texture->mFilename.C_Str(), reinterpret_cast<uint8_t*>(texture->pcData), texture->mWidth);
      m_textures.push_back(std::move(textureAsset));
      continue;
    } catch (Exception& exception) {
      // Load fallback material in case we get any error.
      std::vector<Path> texturePaths = {
        Path{"textures"} + Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("png"),
        Path{"textures"} + Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("jpg"),
        Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("png"),
        Path{Model::BrushFaceAttributes::NoTextureName}.addExtension("jpg"),
      };

      IO::FreeImageTextureReader imageReader{
        IO::TextureReader::StaticNameStrategy(""), m_fs, logger};
      for (const auto& texturePath : texturePaths) {
        try {
          auto file = m_fs.openFile(texturePath);
          m_textures.push_back(imageReader.readTexture(file));
        } catch (const Exception& /*ex1*/) {
          // ignore and try the next texture path
        }
      }

      // Materials aren't guaranteed to have a name.
      std::string materialName = scene->mMaterials[i]->GetName() != aiString("")
                                   ? scene->mMaterials[i]->GetName().C_Str()
                                   : "nr. " + std::to_string(i + 1);
      logger.error(
        std::string{"Model "} + m_path.asString() + ": Loading fallback material for material " +
        materialName + ": " + exception.what());
    }
  }
}

std::vector<std::string> AssimpParser::get_supported_extensions() {
  return std::vector<std::string>{
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
}
} // namespace IO
} // namespace TrenchBroom