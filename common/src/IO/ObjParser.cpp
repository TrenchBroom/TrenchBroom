/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "ObjParser.h"

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "Model/BrushFaceAttributes.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

#include <vecmath/forward.h>

#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <functional>
#include <string>

namespace TrenchBroom::IO
{

namespace
{
struct ObjVertexRef
{
  // Position index (1-based. Should always be present.)
  size_t m_position{0};
  // Texture coordinate index (Also 1-based. If not present, 0.)
  size_t m_texcoord{0};

  /**
   * Parses a vertex reference.
   *
   * @param text the text of this reference (such as "1/2/3")
   */
  explicit ObjVertexRef(const std::string& text)
  {
    const auto components = kdl::str_split(text, "/");
    auto components_num = std::vector<size_t>{};
    for (const std::string& com : components)
    {
      if (const auto csize = kdl::str_to_size(com))
      {
        components_num.push_back(*csize);
      }
      else
      {
        throw ParserException{"OBJ file has invalid number in vertex reference"};
      }
    }

    if (components_num.empty())
    {
      throw ParserException{"OBJ file has an empty vertex reference"};
    }

    if (components_num.size() == 1)
    {
      m_position = components_num[0];
    }
    else
    {
      m_position = components_num[0];
      m_texcoord = components_num[1];
    }
  }
};

struct ObjFace
{
  // The material of this face (as a skin index)
  size_t m_material{0};
  // The vertices of this face.
  std::vector<ObjVertexRef> m_vertices;
};

} // namespace

ObjParser::ObjParser(std::string name, const std::string_view text)
  : m_name{std::move(name)}
  , m_text{text}
{
}

std::unique_ptr<Assets::EntityModel> ObjParser::doInitializeModel(Logger& logger)
{
  // Model construction prestart (skins are added to this mid-parse)
  auto model = std::make_unique<Assets::EntityModel>(
    m_name, Assets::PitchType::Normal, Assets::Orientation::Oriented);
  model->addFrame();
  auto& surface = model->addSurface(m_name);

  auto textures = std::vector<Assets::Texture>{};

  // Load the default material (skin 0) ; must be present as a default for materialless
  // faces This default skin is used for all unloadable textures and all unspecified
  // textures. As such this implicitly covers situations where the default skin is
  // intended to be used, but is manually specified incorrectly.
  if (auto fallbackMaterial = loadFallbackMaterial(logger))
  {
    textures.push_back(std::move(*fallbackMaterial));
  }

  // Define the various OBJ parsing state.
  auto positions = std::vector<vm::vec3f>{};
  auto texcoords = std::vector<vm::vec2f>{};
  auto faces = std::vector<ObjFace>{};

  // Begin parsing.
  auto current_material = size_t(0);
  auto last_material = size_t(0);
  for (const auto& line : kdl::str_split(m_text, "\n"))
  {
    // logger.debug() << "obj line: " << line;
    const auto trimmed = kdl::str_trim(line);
    const auto tokens = kdl::str_split(trimmed, " \t");
    if (!tokens.empty())
    {
      if (tokens[0] == "v")
      {
        if (tokens.size() < 4)
        {
          throw ParserException{"OBJ file has a vertex with too few dimensions"};
        }

        // This can and should be replaced with a less Neverball-specific transform
        positions.emplace_back(
          std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
      }
      else if (tokens[0] == "vt")
      {
        if (tokens.size() < 3)
        {
          throw ParserException{"OBJ file has a texcoord with too few dimensions"};
        }

        texcoords.emplace_back(std::stof(tokens[1]), std::stof(tokens[2]));
      }
      else if (tokens[0] == "usemtl")
      {
        if (tokens.size() < 2)
        {
          // Assume they meant "use default material" (just in case; this doesn't really
          // make sense, but...)
          current_material = 0;
        }
        else
        {
          if (auto texture = loadMaterial(tokens[1]))
          {
            textures.push_back(std::move(*texture));
            ++last_material;
            current_material = last_material;
          }
          else
          {
            current_material = 0;
            logger.warn() << "unable to find OBJ model material " << tokens[1];
          }
        }
      }
      else if (tokens[0] == "f")
      {
        auto face = ObjFace{};
        face.m_material = current_material;
        for (size_t i = 1; i < tokens.size(); ++i)
        {
          face.m_vertices.emplace_back(tokens[i]);
        }
        faces.push_back(face);
      }
    }
  }
  surface.setSkins(std::move(textures));

  // Done parsing; transform (and get the 'reverse' flag for future use)
  const auto reverse = transformObjCoordinateSet(positions, texcoords);

  if (positions.empty())
  {
    // passing empty bounds as bbox crashes the program, don't let it happen
    throw ParserException{"OBJ file has no vertices (so no valid bounding box)"};
  }

  // Everything's in TrenchBroom Relative Coordinates! Build bounds.
  auto bounds = vm::bbox3f::builder{};
  bounds.add(std::begin(positions), std::end(positions));

  // Model construction prestart
  // Begin model construction, part 1. Collation
  auto totalVertexCount = size_t(0);
  auto size = Renderer::TexturedIndexRangeMap::Size{};
  for (const auto& face : faces)
  {
    size.inc(
      surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
    totalVertexCount += face.m_vertices.size();
  }

  // Model construction, part 2. Building
  auto& frame = model->loadFrame(0, m_name, bounds.bounds());
  auto builder = Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type>{
    totalVertexCount, size};

  // Actual build
  for (const auto& face : faces)
  {
    auto vertices = std::vector<Assets::EntityModelVertex>{};
    for (const auto& ref : face.m_vertices)
    {
      auto point = ref.m_position;
      if (point == 0)
      {
        throw ParserException{
          "OBJ file has vertex with no position (was this generated/parsed correctly?)"};
      }

      // As previously stated, OBJ file indexes are 1-based. This converts it to a 0-based
      // index.
      --point;
      if (point >= positions.size())
      {
        throw ParserException{
          "OBJ file has vertex referring to a position that hasn't been defined"};
      }

      auto texcoord = vm::vec2f{0, 0};
      if (ref.m_texcoord != 0)
      {
        const auto c = ref.m_texcoord - 1;
        if (c >= texcoords.size())
        {
          throw ParserException{
            "OBJ file has vertex referring to a texcoord that hasn't been defined"};
        }
        texcoord = texcoords[c];
      }

      if (reverse)
      {
        vertices.emplace(vertices.begin(), positions[point], texcoord);
      }
      else
      {
        vertices.emplace_back(positions[point], texcoord);
      }
    }
    builder.addPolygon(surface.skin(face.m_material), vertices);
  }

  surface.addTexturedMesh(
    frame, std::move(builder.vertices()), std::move(builder.indices()));

  return model;
}

// -- Neverball --

NvObjParser::NvObjParser(Path path, const std::string_view text, const FileSystem& fs)
  : ObjParser{path.lastComponent().string(), text}
  , m_path{std::move(path)}
  , m_fs{fs}
{
}

bool NvObjParser::canParse(const Path& path)
{
  return kdl::str_to_lower(path.extension().string()) == ".obj";
}

bool NvObjParser::transformObjCoordinateSet(
  std::vector<vm::vec3f>& positions, std::vector<vm::vec2f>& texcoords) const
{
  for (auto& pos : positions)
  {
    // The transform we want to perform is OBJ-To-MAP.
    // The transform used in make_body is MAP-To-OBJ, as Neverball uses the OBJ coordinate
    // space natively. The output is (X, Z, -Y); thus the inverse transform is (X, -Z, Y)
    pos[0] *= 64.0f;
    auto y = pos[1];
    pos[1] = pos[2] * -64.0f;
    pos[2] = y * 64.0f;
  }

  for (auto& uv : texcoords)
  {
    // This should be checked using the __TB_info_player_start model;
    // Blender-defaults-output files are consistent with Neverball.
    uv[1] = 1.0f - uv[1];
  }

  return true;
}

std::optional<Assets::Texture> NvObjParser::loadMaterial(const std::string& name) const
{
  // NOTE: A reasonable solution here would be to use the same material handling as the
  // brushes unless otherwise required. Then Neverball just gets an additional texture
  // search directory. But there's raw pointers all over the Texture system, so without
  // further details on how memory is managed there, that's a bad idea.

  auto texturePaths = std::vector<Path>{
    Path{"textures"} / Path{name}.addExtension(".png"),
    Path{"textures"} / Path{name}.addExtension(".jpg"),
    Path{name}.addExtension(".png"),
    Path{name}.addExtension(".jpg"),
  };


  for (const auto& texturePath : texturePaths)
  {
    try
    {
      const auto file = m_fs.openFile(texturePath);
      auto reader = file->reader().buffer();
      auto result = readFreeImageTexture("", reader);
      if (result.is_success())
      {
        return std::move(result).value();
      }
    }
    catch (const Exception&)
    {
      // ignore and try the next texture path
    }
  }

  return std::nullopt;
}

std::optional<Assets::Texture> NvObjParser::loadFallbackMaterial(Logger& logger) const
{
  // Try to remove the '.obj' extension and grab that as a texture.
  // This isn't really how it works, but the Neverball-side truth involves MAP files
  // acting as a replacement for something like JSON. This is a less Neverball-specific
  // set of logic which should be useful for any game.
  const auto basic_skin_name = m_path.lastComponent().deleteExtension().string();
  if (auto material = loadMaterial(basic_skin_name))
  {
    return material;
  }
  logger.warn() << "Loading fallback material for '" << basic_skin_name << "'";
  return loadMaterial(Model::BrushFaceAttributes::NoTextureName);
}

} // namespace TrenchBroom::IO
