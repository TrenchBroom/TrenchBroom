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
#include "IO/PathInfo.h"
#include "IO/ReadFreeImageTexture.h"
#include "Logger.h"
#include "Model/BrushFaceAttributes.h"
#include "ReaderException.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

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
  const FileSystem& m_fs;
  std::shared_ptr<File> m_file;
  Reader m_reader;

protected:
  AssimpIOStream(const std::filesystem::path& path, const FileSystem& fs)
    : m_fs{fs}
    , m_file{m_fs.openFile(path)}
    , m_reader{m_file->reader()}
  {
  }

public:
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
    return new AssimpIOStream{path, m_fs};
  }
};

/**
 * Gets the channel index for a particular assimp node, matched by name.
 * Returns -1 if this node doesn't have a channel for this animation.
 */
int16_t getChannelIndex(const aiAnimation& animation, const aiNode& node)
{
  for (uint16_t b = 0; b < animation.mNumChannels; b++)
  {
    const aiNodeAnim* ch = animation.mChannels[b];
    if (!std::strcmp(node.mName.data, ch->mNodeName.data))
    {
      return static_cast<int16_t>(b);
    }
  }
  return -1;
}

/**
 * Computes the animation information for each channel in an
 * animation sequence. Always uses the first frame of the animation.
 */
std::vector<AssimpBoneInformation> getAnimationInformation(
  const aiNode& root, const aiAnimation& animation)
{
  auto indivTransforms = std::vector<aiMatrix4x4>{animation.mNumChannels};

  // calculate the transformations for each animation channel
  for (unsigned int a = 0; a < animation.mNumChannels; ++a)
  {
    const aiNodeAnim* channel = animation.mChannels[a];

    aiVector3D position(0, 0, 0);
    if (channel->mNumPositionKeys > 0)
    {
      position = channel->mPositionKeys[0].mValue;
    }

    aiQuaternion rotation(1, 0, 0, 0);
    if (channel->mNumRotationKeys > 0)
    {
      rotation = channel->mRotationKeys[0].mValue;
    }

    aiVector3D scale(1, 1, 1);
    if (channel->mNumScalingKeys > 0)
    {
      scale = channel->mScalingKeys[0].mValue;
    }

    // build a transformation matrix from it
    aiMatrix4x4& mat = indivTransforms[a];
    mat = aiMatrix4x4(rotation.GetMatrix());
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
  }

  // assemble the transform information from the bone hierarchy (child
  // bones must be multiplied by their parent transformations, recursively)
  auto transforms = std::vector<AssimpBoneInformation>{animation.mNumChannels};
  for (unsigned int a = 0; a < animation.mNumChannels; ++a)
  {
    const aiNodeAnim* channel = animation.mChannels[a];

    // start with the individual transformation of this channel
    auto globalTransform = indivTransforms[a];
    int32_t parentId = -1;

    // traverse the bone hierarchy to compute the global transformation
    auto boneNode = root.FindNode(channel->mNodeName);
    if (!boneNode)
    {
      continue; // couldn't find the bone node, something is weird
    }

    // start at the first parent and walk up the tree
    aiNode* parentNode = boneNode->mParent;
    while (parentNode)
    {
      // use the node's default transformation, in case node isn't a bone
      // and won't be transformed by this animation
      aiMatrix4x4 nodeTransformation = parentNode->mTransformation;

      // find the index of this bone in the channel list
      const auto b = getChannelIndex(animation, *parentNode);
      if (b >= 0)
      {
        nodeTransformation = indivTransforms[static_cast<size_t>(b)];

        // if this is the first parent of the bone, set the parent id
        if (parentNode == boneNode->mParent)
        {
          parentId = b;
        }
      }
      else
      {
        break;
      }

      globalTransform = nodeTransformation * globalTransform;
      parentNode = parentNode->mParent;
    }

    // set the info and carry on
    transforms[a] = AssimpBoneInformation{
      a, parentId, channel->mNodeName, indivTransforms[a], globalTransform};
  }

  return transforms;
}

} // namespace

std::unique_ptr<Assets::EntityModel> AssimpParser::doInitializeModel(
  TrenchBroom::Logger& logger)
{
  m_textures.clear();

  // Import the file as an Assimp scene and populate our vectors.
  auto importer = Assimp::Importer{};
  importer.SetIOHandler(new AssimpIOSystem{m_fs});

  // no post processing needed at this stage, since we only need to know how many frames
  // (animations) the model has
  const auto* scene = importer.ReadFile(m_path.string(), 0);

  if (!scene)
  {
    throw ParserException{
      std::string{"Assimp couldn't import the file: "} + importer.GetErrorString()};
  }

  // Create model
  auto model = std::make_unique<Assets::EntityModel>(
    m_path.string(), Assets::PitchType::Normal, Assets::Orientation::Oriented);

  // Create a frame for each animation in the scene
  // If we have no animations, always load 1 frame as the reference model
  const auto numSequences = std::max(scene->mNumAnimations, 1u);
  for (size_t i = 0; i < numSequences; ++i)
  {
    model->addFrame();
  }

  auto& surface = model->addSurface(m_path.string());

  // Load materials as textures
  processMaterials(*scene, logger);
  surface.setSkins(std::move(m_textures));

  // the entity browser will want to see frame 0 most of the time, pre-emptively load it
  loadSceneFrame(scene, 0, *model);

  return model;
}

void AssimpParser::doLoadFrame(
  const size_t frameIndex, Assets::EntityModel& model, Logger& /* logger */)
{
  // Import the file as an Assimp scene and populate our vectors
  auto importer = Assimp::Importer{};
  importer.SetIOHandler(new AssimpIOSystem{m_fs});

  // we'll be using the actual vertex data for this scene, specify some post-processing
  const auto* scene = importer.ReadFile(
    m_path.string(),
    aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType
      | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

  if (!scene)
  {
    throw ParserException{
      std::string{"Assimp couldn't import the file: "} + importer.GetErrorString()};
  }

  // load the requested frame
  loadSceneFrame(scene, frameIndex, model);
}

void AssimpParser::loadSceneFrame(
  const aiScene* scene, const size_t frameIndex, Assets::EntityModel& model)
{
  auto& surface = model.surface(0);

  m_positions.clear();
  m_vertices.clear();
  m_faces.clear();

  // load the animation information for the current "frame" (animation)
  std::vector<AssimpBoneInformation> boneTransforms;
  if (frameIndex < scene->mNumAnimations)
  {
    const auto animation = scene->mAnimations[frameIndex];
    boneTransforms = getAnimationInformation(*scene->mRootNode, *animation);
  }

  // Assimp files import as y-up. We must multiply the root transform with an axis
  // transform matrix.
  processRootNode(
    *scene->mRootNode,
    *scene,
    scene->mRootNode->mTransformation,
    get_axis_transform(*scene),
    boneTransforms);


  // Build bounds.
  auto bounds = vm::bbox3f::builder{};
  if (m_positions.empty())
  {
    // Passing empty bounds as bbox crashes the program, don't let it happen.
    throw ParserException{"Model has no vertices. (So no valid bounding box.)"};
  }
  else
  {
    bounds.add(std::begin(m_positions), std::end(m_positions));
  }

  // Begin model construction.
  // Part 1: Collation
  size_t totalVertexCount = 0;
  auto size = Renderer::TexturedIndexRangeMap::Size{};
  for (const auto& face : m_faces)
  {
    size.inc(
      surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
    totalVertexCount += face.m_vertices.size();
  }

  // Part 2: Building
  auto& frame = model.loadFrame(frameIndex, m_path.string(), bounds.bounds());
  auto builder = Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type>{
    totalVertexCount, size};

  for (const auto& face : m_faces)
  {
    auto entityVertices = kdl::vec_transform(face.m_vertices, [&](const auto& index) {
      return Assets::EntityModelVertex{
        m_positions[m_vertices[index].m_position], m_vertices[index].m_texcoords};
    });
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

#define AI_MDL_HL1_NODE_BODYPARTS "<MDL_bodyparts>"

void AssimpParser::processRootNode(
  const aiNode& node,
  const aiScene& scene,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform,
  const std::vector<AssimpBoneInformation>& boneTransforms)
{
  // HL1 models have a slightly different structure than normal, the format consists
  // of multiple body parts, and each body part has one or more submodels. Only one
  // submodel per body part should be rendered at a time.

  // See if we have loaded a HL1 model
  const auto hl1bodyParts = node.FindNode(AI_MDL_HL1_NODE_BODYPARTS);
  if (!hl1bodyParts)
  {
    // not a HL1 model, just process like normal
    processNode(node, scene, transform, axisTransform, boneTransforms);
    return;
  }

  /* HL models are loaded by assimp in a particular way, each bodypart and all
   * its submodels are loaded into different nodes in the scene. To properly
   * display the model, we must choose EXACTLY ONE submodel from each body
   * part and render the meshes for those chosen submodels. */

  // Assimp HL models face sideways by default so spin by -90 on the z axis
  // this MIGHT be needed for non-HL models as well. To be safe for now, we
  // only do this for HL models.
  const aiQuaternion rotation(aiVector3t(0, 1, 0), -vm::Cf::half_pi());
  const auto rotMatrix = aiMatrix4x4(rotation.GetMatrix());
  const auto newAxisTransform = axisTransform * rotMatrix;

  // loop through each body part
  for (unsigned int bpi = 0; bpi < hl1bodyParts->mNumChildren; bpi++)
  {
    const auto* bodypart = hl1bodyParts->mChildren[bpi];
    if (bodypart->mNumChildren == 0)
    {
      // the body has no submodels (shouldn't happen for a normal HL model)
      continue;
    }

    // for now, just pick the first submodel for each body part
    const auto* submodel = bodypart->mChildren[0];

    // for future implementation:
    /*
    // HL has a concept of a "body" keyvalue which allows the mapper
    // to set which submodels to use (e.g. change npc heads, carried weapons).
    // This would need to be added to the model spec and the doLoadFrame
    // parameters so it could be passed along to the parser.

    // The "body" keyvalue is treated similar to a bitflag field, with a
    // number of bytes for each submodel in the model held in the "base" value.

    int32_t bodyValue ; // passed in as an argument from the modelspec

    int32_t base;
    if (bodypart->mMetaData->Get("Base", base))
    {
      submodel = bodypart->mChildren[bodyValue % base];
      // we should also do some sanity checks for array bounds, etc
    }
    */

    processNode(*submodel, scene, transform, newAxisTransform, boneTransforms);
  }
}

void AssimpParser::processNode(
  const aiNode& node,
  const aiScene& scene,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform,
  const std::vector<AssimpBoneInformation>& boneTransforms)
{
  for (unsigned int i = 0; i < node.mNumMeshes; i++)
  {
    const auto* mesh = scene.mMeshes[node.mMeshes[i]];
    processMesh(*mesh, transform, axisTransform, boneTransforms);
  }
  for (unsigned int i = 0; i < node.mNumChildren; i++)
  {
    processNode(
      *node.mChildren[i],
      scene,
      transform * node.mChildren[i]->mTransformation,
      axisTransform,
      boneTransforms);
  }
}

struct vertexBoneWeight
{
  const size_t m_boneIndex;
  const float m_weight;
  const aiBone* m_bone;

  vertexBoneWeight(size_t boneIndex, float weight, const aiBone* bone)
    : m_boneIndex(boneIndex)
    , m_weight(weight)
    , m_bone(bone)
  {
  }
};

void AssimpParser::processMesh(
  const aiMesh& mesh,
  const aiMatrix4x4& transform,
  const aiMatrix4x4& axisTransform,
  const std::vector<AssimpBoneInformation>& boneTransforms)
{
  // the weights for each vertex are stored in the bones, not in
  // the vertices. this loop lets us collect the bone weightings
  // per vertex so we can process them.
  std::vector<std::vector<vertexBoneWeight>> weightsPerVertex(mesh.mNumVertices);
  for (unsigned int a = 0; a < mesh.mNumBones; a++)
  {
    const aiBone* bone = mesh.mBones[a];

    // find the bone with the matching name
    size_t idx;
    for (idx = 0; idx < boneTransforms.size(); idx++)
    {
      const auto info = boneTransforms[idx];
      if (!std::strcmp(info.m_name.data, bone->mName.data))
      {
        break;
      }
    }
    if (idx >= boneTransforms.size())
    {
      // couldn't find a bone with this name, skip it
      continue;
    }

    for (unsigned int b = 0; b < bone->mNumWeights; b++)
    {
      weightsPerVertex[bone->mWeights[b].mVertexId].emplace_back(
        idx, bone->mWeights[b].mWeight, bone);
    }
  }

  // Meshes have been sorted by primitive type, so we know for sure we'll ONLY get
  // triangles in a single mesh.
  if (mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
  {
    const auto offset = m_vertices.size();

    // Add all the vertices of the mesh
    for (unsigned int i = 0; i < mesh.mNumVertices; i++)
    {
      const auto texcoords =
        mesh.mTextureCoords[0]
          ? vm::vec2f{mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y}
          : vm::vec2f{0.0f, 0.0f};

      m_vertices.emplace_back(m_positions.size(), texcoords);

      auto meshVertices = mesh.mVertices[i];

      // Bone indices and weights
      if (mesh.HasBones() && !boneTransforms.empty() && !weightsPerVertex[i].empty())
      {
        const auto vertWeights = weightsPerVertex[i];
        auto vertPos = aiVector3D();

        for (const auto& vertWeight : vertWeights)
        {
          auto boneIndex = vertWeight.m_boneIndex;
          auto weight = vertWeight.m_weight;
          auto bone = vertWeight.m_bone;
          if (boneIndex < boneTransforms.size())
          {
            const auto& boneTransform = boneTransforms[boneIndex];
            auto weightedPosition = meshVertices;
            weightedPosition *= bone->mOffsetMatrix;
            weightedPosition *= boneTransform.m_globalTransform;
            weightedPosition *= weight;
            vertPos += weightedPosition;
          }
        }

        meshVertices = vertPos;
      }

      meshVertices *= transform;
      meshVertices *= axisTransform;

      m_positions.emplace_back(meshVertices.x, meshVertices.y, meshVertices.z);
    }

    for (unsigned int i = 0; i < mesh.mNumFaces; i++)
    {
      auto verts = std::vector<size_t>{};
      verts.reserve(mesh.mFaces[i].mNumIndices);

      for (unsigned int j = 0; j < mesh.mFaces[i].mNumIndices; j++)
      {
        verts.push_back(mesh.mFaces[i].mIndices[j] + offset);
      }
      m_faces.emplace_back(mesh.mMaterialIndex, verts);
    }
  }
}

namespace
{

Assets::Texture loadTextureFromFileSystem(
  const std::filesystem::path& path, const FileSystem& fs, Logger& logger)
{
  const auto file = fs.openFile(path);
  auto reader = file->reader().buffer();
  return readFreeImageTexture("", reader)
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
  std::string name,
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

  for (const auto& texturePath : texturePaths)
  {
    try
    {
      const auto file = fs.openFile(texturePath);
      auto reader = file->reader().buffer();
      auto result = readFreeImageTexture("", reader);
      if (result.is_success())
      {
        return std::move(result).value();
      }
    }
    catch (const Exception& /*ex1*/)
    {
      // ignore and try the next texture path
    }
  }

  return std::nullopt;
}

} // namespace

void AssimpParser::processMaterials(const aiScene& scene, Logger& logger)
{
  for (unsigned int i = 0; i < scene.mNumMaterials; i++)
  {
    try
    {
      // Is there even a single diffuse texture? If not, fail and load fallback material.
      if (scene.mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) == 0)
      {
        throw Exception{"Material does not contain a texture."};
      }

      auto path = aiString{};
      scene.mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);

      const auto texturePath = std::filesystem::path{path.C_Str()};
      const auto* texture = scene.GetEmbeddedTexture(path.C_Str());
      if (!texture)
      {
        // The texture is not embedded. Load it using the file system.
        const auto filePath = m_path.parent_path() / texturePath;
        m_textures.push_back(loadTextureFromFileSystem(filePath, m_fs, logger));
      }
      else if (texture->mHeight != 0)
      {
        // The texture is uncompressed, load it directly.
        m_textures.push_back(loadUncompressedEmbeddedTexture(
          texture->pcData,
          texture->mFilename.C_Str(),
          texture->mWidth,
          texture->mHeight));
      }
      else
      {
        // The texture is embedded, but compressed. Let FreeImage load it from memory.
        m_textures.push_back(loadCompressedEmbeddedTexture(
          texture->mFilename.C_Str(), texture->pcData, texture->mWidth, m_fs, logger));
      }
    }
    catch (Exception& exception)
    {
      // Load fallback material in case we get any error.
      if (auto fallbackTexture = loadFallbackTexture(m_fs))
      {
        m_textures.push_back(std::move(*fallbackTexture));
      }

      // Materials aren't guaranteed to have a name.
      const auto materialName = scene.mMaterials[i]->GetName() != aiString{""}
                                  ? scene.mMaterials[i]->GetName().C_Str()
                                  : "nr. " + std::to_string(i + 1);
      logger.error(
        "Model " + m_path.string() + ": Loading fallback material for material "
        + materialName + ": " + exception.what());
    }
  }
}

aiMatrix4x4 AssimpParser::get_axis_transform(const aiScene& scene)
{
  aiMatrix4x4 matrix = aiMatrix4x4();

  if (scene.mMetaData)
  {
    // These MUST be in32_t, or the metadata 'Get' function will get confused.
    int32_t upAxis = 0, frontAxis = 0, coordAxis = 0, upAxisSign = 0, frontAxisSign = 0,
            coordAxisSign = 0;
    float unitScale = 1.0f;

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

    aiVector3D up, front, coord;
    up[static_cast<unsigned int>(upAxis)] = static_cast<float>(upAxisSign) * unitScale;
    front[static_cast<unsigned int>(frontAxis)] =
      static_cast<float>(frontAxisSign) * unitScale;
    coord[static_cast<unsigned int>(coordAxis)] =
      static_cast<float>(coordAxisSign) * unitScale;

    matrix = aiMatrix4x4t(
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
      1.0f);
  }

  return matrix;
}

} // namespace IO
} // namespace TrenchBroom
