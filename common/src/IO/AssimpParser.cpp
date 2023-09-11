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
#include "Error.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/PathInfo.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "Model/BrushFaceAttributes.h"
#include "ReaderException.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

#include "kdl/result_fold.h"
#include <kdl/path_utils.h>
#include <kdl/result.h>
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

namespace TrenchBroom
{
namespace IO
{

namespace
{

class AssimpIOStream : public Assimp::IOStream
{
  friend class AssimpIOSystem;

private:
  std::shared_ptr<File> m_file;
  Reader m_reader;

public:
  explicit AssimpIOStream(std::shared_ptr<File> file)
    : m_file{std::move(file)}
    , m_reader{m_file->reader()}
  {
  }

  size_t Read(void* buffer, const size_t size, const size_t count) override
  {
    if (m_reader.canRead(size * count))
    {
      m_reader.read(reinterpret_cast<char*>(buffer), size * count);
      return count;
    }
    else
    {
      return 0;
    }
  }

  size_t Write(
    const void* /* buffer */, const size_t /* size */, const size_t /* count */) override
  {
    return 0; // unsupported
  }

  aiReturn Seek(const size_t offset, const aiOrigin origin) override
  {
    try
    {
      switch (origin)
      {
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
    }
    catch (const ReaderException& /*e*/)
    {
      return aiReturn_FAILURE;
    }
    return aiReturn_FAILURE;
  }

  size_t Tell() const override { return m_reader.position(); }

  size_t FileSize() const override { return m_reader.size(); }

  // No writing.
  void Flush() override {}
};

class AssimpIOSystem : public Assimp::IOSystem
{
private:
  const FileSystem& m_fs;

public:
  explicit AssimpIOSystem(const FileSystem& fs)
    : m_fs{fs}
  {
  }

  bool Exists(const char* path) const override
  {
    return m_fs.pathInfo(std::filesystem::path{path}) == PathInfo::File;
  }

  char getOsSeparator() const override
  {
    return std::filesystem::path::preferred_separator;
  }

  void Close(Assimp::IOStream* file) override { delete file; }

  Assimp::IOStream* Open(const char* path, const char* mode) override
  {
    if (mode[0] != 'r')
    {
      throw ParserException{"Assimp attempted to open a file not for reading."};
    }

    return m_fs.openFile(path)
      .transform(
        [](auto file) { return std::make_unique<AssimpIOStream>(std::move(file)); })
      .if_error([](auto e) { throw ParserException{e.msg}; })
      .value()
      .release();
  }
};

} // namespace

std::unique_ptr<Assets::EntityModel> AssimpParser::doInitializeModel(
  TrenchBroom::Logger& logger)
{
  // Create model.
  auto model = std::make_unique<Assets::EntityModel>(
    m_path.string(), Assets::PitchType::Normal, Assets::Orientation::Oriented);
  model->addFrame();
  auto& surface = model->addSurface(m_path.string());

  // Import the file as an Assimp scene and populate our vectors.
  auto importer = Assimp::Importer{};
  importer.SetIOHandler(new AssimpIOSystem{m_fs});

  const auto* scene = importer.ReadFile(
    m_path.string(),
    aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipWindingOrder
      | aiProcess_SortByPType | aiProcess_FlipUVs);

  if (!scene)
  {
    throw ParserException{
      std::string{"Assimp couldn't import the file: "} + importer.GetErrorString()};
  }

  // Load materials as textures.
  auto textures = std::vector<Assets::Texture>{};
  for (unsigned int i = 0; i < scene->mNumMaterials; i++)
  {
    textures.push_back(processMaterial(*scene, i, logger));
  }
  surface.setSkins(std::move(textures));

  // load the reference pose as the only frame for this model
  loadSceneFrame(scene, surface, *model);

  return model;
}

void AssimpParser::loadSceneFrame(
  const aiScene* scene,
  Assets::EntityModelSurface& surface,
  Assets::EntityModel& model) const
{
  auto meshes = std::vector<AssimpMeshWithTransforms>{};

  // Assimp files import as y-up. We must multiply the root transform with an axis
  // transform matrix.
  processNode(
    meshes,
    *scene->mRootNode,
    *scene,
    scene->mRootNode->mTransformation,
    getAxisTransform(*scene));


  auto vertices = std::vector<Assets::EntityModelVertex>{};
  auto faces = std::vector<AssimpFace>{};

  for (auto mesh : meshes)
  {
    const auto offset = vertices.size();

    const auto meshVerts =
      computeMeshVertices(*mesh.m_mesh, mesh.m_transform, mesh.m_axisTransform);
    const auto meshFaces = computeMeshFaces(*mesh.m_mesh, offset);

    vertices.insert(vertices.end(), meshVerts.begin(), meshVerts.end());
    faces.insert(faces.end(), meshFaces.begin(), meshFaces.end());
  }

  // Build bounds.
  auto bounds = vm::bbox3f::builder{};
  if (vertices.empty())
  {
    // Passing empty bounds as bbox crashes the program, don't let it happen.
    throw ParserException{"Model has no vertices. (So no valid bounding box.)"};
  }
  else
  {
    bounds.add(
      std::begin(vertices), std::end(vertices), [](const Assets::EntityModelVertex& v) {
        return v.attr;
      });
  }

  // Begin model construction.
  // Part 1: Collation
  size_t totalVertexCount = 0;
  auto size = Renderer::TexturedIndexRangeMap::Size{};
  for (const auto& face : faces)
  {
    size.inc(
      surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
    totalVertexCount += face.m_vertices.size();
  }

  // Part 2: Building
  auto& frame = model.loadFrame(0, m_path.string(), bounds.bounds());
  auto builder = Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type>{
    totalVertexCount, size};

  for (const auto& face : faces)
  {
    auto entityVertices = kdl::vec_transform(
      face.m_vertices, [&](const auto& index) { return vertices[index]; });
    builder.addPolygon(surface.skin(face.m_material), entityVertices);
  }

  surface.addTexturedMesh(frame, builder.vertices(), builder.indices());
}

AssimpParser::AssimpParser(std::filesystem::path path, const FileSystem& fs)
  : m_path{std::move(path)}
  , m_fs{fs}
{
}

bool AssimpParser::canParse(const std::filesystem::path& path)
{
  // clang-format off
  static const auto supportedExtensions = std::vector<std::string>{
    // Quake model formats have been omitted since Trenchbroom's got its own parsers
    // already.
    ".3mf",  ".dae",      ".xml",          ".blend",    ".bvh",       ".3ds",  ".ase",
    ".lwo",  ".lws",      ".md5mesh",      ".md5anim",  ".md5camera", // Lightwave and Doom 3 formats
    ".gltf", ".fbx",      ".glb",          ".ply",      ".dxf",       ".ifc",  ".iqm",
    ".nff",  ".smd",      ".vta", // .smd and .vta are uncompiled Source engine models.
    ".mdc",  ".x",        ".q30",          ".qrs",      ".ter",       ".raw",  ".ac",
    ".ac3d", ".stl",      ".dxf",          ".irrmesh",  ".irr",       ".off",
    ".obj", // .obj files will only be parsed by Assimp if the neverball importer isn't enabled
    ".mdl", // 3D GameStudio Model. It requires a palette file to load.
    ".hmp",  ".mesh.xml", ".skeleton.xml", ".material", ".ogex",      ".ms3d", ".lxo",
    ".csm",  ".ply",      ".cob",          ".scn",      ".xgl"};
  // clang-format on

  return kdl::vec_contains(
    supportedExtensions, kdl::str_to_lower(path.extension().string()));
}

void AssimpParser::processNode(
  std::vector<AssimpMeshWithTransforms>& meshes,
  const aiNode& node,
  const aiScene& scene,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform)
{
  for (unsigned int i = 0; i < node.mNumMeshes; i++)
  {
    const auto* mesh = scene.mMeshes[node.mMeshes[i]];
    meshes.emplace_back(mesh, transform, axisTransform);
  }
  for (unsigned int i = 0; i < node.mNumChildren; i++)
  {
    processNode(
      meshes,
      *node.mChildren[i],
      scene,
      transform * node.mChildren[i]->mTransformation,
      axisTransform);
  }
}

std::vector<Assets::EntityModelVertex> AssimpParser::computeMeshVertices(
  const aiMesh& mesh, const aiMatrix4x4& transform, const aiMatrix4x4& axisTransform)
{
  std::vector<Assets::EntityModelVertex> vertices{};

  // We pass through the aiProcess_Triangulate flag to assimp, so we know for sure we'll
  // ONLY get triangles in a single mesh. This is just a safety net to make sure we don't
  // do anything bad.
  if (!(mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
  {
    return vertices;
  }

  const size_t numVerts = mesh.mNumVertices;
  vertices.reserve(numVerts);

  // Add all the vertices of the mesh.
  for (unsigned int i = 0; i < numVerts; i++)
  {
    const auto texcoords =
      mesh.mTextureCoords[0]
        ? vm::vec2f{mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y}
        : vm::vec2f{0.0f, 0.0f};

    auto meshVertices = mesh.mVertices[i];
    meshVertices *= transform;
    meshVertices *= axisTransform;

    const auto vec = vm::vec3f(meshVertices.x, meshVertices.y, meshVertices.z);
    vertices.emplace_back(vec, texcoords);
  }

  return vertices;
}

std::vector<AssimpFace> AssimpParser::computeMeshFaces(const aiMesh& mesh, size_t offset)
{
  std::vector<AssimpFace> faces{};

  // We pass through the aiProcess_Triangulate flag to assimp, so we know for sure we'll
  // ONLY get triangles in a single mesh. This is just a safety net to make sure we don't
  // do anything bad.
  if (!(mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
  {
    return faces;
  }

  for (unsigned int i = 0; i < mesh.mNumFaces; i++)
  {
    auto verts = std::vector<size_t>{};
    verts.reserve(mesh.mFaces[i].mNumIndices);

    for (unsigned int j = 0; j < mesh.mFaces[i].mNumIndices; j++)
    {
      verts.push_back(mesh.mFaces[i].mIndices[j] + offset);
    }

    faces.emplace_back(mesh.mMaterialIndex, verts);
  }
  return faces;
}

namespace
{

Assets::Texture loadTextureFromFileSystem(
  const std::filesystem::path& path, const FileSystem& fs, Logger& logger)
{
  return fs.openFile(path)
    .and_then([](auto file) {
      auto reader = file->reader().buffer();
      return readFreeImageTexture("", reader);
    })
    .or_else(makeReadTextureErrorHandler(fs, logger))
    .value();
}

Assets::Texture loadUncompressedEmbeddedTexture(
  const aiTexel* data, std::string name, const size_t width, const size_t height)
{
  auto buffer = Assets::TextureBuffer{width * height * sizeof(aiTexel)};
  std::memcpy(buffer.data(), data, width * height * sizeof(aiTexel));

  const auto averageColor = getAverageColor(buffer, GL_BGRA);
  return {
    std::move(name),
    width,
    height,
    averageColor,
    std::move(buffer),
    GL_BGRA,
    Assets::TextureType::Masked};
}

Assets::Texture loadCompressedEmbeddedTexture(
  const std::string& name,
  const aiTexel* data,
  const size_t size,
  const FileSystem& fs,
  Logger& logger)
{
  return readFreeImageTextureFromMemory(
           name, reinterpret_cast<const uint8_t*>(data), size)
    .or_else(makeReadTextureErrorHandler(fs, logger))
    .value();
}

std::optional<Assets::Texture> loadFallbackTexture(const FileSystem& fs)
{
  static const auto texturePaths = std::vector<std::filesystem::path>{
    "textures"
      / kdl::path_add_extension(Model::BrushFaceAttributes::NoTextureName, ".png"),
    "textures"
      / kdl::path_add_extension(Model::BrushFaceAttributes::NoTextureName, ".jpg"),
    kdl::path_add_extension(Model::BrushFaceAttributes::NoTextureName, ".png"),
    kdl::path_add_extension(Model::BrushFaceAttributes::NoTextureName, ".jpg"),
  };

  return kdl::select_first(texturePaths, [&](const auto& texturePath) {
    return fs.openFile(texturePath).and_then([](auto file) {
      auto reader = file->reader().buffer();
      return readFreeImageTexture("", reader);
    });
  });
}

} // namespace

Assets::Texture AssimpParser::processMaterial(
  const aiScene& scene, size_t materialIndex, Logger& logger) const
{
  try
  {
    // Is there even a single diffuse texture? If not, fail and load fallback material.
    if (scene.mMaterials[materialIndex]->GetTextureCount(aiTextureType_DIFFUSE) == 0)
    {
      throw Exception{"Material does not contain a texture."};
    }

    auto path = aiString{};
    scene.mMaterials[materialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &path);

    const auto texturePath = std::filesystem::path{path.C_Str()};
    const auto* texture = scene.GetEmbeddedTexture(path.C_Str());
    if (!texture)
    {
      // The texture is not embedded. Load it using the file system.
      const auto filePath = m_path.parent_path() / texturePath;
      return loadTextureFromFileSystem(filePath, m_fs, logger);
    }
    else if (texture->mHeight != 0)
    {
      // The texture is uncompressed, load it directly.
      return loadUncompressedEmbeddedTexture(
        texture->pcData, texture->mFilename.C_Str(), texture->mWidth, texture->mHeight);
    }
    else
    {
      // The texture is embedded, but compressed. Let FreeImage load it from memory.
      return loadCompressedEmbeddedTexture(
        texture->mFilename.C_Str(), texture->pcData, texture->mWidth, m_fs, logger);
    }
  }
  catch (Exception& exception)
  {
    // Materials aren't guaranteed to have a name.
    const auto materialName = scene.mMaterials[materialIndex]->GetName() != aiString{""}
                                ? scene.mMaterials[materialIndex]->GetName().C_Str()
                                : "nr. " + std::to_string(materialIndex + 1);
    logger.error(
      "Model " + m_path.string() + ": Loading fallback material for material "
      + materialName + ": " + exception.what());

    // Load fallback material in case we get any error.
    if (auto fallbackTexture = loadFallbackTexture(m_fs))
    {
      return std::move(*fallbackTexture);
    }
    else
    {
      return loadDefaultTexture(m_fs, materialName, logger);
    }
  }
}

aiMatrix4x4 AssimpParser::getAxisTransform(const aiScene& scene)
{
  auto matrix = aiMatrix4x4{};

  if (scene.mMetaData)
  {
    // These MUST be in32_t, or the metadata 'Get' function will get confused.
    int32_t upAxis = 0, frontAxis = 0, coordAxis = 0, upAxisSign = 0, frontAxisSign = 0,
            coordAxisSign = 0;
    float unitScale = 1.0f;

    const bool metadataPresent = scene.mMetaData->Get("UpAxis", upAxis)
                                 && scene.mMetaData->Get("UpAxisSign", upAxisSign)
                                 && scene.mMetaData->Get("FrontAxis", frontAxis)
                                 && scene.mMetaData->Get("FrontAxisSign", frontAxisSign)
                                 && scene.mMetaData->Get("CoordAxis", coordAxis)
                                 && scene.mMetaData->Get("CoordAxisSign", coordAxisSign)
                                 && scene.mMetaData->Get("UnitScaleFactor", unitScale);

    if (!metadataPresent)
    {
      // By default, all 3D data from is provided in a right-handed coordinate system.
      // +X to the right. -Z into the screen. +Y upwards.
      upAxis = 1;
      upAxisSign = 1;
      frontAxis = 2;
      frontAxisSign = 1;
      coordAxis = 0;
      coordAxisSign = 1;
      unitScale = 1.0f;
    }

    aiVector3D up, front, coord;
    up[static_cast<unsigned int>(upAxis)] = static_cast<float>(upAxisSign) * unitScale;
    front[static_cast<unsigned int>(frontAxis)] =
      static_cast<float>(frontAxisSign) * unitScale;
    coord[static_cast<unsigned int>(coordAxis)] =
      static_cast<float>(coordAxisSign) * unitScale;

    matrix = aiMatrix4x4t{
      coord.x,
      coord.y,
      coord.z,
      0.0f,
      -front.x,
      -front.y,
      -front.z,
      0.0f,
      up.x,
      up.y,
      up.z,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f};
  }

  return matrix;
}

} // namespace IO
} // namespace TrenchBroom
