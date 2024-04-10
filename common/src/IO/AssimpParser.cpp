/*
 Copyright (C) 2023 Daniel Walder
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
#include "IO/PathInfo.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "Model/BrushFaceAttributes.h"
#include "ReaderException.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <fmt/format.h>

#include <optional>
#include <string_view>
#include <utility>

namespace TrenchBroom::IO
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
    return 0;
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

std::optional<Assets::Texture> loadFallbackTexture(const FileSystem& fs)
{
  static const auto NoTextureName = Model::BrushFaceAttributes::NoTextureName;

  static const auto texturePaths = std::vector<std::filesystem::path>{
    "textures" / kdl::path_add_extension(NoTextureName, ".png"),
    "textures" / kdl::path_add_extension(NoTextureName, ".jpg"),
    kdl::path_add_extension(NoTextureName, ".png"),
    kdl::path_add_extension(NoTextureName, ".jpg"),
  };

  return kdl::select_first(texturePaths, [&](const auto& texturePath) {
    return fs.openFile(texturePath).and_then([](auto file) {
      auto reader = file->reader().buffer();
      return readFreeImageTexture("", reader);
    });
  });
}

Assets::Texture loadFallbackOrDefaultTexture(
  const FileSystem& fs, const std::string& defaultMaterialName, Logger& logger)
{
  if (auto fallbackTexture = loadFallbackTexture(fs))
  {
    return {std::move(*fallbackTexture)};
  }
  return {loadDefaultTexture(fs, defaultMaterialName, logger)};
}

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
  std::string name, const aiTexel& data, const size_t width, const size_t height)
{
  auto buffer = Assets::TextureBuffer{width * height * sizeof(aiTexel)};
  std::memcpy(buffer.data(), &data, width * height * sizeof(aiTexel));

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
  std::string name,
  const aiTexel& data,
  const size_t size,
  const FileSystem& fs,
  Logger& logger)
{
  return readFreeImageTextureFromMemory(
           std::move(name), reinterpret_cast<const uint8_t*>(&data), size)
    .or_else(makeReadTextureErrorHandler(fs, logger))
    .value();
}

Assets::Texture loadTexture(
  const aiTexture* texture,
  const std::filesystem::path& texturePath,
  const std::filesystem::path& modelPath,
  const FileSystem& fs,
  Logger& logger)
{
  if (!texture)
  {
    // The texture is not embedded. Load it using the file system.
    const auto filePath = modelPath.parent_path() / texturePath;
    return loadTextureFromFileSystem(filePath, fs, logger);
  }

  if (texture->mHeight != 0)
  {
    // The texture is uncompressed, load it directly.
    return loadUncompressedEmbeddedTexture(
      texture->mFilename.C_Str(), *texture->pcData, texture->mWidth, texture->mHeight);
  }

  // The texture is embedded, but compressed. Let FreeImage load it from memory.
  return loadCompressedEmbeddedTexture(
    texture->mFilename.C_Str(), *texture->pcData, texture->mWidth, fs, logger);
}

std::vector<Assets::Texture> loadTexturesForMaterial(
  const aiScene& scene,
  const size_t materialIndex,
  const std::filesystem::path& modelPath,
  const FileSystem& fs,
  Logger& logger)
{
  auto textures = std::vector<Assets::Texture>{};

  // Is there even a single diffuse texture? If not, fail and load fallback material.
  const auto textureCount =
    scene.mMaterials[materialIndex]->GetTextureCount(aiTextureType_DIFFUSE);
  if (textureCount > 0)
  {
    // load up every diffuse texture
    for (unsigned int ti = 0; ti < textureCount; ++ti)
    {
      auto path = aiString{};
      scene.mMaterials[materialIndex]->GetTexture(aiTextureType_DIFFUSE, ti, &path);

      const auto texturePath = std::filesystem::path{path.C_Str()};
      const auto* texture = scene.GetEmbeddedTexture(path.C_Str());
      textures.push_back(loadTexture(texture, texturePath, modelPath, fs, logger));
    }
  }
  else
  {
    logger.error(fmt::format(
      "No diffuse textures found for material {} of model '{}', loading fallback texture",
      materialIndex,
      modelPath.string()));

    // Materials aren't guaranteed to have a name.
    const auto materialName = scene.mMaterials[materialIndex]->GetName() != aiString{""}
                                ? scene.mMaterials[materialIndex]->GetName().C_Str()
                                : "nr. " + std::to_string(materialIndex + 1);
    textures.push_back(loadFallbackOrDefaultTexture(fs, materialName, logger));
  }

  return textures;
}

struct AssimpComputedMeshData
{
  size_t m_meshIndex;
  std::vector<Assets::EntityModelVertex> m_vertices;
  Assets::EntityModelIndices m_indices;
};

struct AssimpBoneInformation
{
  size_t m_boneIndex;
  int32_t m_parentIndex;
  aiString m_name;
  aiMatrix4x4 m_localTransform;
  aiMatrix4x4 m_globalTransform;
};

struct AssimpVertexBoneWeight
{
  size_t m_boneIndex;
  float m_weight;
  const aiBone& m_bone;
};

/**
 * Gets the index for a particular assimp mesh.
 * Assimp meshes do not have unique names so we match by reference.
 */
std::optional<size_t> getMeshIndex(const aiScene& scene, const aiMesh& mesh)
{
  for (unsigned int i = 0; i < scene.mNumMeshes; ++i)
  {
    if (&mesh == scene.mMeshes[i])
    {
      return size_t(i);
    }
  }
  return std::nullopt;
}

/**
 * Gets the channel index for a particular assimp node, matched by name.
 */
std::optional<size_t> getChannelIndex(const aiAnimation& animation, const aiNode& node)
{
  const auto nodeName = std::string_view{node.mName.data};
  for (unsigned int i = 0; i < animation.mNumChannels; ++i)
  {
    const auto channelName = std::string_view{animation.mChannels[i]->mNodeName.data};
    if (channelName == nodeName)
    {
      return size_t(i);
    }
  }
  return std::nullopt;
}

/**
 * Gets the parent bone's channel index as well as its transformation,
 * recursively multiplied by all bones further up the hierarchy.
 */
std::tuple<size_t, aiMatrix4x4> getBoneParentChannelAndTransformation(
  const aiAnimation& animation,
  const aiNode& boneNode,
  const std::vector<aiMatrix4x4>& channelTransforms)
{
  const auto* parentNode = boneNode.mParent;
  if (!parentNode)
  {
    // reached the root node
    return {-1, aiMatrix4x4{}};
  }

  if (const auto index = getChannelIndex(animation, *parentNode))
  {
    // we have found the index of this bone in the channel list
    const auto [parentIndex, parentTransform] =
      getBoneParentChannelAndTransformation(animation, *parentNode, channelTransforms);
    unused(parentIndex);

    return {*index, parentTransform * channelTransforms[*index]};
  }

  // this node is not a bone, use the node's default transformation
  return {-1, parentNode->mTransformation};
}

/**
 * Computes the animation information for each channel in an
 * animation sequence. Always uses the first frame of the animation.
 */
std::vector<AssimpBoneInformation> getAnimationInformation(
  const aiNode& root, const aiAnimation& animation)
{
  auto indivTransforms = std::vector<aiMatrix4x4>{};
  indivTransforms.reserve(animation.mNumChannels);

  // calculate the transformations for each animation channel
  for (unsigned int i = 0; i < animation.mNumChannels; ++i)
  {
    const auto& channel = *animation.mChannels[i];

    const auto position = channel.mNumPositionKeys > 0 ? channel.mPositionKeys[0].mValue
                                                       : aiVector3D{0, 0, 0};
    const auto rotation = channel.mNumRotationKeys > 0 ? channel.mRotationKeys[0].mValue
                                                       : aiQuaternion{1, 0, 0, 0};
    const auto scale =
      channel.mNumScalingKeys > 0 ? channel.mScalingKeys[0].mValue : aiVector3D{1, 1, 1};

    // build a transformation matrix from it
    auto mat = aiMatrix4x4{rotation.GetMatrix()};
    mat.a1 *= scale.x;
    mat.b1 *= scale.x;
    mat.c1 *= scale.x;
    mat.a2 *= scale.y;
    mat.b2 *= scale.y;
    mat.c2 *= scale.y;
    mat.a3 *= scale.z;
    mat.b3 *= scale.z;
    mat.c3 *= scale.z;
    mat.a4 = position.x;
    mat.b4 = position.y;
    mat.c4 = position.z;

    indivTransforms.push_back(std::move(mat));
  }

  // assemble the transform information from the bone hierarchy (child
  // bones must be multiplied by their parent transformations, recursively)
  auto transforms = std::vector<AssimpBoneInformation>{};
  transforms.reserve(animation.mNumChannels);

  for (unsigned int i = 0; i < animation.mNumChannels; ++i)
  {
    const auto& channel = *animation.mChannels[i];

    // traverse the bone hierarchy to compute the global transformation
    if (const auto* boneNode = root.FindNode(channel.mNodeName))
    {
      const auto [parentId, parentTransform] =
        getBoneParentChannelAndTransformation(animation, *boneNode, indivTransforms);

      transforms.push_back(
        {i,
         int32_t(parentId),
         channel.mNodeName,
         indivTransforms[i],
         parentTransform * indivTransforms[i]});
    }
    else
    {
      // couldn't find the bone node, something is weird
      transforms.emplace_back();
    }
  }

  return transforms;
}

void processNode(
  std::vector<AssimpMeshWithTransforms>& meshes,
  const aiNode& node,
  const aiScene& scene,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform)
{
  for (unsigned int i = 0; i < node.mNumMeshes; ++i)
  {
    const auto* mesh = scene.mMeshes[node.mMeshes[i]];
    meshes.push_back({mesh, transform, axisTransform});
  }

  for (unsigned int i = 0; i < node.mNumChildren; ++i)
  {
    processNode(
      meshes,
      *node.mChildren[i],
      scene,
      transform * node.mChildren[i]->mTransformation,
      axisTransform);
  }
}

const auto AiMdlHl1NodeBodyparts = "<MDL_bodyparts>";

void processRootNode(
  std::vector<AssimpMeshWithTransforms>& meshes,
  const aiNode& node,
  const aiScene& scene,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform)
{
  // HL1 models have a slightly different structure than normal, the format consists
  // of multiple body parts, and each body part has one or more submodels. Only one
  // submodel per body part should be rendered at a time.

  // See if we have loaded a HL1 model
  if (const auto hl1BodyParts = node.FindNode(AiMdlHl1NodeBodyparts))
  {
    // HL models are loaded by assimp in a particular way, each bodypart and all
    // its submodels are loaded into different nodes in the scene. To properly
    // display the model, we must choose EXACTLY ONE submodel from each body
    // part and render the meshes for those chosen submodels.

    // Assimp HL models face sideways by default so spin by -90 on the z axis
    // this MIGHT be needed for non-HL models as well. To be safe for now, we
    // only do this for HL models.
    const auto rotation = aiQuaternion{aiVector3t{0, 1, 0}, -vm::Cf::half_pi()};
    const auto rotMatrix = aiMatrix4x4{rotation.GetMatrix()};
    const auto newAxisTransform = axisTransform * rotMatrix;

    // loop through each body part
    for (unsigned int i = 0; i < hl1BodyParts->mNumChildren; ++i)
    {
      const auto& bodypart = *hl1BodyParts->mChildren[i];
      if (bodypart.mNumChildren > 0)
      {
        // currently we don't have a way to know which submodel the user
        // might want to see, so just use the first one.
        processNode(meshes, *bodypart.mChildren[0], scene, transform, newAxisTransform);
      }
    }
  }
  else
  {
    // not a HL1 model, just process like normal
    processNode(meshes, node, scene, transform, axisTransform);
  }
}

std::optional<size_t> getBoneIndexByName(
  const std::vector<AssimpBoneInformation>& boneTransforms, const aiBone& bone)
{
  const auto boneName = std::string_view{bone.mName.data};
  for (size_t i = 0; i < boneTransforms.size(); ++i)
  {
    const auto boneTransformName = std::string_view{boneTransforms[i].m_name.data};
    if (boneTransformName == boneName)
    {
      return i;
    }
  }
  return std::nullopt;
}

std::vector<Assets::EntityModelVertex> computeMeshVertices(
  const aiMesh& mesh,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform,
  const std::vector<AssimpBoneInformation>& boneTransforms)
{
  auto vertices = std::vector<Assets::EntityModelVertex>{};

  // We pass through the aiProcess_Triangulate flag to assimp, so we know for sure we'll
  // ONLY get triangles in a single mesh. This is just a safety net to make sure we don't
  // do anything bad.
  if (!(mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
  {
    return vertices;
  }

  // the weights for each vertex are stored in the bones, not in
  // the vertices. this loop lets us collect the bone weightings
  // per vertex so we can process them.
  auto weightsPerVertex =
    std::vector<std::vector<AssimpVertexBoneWeight>>{mesh.mNumVertices};
  for (unsigned int i = 0; i < mesh.mNumBones; ++i)
  {
    const auto& bone = *mesh.mBones[i];

    // find the bone with the matching name
    if (const auto index = getBoneIndexByName(boneTransforms, bone))
    {
      for (unsigned int j = 0; j < bone.mNumWeights; ++j)
      {
        weightsPerVertex[bone.mWeights[j].mVertexId].push_back(
          {*index, bone.mWeights[j].mWeight, bone});
      }
    }
  }

  const auto numVerts = mesh.mNumVertices;
  vertices.reserve(numVerts);

  // Add all the vertices of the mesh.
  for (unsigned int i = 0; i < numVerts; ++i)
  {
    const auto texcoords =
      mesh.mTextureCoords[0]
        ? vm::vec2f{mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y}
        : vm::vec2f{0.0f, 0.0f};

    auto meshVertices = mesh.mVertices[i];

    // Bone indices and weights
    if (mesh.HasBones() && !boneTransforms.empty() && !weightsPerVertex[i].empty())
    {
      const auto vertWeights = weightsPerVertex[i];
      auto vertPos = aiVector3D{};

      for (const auto& vertWeight : vertWeights)
      {
        const auto boneIndex = vertWeight.m_boneIndex;
        const auto weight = vertWeight.m_weight;
        const auto& bone = vertWeight.m_bone;
        if (boneIndex < boneTransforms.size())
        {
          const auto& boneTransform = boneTransforms[boneIndex];
          auto weightedPosition = meshVertices;
          weightedPosition *= bone.mOffsetMatrix;
          weightedPosition *= boneTransform.m_globalTransform;
          weightedPosition *= weight;
          vertPos += weightedPosition;
        }
      }

      meshVertices = vertPos;
    }

    meshVertices *= transform;
    meshVertices *= axisTransform;

    vertices.emplace_back(
      vm::vec3f{meshVertices.x, meshVertices.y, meshVertices.z}, texcoords);
  }

  return vertices;
}

aiMatrix4x4 getAxisTransform(const aiScene& scene)
{
  if (scene.mMetaData)
  {
    // These MUST be in32_t, or the metadata 'Get' function will get confused.
    int32_t upAxis = 0, frontAxis = 0, coordAxis = 0, upAxisSign = 0, frontAxisSign = 0,
            coordAxisSign = 0;
    auto unitScale = 1.0f;

    const auto metadataPresent = scene.mMetaData->Get("UpAxis", upAxis)
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

    auto up = aiVector3D{};
    auto front = aiVector3D{};
    auto coord = aiVector3D{};
    up[static_cast<unsigned int>(upAxis)] = float(upAxisSign) * unitScale;
    front[static_cast<unsigned int>(frontAxis)] = float(frontAxisSign) * unitScale;
    coord[static_cast<unsigned int>(coordAxis)] = float(coordAxisSign) * unitScale;

    return aiMatrix4x4t{
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

  return aiMatrix4x4{};
}

Result<void> loadSceneFrame(
  const aiScene& scene,
  const size_t frameIndex,
  Assets::EntityModel& model,
  const std::string& name)
{
  // load the animation information for the current "frame" (animation)
  const auto boneTransforms =
    frameIndex < scene.mNumAnimations
      ? getAnimationInformation(*scene.mRootNode, *scene.mAnimations[frameIndex])
      : std::vector<AssimpBoneInformation>{};

  auto meshes = std::vector<AssimpMeshWithTransforms>{};

  // Assimp files import as y-up. We must multiply the root transform with an axis
  // transform matrix.
  processRootNode(
    meshes,
    *scene.mRootNode,
    scene,
    scene.mRootNode->mTransformation,
    getAxisTransform(scene));

  // store the mesh data in a list so we can compute the bounding box before creating the
  // frame
  auto meshData = std::vector<AssimpComputedMeshData>{};
  auto bounds = vm::bbox3f::builder{};

  for (const auto& mesh : meshes)
  {
    if (const auto meshIndex = getMeshIndex(scene, *mesh.m_mesh))
    {
      const auto vertices = computeMeshVertices(
        *mesh.m_mesh, mesh.m_transform, mesh.m_axisTransform, boneTransforms);

      for (const auto& v : vertices)
      {
        bounds.add(v.attr);
      }

      // build the mesh faces as triangles
      const auto numTriangles = mesh.m_mesh->mNumFaces;
      const auto numIndices = numTriangles * 3;

      auto size = Renderer::IndexRangeMap::Size{};
      size.inc(Renderer::PrimType::Triangles, numTriangles);
      auto builder =
        Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{numIndices, size};

      for (unsigned int i = 0; i < numTriangles; ++i)
      {
        const auto& face = mesh.m_mesh->mFaces[i];

        // ignore anything that's not a triangle
        if (face.mNumIndices == 3)
        {
          builder.addTriangle(
            vertices[face.mIndices[0]],
            vertices[face.mIndices[1]],
            vertices[face.mIndices[2]]);
        }
      }

      meshData.push_back({*meshIndex, builder.vertices(), builder.indices()});
    }
  }

  if (!bounds.initialized())
  {
    // passing empty bounds as bbox crashes the program, don't let it happen
    return Error{"Model has no vertices. (So no valid bounding box.)"};
  }

  // we've processed the model, now we can create the frame and bind the meshes to it
  const auto frameBounds = bounds.bounds();
  auto& frame = model.loadFrame(frameIndex, name, frameBounds);

  for (const auto& data : meshData)
  {
    auto& surface = model.surface(data.m_meshIndex);
    surface.addIndexedMesh(frame, data.m_vertices, data.m_indices);
  }

  return Result<void>{};
}

} // namespace

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

std::unique_ptr<Assets::EntityModel> AssimpParser::initializeModel(
  TrenchBroom::Logger& logger)
{
  constexpr auto assimpFlags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
                               | aiProcess_FlipWindingOrder | aiProcess_SortByPType
                               | aiProcess_FlipUVs;

  const auto modelPath = m_path.string();

  // Import the file as an Assimp scene and populate our vectors.
  auto importer = Assimp::Importer{};
  importer.SetIOHandler(new AssimpIOSystem{m_fs});

  const auto* scene = importer.ReadFile(modelPath, assimpFlags);
  if (!scene)
  {
    throw ParserException{fmt::format(
      "Assimp couldn't import model from '{}': {}",
      m_path.string(),
      importer.GetErrorString())};
  }

  // Create model.
  auto model = std::make_unique<Assets::EntityModel>(
    modelPath, Assets::PitchType::Normal, Assets::Orientation::Oriented);

  // create a frame for each animation in the scene
  // if we have no animations, always load 1 frame for the reference model
  const auto numSequences = std::max(scene->mNumAnimations, 1u);
  for (size_t i = 0; i < numSequences; ++i)
  {
    model->addFrame();
  }

  // create a surface for each mesh in the scene and assign the skins/materials to it
  const auto numMeshes = scene->mNumMeshes;
  for (size_t i = 0; i < numMeshes; ++i)
  {
    const auto& mesh = scene->mMeshes[i];

    auto& surface = model->addSurface(scene->mMeshes[i]->mName.data);

    // an assimp mesh will only ever have one material, but a material can have multiple
    // alternatives (this is how assimp handles skins)

    // load skins for this surface
    surface.setSkins(
      {loadTexturesForMaterial(*scene, mesh->mMaterialIndex, m_path, m_fs, logger)});
  }

  return model;
}

void AssimpParser::loadFrame(
  size_t frameIndex, Assets::EntityModel& model, Logger& /* logger */)
{
  constexpr auto assimpFlags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
                               | aiProcess_FlipWindingOrder | aiProcess_SortByPType
                               | aiProcess_FlipUVs;

  const auto modelPath = m_path.string();

  // Import the file as an Assimp scene and populate our vectors.
  auto importer = Assimp::Importer{};
  importer.SetIOHandler(new AssimpIOSystem{m_fs});

  const auto* scene = importer.ReadFile(modelPath, assimpFlags);
  if (!scene)
  {
    throw ParserException{fmt::format(
      "Assimp couldn't import model from '{}': {}",
      m_path.string(),
      importer.GetErrorString())};
  }

  // load the requested frame
  loadSceneFrame(*scene, frameIndex, model, modelPath).transform_error([&](auto e) {
    throw ParserException{
      fmt::format("Assimp couldn't import model from '{}': {}", m_path.string(), e.msg)};
  });
}

} // namespace TrenchBroom::IO
