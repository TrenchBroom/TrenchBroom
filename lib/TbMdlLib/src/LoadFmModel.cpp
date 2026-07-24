/*
 Copyright (C) 2026 MaxED
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/LoadFmModel.h"

#include "base/Logger.h"
#include "fs/ReaderException.h"
#include "gl/IndexRangeMapBuilder.h"
#include "mdl/LoadSkin.h"

#include "kd/path_utils.h"
#include "kd/ranges/to.h"

#include <optional>
#include <ranges>
#include <utility>

namespace tb::mdl
{
namespace
{
namespace FmLayout
{
constexpr uint32_t MaxVerts = 2048;
constexpr uint32_t MaxTriangles = 2048;
constexpr uint32_t MaxFrames = 2048;
constexpr uint32_t MaxSkinWidth = 640;
constexpr uint32_t MaxSkinHeight = 480;

constexpr int HeaderNameLength = 32;
constexpr int SkinNameLength = 64;
constexpr int FrameNameLength = 16;

// Initial Header.
const std::string HeaderName = "header";
constexpr uint32_t HeaderVersion = 2;

// Skin header.
const std::string SkinName = "skin";
constexpr uint32_t SkinVersion = 1;

// Frame header.
const std::string FrameName = "frames";
constexpr uint32_t FrameVersion = 1;

// GLCmds header.
const std::string GlCmdsName = "glcmds";
constexpr uint32_t GlCmdsVersion = 1;

// Mesh nodes header.
const std::string MeshNodesName = "mesh nodes";
constexpr uint32_t MeshNodesVersion = 3;

// Ignored blocks.
const std::set<std::string> IgnoredBlocks = {
  "st coord", "tris", "short frames", "normals", "comp data", "skeleton", "references"};
} // namespace FmLayout

struct FmBlockHeader
{
  char ident[FmLayout::HeaderNameLength];
  uint32_t version;
  uint32_t size;
};

struct FmHeader
{
  uint32_t skinwidth;
  uint32_t skinheight;
  uint32_t framesize; // Byte size of each frame.

  uint32_t num_skins;
  uint32_t num_xyz;
  uint32_t num_st; // Greater than num_xyz for seams.
  uint32_t num_tris;
  uint32_t num_glcmds; // dwords in strip/fan command list.
  uint32_t num_frames;
  uint32_t num_mesh_nodes;
};

struct FmTriVertex
{
  uint8_t v[3]; // Scaled byte to fit in frame mins/maxs.
  uint8_t lightnormalindex;
};

struct FmMeshNode
{
  short start_glcmds;
  short num_glcmds;
};

struct FmAliasFrame
{
  vm::vec3f scale;
  vm::vec3f translate;
  std::string name;
  std::vector<FmTriVertex> vertices;

  vm::vec3f vertex(const size_t index) const
  {
    const auto& vertex = vertices[index];
    const auto position = vm::vec3f{
      static_cast<float>(vertex.v[0]),
      static_cast<float>(vertex.v[1]),
      static_cast<float>(vertex.v[2])};

    return position * scale + translate;
  }
};

struct FmModel
{
  FmHeader header;
  std::vector<gl::Material> skins;
  std::vector<FmAliasFrame> frames;
  std::vector<FmMeshNode> mesh_nodes;
  std::vector<int> glcmds;
};

Result<FmHeader> loadModelHeader(fs::Reader& reader)
{
  const auto header = reader.read<FmHeader, FmHeader>();

  // Sanity checks.
  if (
    header.skinwidth < 1 || header.skinwidth > FmLayout::MaxSkinWidth
    || header.skinheight < 1 || header.skinheight > FmLayout::MaxSkinHeight)
  {
    return Error{fmt::format(
      "FM model has invalid skin size: {}x{}", header.skinwidth, header.skinheight)};
  }

  if (header.num_xyz < 1 || header.num_xyz > FmLayout::MaxVerts)
  {
    return Error{
      fmt::format("FM model has invalid number of vertices: {}", header.num_xyz)};
  }

  if (header.num_tris < 1 || header.num_tris > FmLayout::MaxTriangles)
  {
    return Error{
      fmt::format("FM model has invalid number of triangles: {}", header.num_tris)};
  }

  if (header.num_frames < 1 || header.num_frames > FmLayout::MaxFrames)
  {
    return Error{
      fmt::format("FM model has invalid number of frames: {}", header.num_frames)};
  }

  return header;
}

std::vector<gl::Material> loadSkins(
  const size_t numSkins, fs::Reader reader, const fs::FileSystem& fs, Logger& logger)
{
  // AppleClang doesn't compile this without the cast to uint32_t, but I don't know why.
  return std::views::iota(0u, uint32_t(numSkins))
         | std::views::transform([&](const auto) {
             const auto pathStr = reader.readString(FmLayout::SkinNameLength);
             const auto path = kdl::parse_path(pathStr);
             return loadSkin(path, fs, logger);
           })
         | kdl::ranges::to<std::vector>();
}

std::vector<FmTriVertex> loadVertices(const size_t numVertices, fs::Reader& reader)
{
  return std::views::iota(0u, numVertices) | std::views::transform([&](const auto) {
           return reader.read<FmTriVertex, FmTriVertex>();
         })
         | kdl::ranges::to<std::vector>();
}

std::vector<FmAliasFrame> loadFrames(
  const size_t numFrames, const size_t numVertices, fs::Reader reader)
{
  return std::views::iota(0u, numFrames) | std::views::transform([&](const auto) {
           return FmAliasFrame{
             .scale = reader.readVec<float, 3>(),
             .translate = reader.readVec<float, 3>(),
             .name = reader.readString(FmLayout::FrameNameLength),
             .vertices = loadVertices(numVertices, reader),
           };
         })
         | kdl::ranges::to<std::vector>();
}

std::vector<int32_t> loadGlCmds(const size_t numCommands, fs::Reader reader)
{
  return std::views::iota(0u, numCommands)
         | std::views::transform([&](const auto) { return reader.readInt<int32_t>(); })
         | kdl::ranges::to<std::vector>();
}

std::vector<FmMeshNode> loadMeshNodes(const size_t numMeshNodes, fs::Reader reader)
{
  return std::views::iota(0u, numMeshNodes) | std::views::transform([&](const auto) {
           reader.seekForward(512); // Skip byte tris[256] and byte verts[256].

           return FmMeshNode{
             .start_glcmds = static_cast<short>(reader.readInt<int16_t>()),
             .num_glcmds = static_cast<short>(reader.readInt<int16_t>()),
           };
         })
         | kdl::ranges::to<std::vector>();
}

Result<FmModel> loadModel(
  const FmHeader& modelHeader,
  fs::Reader reader,
  const fs::FileSystem& fs,
  Logger& logger)
{
  auto skins = std::optional<std::vector<gl::Material>>{};
  auto frames = std::optional<std::vector<FmAliasFrame>>{};
  auto meshNodes = std::optional<std::vector<FmMeshNode>>{};
  auto glCmds = std::optional<std::vector<int>>{};

  while (!reader.eof())
  {
    const auto blockHeader = reader.read<FmBlockHeader, FmBlockHeader>();
    const auto startPos = reader.position();

    if (blockHeader.ident == FmLayout::SkinName)
    {
      if (blockHeader.version != FmLayout::SkinVersion)
      {
        return Error{fmt::format("Unexpected skin version {}", blockHeader.version)};
      }

      skins = loadSkins(modelHeader.num_skins, reader, fs, logger);
    }
    else if (blockHeader.ident == FmLayout::FrameName)
    {
      if (blockHeader.version != FmLayout::FrameVersion)
      {
        return Error{fmt::format("Unexpected frame version {}", blockHeader.version)};
      }

      frames = loadFrames(modelHeader.num_frames, modelHeader.num_xyz, reader);
    }
    else if (blockHeader.ident == FmLayout::GlCmdsName)
    {
      if (blockHeader.version != FmLayout::GlCmdsVersion)
      {
        return Error{
          fmt::format("Unexpected GL commands version {}", blockHeader.version)};
      }

      glCmds = loadGlCmds(modelHeader.num_glcmds, reader);
    }
    else if (blockHeader.ident == FmLayout::MeshNodesName)
    {
      if (blockHeader.version != FmLayout::MeshNodesVersion)
      {
        return Error{
          fmt::format("Unexpected mesh nodes version {}", blockHeader.version)};
      }
      meshNodes = loadMeshNodes(modelHeader.num_mesh_nodes, reader);
    }
    else if (!FmLayout::IgnoredBlocks.contains(blockHeader.ident))
    {
      return Error{fmt::format("Unexpected block {}", blockHeader.ident)};
    }

    // Go to next block.
    reader.seekFromBegin(startPos + blockHeader.size);
  }

  if (!skins)
  {
    return Error{"Missing skins block"};
  }
  if (!frames)
  {
    return Error{"Missing frames block"};
  }
  if (!meshNodes)
  {
    return Error{"Missing mesh nodes block"};
  }
  if (!glCmds)
  {
    return Error{"Missing GL commands block"};
  }

  return FmModel{
    .header = modelHeader,
    .skins = std::move(*skins),
    .frames = std::move(*frames),
    .mesh_nodes = std::move(*meshNodes),
    .glcmds = std::move(*glCmds),
  };
}

size_t initFrame(const FmModel& fmdl, gl::IndexRangeMap::Size& size)
{
  int num_verts = 0;

  // Count used verts and GL primitive types.
  for (uint32_t i = 0; i < fmdl.header.num_mesh_nodes; i++)
  {
    int prim_pos = fmdl.mesh_nodes[i].start_glcmds;

    while (true)
    {
      int num_prim_verts = fmdl.glcmds[static_cast<uint32_t>(prim_pos)];
      if (num_prim_verts == 0)
      {
        break; // Done.
      }

      if (num_prim_verts < 0)
      {
        num_prim_verts = -num_prim_verts;
        size.inc(gl::PrimType::TriangleFan);
      }
      else
      {
        size.inc(gl::PrimType::TriangleStrip);
      }

      num_verts += num_prim_verts;
      prim_pos += num_prim_verts * 3 + 1; // Skip verts and 'num_prim_verts' slot.
    }
  }

  return static_cast<size_t>(num_verts);
}

void buildFrame(
  const FmModel& fmdl,
  const uint32_t frame_index,
  vm::bbox3f::builder& bounds,
  gl::IndexRangeMapBuilder<EntityModelVertex::Type>& builder)
{
  for (uint32_t i = 0; i < fmdl.header.num_mesh_nodes; i++)
  {
    // Get the vertex count and primitive type.
    const auto& node = fmdl.mesh_nodes[i];
    int prim_pos = node.start_glcmds;

    while (prim_pos - node.start_glcmds < node.num_glcmds)
    {
      int num_verts = fmdl.glcmds[static_cast<uint32_t>(prim_pos)];
      if (num_verts == 0)
      {
        break; // Done.
      }

      gl::PrimType prim_type;
      if (num_verts < 0)
      {
        num_verts = -num_verts;
        prim_type = gl::PrimType::TriangleFan;
      }
      else
      {
        prim_type = gl::PrimType::TriangleStrip;
      }

      auto vertices = std::vector<EntityModelVertex>{};
      vertices.reserve(static_cast<uint32_t>(num_verts));

      prim_pos++;

      for (int c = 0; c < num_verts; c++)
      {
        union IntFloat
        {
          int i;
          float f;
        };

        const IntFloat s = {.i = fmdl.glcmds[static_cast<uint32_t>(prim_pos + 0)]};
        const IntFloat t = {.i = fmdl.glcmds[static_cast<uint32_t>(prim_pos + 1)]};
        const auto vert_index =
          static_cast<uint32_t>(fmdl.glcmds[static_cast<uint32_t>(prim_pos + 2)]);

        const auto position = fmdl.frames[frame_index].vertex(vert_index);
        vertices.emplace_back(position, vm::vec2f{s.f, t.f});

        prim_pos += 3;
      }

      bounds.add(std::begin(vertices), std::end(vertices), gl::GetVertexComponent<0>());

      if (prim_type == gl::PrimType::TriangleFan)
      {
        builder.addTriangleFan(vertices);
      }
      else // PrimType::TriangleStrip
      {
        builder.addTriangleStrip(vertices);
      }
    }
  }
}

EntityModelData buildModel(FmModel fmdl, std::string name)
{
  auto data = EntityModelData{PitchType::Normal, Orientation::Oriented};
  auto& surface = data.addSurface(std::move(name), fmdl.header.num_frames);
  surface.setSkins(std::move(fmdl.skins));

  for (uint32_t i = 0; i < fmdl.header.num_frames; i++)
  {
    // Init frame data.
    auto size = gl::IndexRangeMap::Size{};
    const size_t num_verts = initFrame(fmdl, size);

    // Build current frame.
    auto bounds = vm::bbox3f::builder{};
    auto builder = gl::IndexRangeMapBuilder<EntityModelVertex::Type>{num_verts, size};
    buildFrame(fmdl, i, bounds, builder);

    // Add to editor model.
    auto& mdl_frame = data.addFrame(fmdl.frames[i].name, bounds.bounds());
    surface.addMesh(
      mdl_frame, std::move(builder.vertices()), std::move(builder.indices()));
  }

  return data;
}

} // namespace

bool canLoadFmModel(const std::filesystem::path& path, fs::Reader reader)
{
  if (!kdl::path_has_extension(kdl::path_to_lower(path), ".fm"))
  {
    return false;
  }

  const auto ident = reader.readString(FmLayout::HeaderNameLength);
  const auto version = reader.readInt<int32_t>();

  return ident == FmLayout::HeaderName && version == FmLayout::HeaderVersion;
}

Result<EntityModelData> loadFmModel(
  std::string name, fs::Reader reader, const fs::FileSystem& fs, Logger& logger)
{
  try
  {
    // Load and validate block header.
    const auto header = reader.read<FmBlockHeader, FmBlockHeader>();

    if (header.ident != FmLayout::HeaderName)
    {
      return Error{fmt::format("Unknown FM model id: '{}'", header.ident)};
    }

    if (header.version != FmLayout::HeaderVersion)
    {
      return Error{fmt::format("Unknown FM model version: {}", header.version)};
    }

    // Load and validate model header (expected to be the first block).
    return loadModelHeader(reader) | kdl::and_then([&](const auto& modelHeader) {
             return loadModel(modelHeader, reader, fs, logger);
           })
           | kdl::transform(
             [&](auto model) { return buildModel(std::move(model), std::move(name)); });
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
