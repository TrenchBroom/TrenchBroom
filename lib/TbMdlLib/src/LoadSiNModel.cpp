/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/LoadSiNModel.h"

#include "fs/FileSystem.h"
#include "fs/ReaderException.h"
#include "gl/IndexRangeMap.h"
#include "gl/IndexRangeMapBuilder.h"
#include "gl/Material.h"
#include "gl/PrimType.h"
#include "mdl/LoadSkin.h"
#include "Tokenizer.h"

#include "kd/path_utils.h"
#include "kd/string_compare.h"

#include <array>
#include <vector>
#include <unordered_set>

namespace tb::mdl
{
namespace
{
namespace SiNLayout
{
constexpr int Ident = (('F' << 24) + ('E' << 16) + ('D' << 8) + 'S');
constexpr int BaseModelIdent = ((' ' << 24) + ('M' << 16) + ('B' << 8) + 'S');
constexpr int BaseModelVersion = 1;

constexpr int AnimModelIdent = ((' ' << 24) + ('M' << 16) + ('A' << 8) + 'S');
constexpr int AnimModelVersion = 1;

constexpr int MDL_GROUP_TWOSIDED = 0x8000;
} // namespace Md2Layout

constexpr auto Normals = std::array<vm::vec3f, 162>{{
  {-0.525731f, 0.000000f, 0.850651f},   {-0.442863f, 0.238856f, 0.864188f},
  {-0.295242f, 0.000000f, 0.955423f},   {-0.309017f, 0.500000f, 0.809017f},
  {-0.162460f, 0.262866f, 0.951056f},   {0.000000f, 0.000000f, 1.000000f},
  {0.000000f, 0.850651f, 0.525731f},    {-0.147621f, 0.716567f, 0.681718f},
  {0.147621f, 0.716567f, 0.681718f},    {0.000000f, 0.525731f, 0.850651f},
  {0.309017f, 0.500000f, 0.809017f},    {0.525731f, 0.000000f, 0.850651f},
  {0.295242f, 0.000000f, 0.955423f},    {0.442863f, 0.238856f, 0.864188f},
  {0.162460f, 0.262866f, 0.951056f},    {-0.681718f, 0.147621f, 0.716567f},
  {-0.809017f, 0.309017f, 0.500000f},   {-0.587785f, 0.425325f, 0.688191f},
  {-0.850651f, 0.525731f, 0.000000f},   {-0.864188f, 0.442863f, 0.238856f},
  {-0.716567f, 0.681718f, 0.147621f},   {-0.688191f, 0.587785f, 0.425325f},
  {-0.500000f, 0.809017f, 0.309017f},   {-0.238856f, 0.864188f, 0.442863f},
  {-0.425325f, 0.688191f, 0.587785f},   {-0.716567f, 0.681718f, -0.147621f},
  {-0.500000f, 0.809017f, -0.309017f},  {-0.525731f, 0.850651f, 0.000000f},
  {0.000000f, 0.850651f, -0.525731f},   {-0.238856f, 0.864188f, -0.442863f},
  {0.000000f, 0.955423f, -0.295242f},   {-0.262866f, 0.951056f, -0.162460f},
  {0.000000f, 1.000000f, 0.000000f},    {0.000000f, 0.955423f, 0.295242f},
  {-0.262866f, 0.951056f, 0.162460f},   {0.238856f, 0.864188f, 0.442863f},
  {0.262866f, 0.951056f, 0.162460f},    {0.500000f, 0.809017f, 0.309017f},
  {0.238856f, 0.864188f, -0.442863f},   {0.262866f, 0.951056f, -0.162460f},
  {0.500000f, 0.809017f, -0.309017f},   {0.850651f, 0.525731f, 0.000000f},
  {0.716567f, 0.681718f, 0.147621f},    {0.716567f, 0.681718f, -0.147621f},
  {0.525731f, 0.850651f, 0.000000f},    {0.425325f, 0.688191f, 0.587785f},
  {0.864188f, 0.442863f, 0.238856f},    {0.688191f, 0.587785f, 0.425325f},
  {0.809017f, 0.309017f, 0.500000f},    {0.681718f, 0.147621f, 0.716567f},
  {0.587785f, 0.425325f, 0.688191f},    {0.955423f, 0.295242f, 0.000000f},
  {1.000000f, 0.000000f, 0.000000f},    {0.951056f, 0.162460f, 0.262866f},
  {0.850651f, -0.525731f, 0.000000f},   {0.955423f, -0.295242f, 0.000000f},
  {0.864188f, -0.442863f, 0.238856f},   {0.951056f, -0.162460f, 0.262866f},
  {0.809017f, -0.309017f, 0.500000f},   {0.681718f, -0.147621f, 0.716567f},
  {0.850651f, 0.000000f, 0.525731f},    {0.864188f, 0.442863f, -0.238856f},
  {0.809017f, 0.309017f, -0.500000f},   {0.951056f, 0.162460f, -0.262866f},
  {0.525731f, 0.000000f, -0.850651f},   {0.681718f, 0.147621f, -0.716567f},
  {0.681718f, -0.147621f, -0.716567f},  {0.850651f, 0.000000f, -0.525731f},
  {0.809017f, -0.309017f, -0.500000f},  {0.864188f, -0.442863f, -0.238856f},
  {0.951056f, -0.162460f, -0.262866f},  {0.147621f, 0.716567f, -0.681718f},
  {0.309017f, 0.500000f, -0.809017f},   {0.425325f, 0.688191f, -0.587785f},
  {0.442863f, 0.238856f, -0.864188f},   {0.587785f, 0.425325f, -0.688191f},
  {0.688191f, 0.587785f, -0.425325f},   {-0.147621f, 0.716567f, -0.681718f},
  {-0.309017f, 0.500000f, -0.809017f},  {0.000000f, 0.525731f, -0.850651f},
  {-0.525731f, 0.000000f, -0.850651f},  {-0.442863f, 0.238856f, -0.864188f},
  {-0.295242f, 0.000000f, -0.955423f},  {-0.162460f, 0.262866f, -0.951056f},
  {0.000000f, 0.000000f, -1.000000f},   {0.295242f, 0.000000f, -0.955423f},
  {0.162460f, 0.262866f, -0.951056f},   {-0.442863f, -0.238856f, -0.864188f},
  {-0.309017f, -0.500000f, -0.809017f}, {-0.162460f, -0.262866f, -0.951056f},
  {0.000000f, -0.850651f, -0.525731f},  {-0.147621f, -0.716567f, -0.681718f},
  {0.147621f, -0.716567f, -0.681718f},  {0.000000f, -0.525731f, -0.850651f},
  {0.309017f, -0.500000f, -0.809017f},  {0.442863f, -0.238856f, -0.864188f},
  {0.162460f, -0.262866f, -0.951056f},  {0.238856f, -0.864188f, -0.442863f},
  {0.500000f, -0.809017f, -0.309017f},  {0.425325f, -0.688191f, -0.587785f},
  {0.716567f, -0.681718f, -0.147621f},  {0.688191f, -0.587785f, -0.425325f},
  {0.587785f, -0.425325f, -0.688191f},  {0.000000f, -0.955423f, -0.295242f},
  {0.000000f, -1.000000f, 0.000000f},   {0.262866f, -0.951056f, -0.162460f},
  {0.000000f, -0.850651f, 0.525731f},   {0.000000f, -0.955423f, 0.295242f},
  {0.238856f, -0.864188f, 0.442863f},   {0.262866f, -0.951056f, 0.162460f},
  {0.500000f, -0.809017f, 0.309017f},   {0.716567f, -0.681718f, 0.147621f},
  {0.525731f, -0.850651f, 0.000000f},   {-0.238856f, -0.864188f, -0.442863f},
  {-0.500000f, -0.809017f, -0.309017f}, {-0.262866f, -0.951056f, -0.162460f},
  {-0.850651f, -0.525731f, 0.000000f},  {-0.716567f, -0.681718f, -0.147621f},
  {-0.716567f, -0.681718f, 0.147621f},  {-0.525731f, -0.850651f, 0.000000f},
  {-0.500000f, -0.809017f, 0.309017f},  {-0.238856f, -0.864188f, 0.442863f},
  {-0.262866f, -0.951056f, 0.162460f},  {-0.864188f, -0.442863f, 0.238856f},
  {-0.809017f, -0.309017f, 0.500000f},  {-0.688191f, -0.587785f, 0.425325f},
  {-0.681718f, -0.147621f, 0.716567f},  {-0.442863f, -0.238856f, 0.864188f},
  {-0.587785f, -0.425325f, 0.688191f},  {-0.309017f, -0.500000f, 0.809017f},
  {-0.147621f, -0.716567f, 0.681718f},  {-0.425325f, -0.688191f, 0.587785f},
  {-0.162460f, -0.262866f, 0.951056f},  {0.442863f, -0.238856f, 0.864188f},
  {0.162460f, -0.262866f, 0.951056f},   {0.309017f, -0.500000f, 0.809017f},
  {0.147621f, -0.716567f, 0.681718f},   {0.000000f, -0.525731f, 0.850651f},
  {0.425325f, -0.688191f, 0.587785f},   {0.587785f, -0.425325f, 0.688191f},
  {0.688191f, -0.587785f, 0.425325f},   {-0.955423f, 0.295242f, 0.000000f},
  {-0.951056f, 0.162460f, 0.262866f},   {-1.000000f, 0.000000f, 0.000000f},
  {-0.850651f, 0.000000f, 0.525731f},   {-0.955423f, -0.295242f, 0.000000f},
  {-0.951056f, -0.162460f, 0.262866f},  {-0.864188f, 0.442863f, -0.238856f},
  {-0.951056f, 0.162460f, -0.262866f},  {-0.809017f, 0.309017f, -0.500000f},
  {-0.864188f, -0.442863f, -0.238856f}, {-0.951056f, -0.162460f, -0.262866f},
  {-0.809017f, -0.309017f, -0.500000f}, {-0.681718f, 0.147621f, -0.716567f},
  {-0.681718f, -0.147621f, -0.716567f}, {-0.850651f, 0.000000f, -0.525731f},
  {-0.688191f, 0.587785f, -0.425325f},  {-0.587785f, 0.425325f, -0.688191f},
  {-0.425325f, 0.688191f, -0.587785f},  {-0.425325f, -0.688191f, -0.587785f},
  {-0.587785f, -0.425325f, -0.688191f}, {-0.688191f, -0.587785f, -0.425325f},
}};

struct SinAnimHeader_t
{
   int         ident;
   int         version;

   char        name[64]; // unused/padding
   vm::vec3f   scale;      // multiply byte verts by this
   vm::vec3f   translate;  // then add this

   vm::vec3f   totaldelta; // total displacement of this animation
   float       totaltime;

   int         num_xyz;
   int         num_frames;
   int         ofs_frames;
   int         ofs_end;      // end of file
};

using SinSt_t = vm::vec2f;

struct SinTriangle_t
{
   vm::vec<int16_t, 3>     index_xyz;
   vm::vec<int16_t, 3>     index_st;
   int                     id;
};

struct SinVertex_t
{
	vm::vec<uint8_t, 3> v;              // scaled byte to fit in frame mins/maxs
	uint8_t             lightnormalindex;
};

struct SinFrame_t
{
   vm::vec3f   movedelta;  // used for driving the model around
   float       frametime;
   vm::vec3f   scale;     // multiply byte verts by this
   vm::vec3f   translate;  // then add this
   int         ofs_verts;
};

struct SinBaseHeader_t
{
   int            ident;
   int            version;

   int            num_xyz;
   int            num_st;       // greater than num_xyz for seams
   int            num_groups;

   int            ofs_st;       // byte offset from start for stverts
   int            ofs_end;      // end of file
};

struct SinGroup_t
{
   int         id;
   int         num_tris;
   int         num_glcmds;   // dwords in strip/fan command list
   int         ofs_glcmds;
   int         ofs_tris;
   int         ofs_end;
};

#if 0
struct Md2Vertex
{
  unsigned char x, y, z;
  unsigned char normalIndex;
};

struct Md2Frame
{
  vm::vec3f scale;
  vm::vec3f offset;
  std::string name;
  std::vector<Md2Vertex> vertices;

  vm::vec3f vertex(size_t index) const
  {
    const auto& vertex = vertices[index];
    const auto position = vm::vec3f{float(vertex.x), float(vertex.y), float(vertex.z)};
    return position * scale + offset;
  }

  const vm::vec3f& normal(size_t index) const
  {
    const auto& vertex = vertices[index];
    return Normals[vertex.normalIndex];
  }
};

struct Md2MeshVertex
{
  size_t vertexIndex;
  vm::vec2f uv;
};

struct Md2Mesh
{
  gl::PrimType type;
  std::vector<Md2MeshVertex> vertices;
};


std::vector<std::string> parseSkins(fs::Reader reader, const size_t count)
{
  auto skins = std::vector<std::string>{};
  skins.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    skins.push_back(reader.readString(Md2Layout::SkinNameLength));
  }

  return skins;
}

auto parseVertices(fs::Reader& reader, const size_t vertexCount)
{
  auto vertices = std::vector<Md2Vertex>{};
  vertices.reserve(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    const auto x = reader.readUnsignedChar<char>();
    const auto y = reader.readUnsignedChar<char>();
    const auto z = reader.readUnsignedChar<char>();
    const auto normalIndex = reader.readUnsignedChar<char>();
    vertices.push_back({x, y, z, normalIndex});
  }

  return vertices;
}

auto parseFrame(
  fs::Reader reader, const size_t /* frameIndex */, const size_t vertexCount)
{
  const auto scale = reader.readVec<float, 3>();
  const auto offset = reader.readVec<float, 3>();
  auto name = reader.readString(Md2Layout::FrameNameLength);
  auto vertices = parseVertices(reader, vertexCount);

  return Md2Frame{scale, offset, std::move(name), std::move(vertices)};
}

auto parseMeshVertices(fs::Reader& reader, const size_t count)
{
  auto vertices = std::vector<Md2MeshVertex>{};
  vertices.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    const auto u = reader.readFloat<float>();
    const auto v = reader.readFloat<float>();
    const auto vertexIndex = reader.readSize<int32_t>();
    vertices.push_back({vertexIndex, {u, v}});
  }

  return vertices;
}

auto parseMeshes(fs::Reader reader, const size_t /* commandCount */)
{
  auto meshes = std::vector<Md2Mesh>{};

  // vertex count is signed, where < 0 indicates a triangle fan and > 0 indicates a
  // triangle strip
  while (!reader.eof())
  {
    const auto vertexCount = reader.readInt<int32_t>();

    const auto type =
      vertexCount < 0 ? gl::PrimType::TriangleFan : gl::PrimType::TriangleStrip;
    auto vertices = parseMeshVertices(reader, size_t(std::abs(vertexCount)));
    meshes.push_back({type, std::move(vertices)});
  }

  return meshes;
}

void loadSkins(
  EntityModelSurface& surface,
  const std::vector<std::string>& skins,
  const Palette& palette,
  const fs::FileSystem& fs,
  Logger& logger)
{
  auto materials = std::vector<gl::Material>{};
  materials.reserve(skins.size());

  for (const auto& skin : skins)
  {
    materials.push_back(loadSkin(skin, fs, palette, logger));
  }

  surface.setSkins(std::move(materials));
}

auto getVertices(const Md2Frame& frame, const std::vector<Md2MeshVertex>& meshVertices)
{
  auto vertices = std::vector<EntityModelVertex>{};
  vertices.reserve(meshVertices.size());

  for (const auto& md2MeshVertex : meshVertices)
  {
    const auto position = frame.vertex(md2MeshVertex.vertexIndex);
    const auto& uv = md2MeshVertex.uv;

    vertices.emplace_back(position, uv);
  }

  return vertices;
}

void buildFrame(
  EntityModelData& model,
  EntityModelSurface& surface,
  const Md2Frame& frame,
  const std::vector<Md2Mesh>& meshes)
{
  size_t vertexCount = 0;
  auto size = gl::IndexRangeMap::Size{};
  for (const auto& md2Mesh : meshes)
  {
    vertexCount += md2Mesh.vertices.size();
    size.inc(md2Mesh.type);
  }

  auto bounds = vm::bbox3f::builder{};

  auto builder = gl::IndexRangeMapBuilder<EntityModelVertex::Type>{vertexCount, size};
  for (const auto& md2Mesh : meshes)
  {
    if (!md2Mesh.vertices.empty())
    {
      vertexCount += md2Mesh.vertices.size();
      const auto vertices = getVertices(frame, md2Mesh.vertices);

      bounds.add(std::begin(vertices), std::end(vertices), gl::GetVertexComponent<0>());

      if (md2Mesh.type == gl::PrimType::TriangleFan)
      {
        builder.addTriangleFan(vertices);
      }
      else if (md2Mesh.type == gl::PrimType::TriangleStrip)
      {
        builder.addTriangleStrip(vertices);
      }
    }
  }

  auto& modelFrame = model.addFrame(frame.name, bounds.bounds());
  surface.addMesh(
    modelFrame, std::move(builder.vertices()), std::move(builder.indices()));
}
#endif

} // namespace

bool canLoadSiNModel(const std::filesystem::path& path, fs::Reader reader)
{
  if (!kdl::path_has_extension(kdl::path_to_lower(path), ".def"))
  {
    return false;
  }

  const auto ident = reader.readInt<int32_t>();
  //const auto version = reader.readInt<int32_t>();

  return ident == SiNLayout::Ident;
}


namespace SiNDefToken
{
using Type = unsigned int;
static const Type Integer = 1 << 0;     // integer number
static const Type Decimal = 1 << 1;     // decimal number
static const Type String = 1 << 2;      // string
static const Type Comment = 1 << 3;     // line comment starting with // or /* */
static const Type Eof = 1 << 4;         // end of file
static const Type Eol = 1 << 5;         // end of line
static const Type Number = Integer | Decimal;
} // namespace SiNDefToken

auto tokenNames()
{
  using namespace SiNDefToken;

  return Tokenizer<SiNDefToken::Type>::TokenNameMap{
    {Integer, "integer"},
    {Decimal, "decimal"},
    {String, "string"},
    {Comment, "comment"},
    {Eof, "end of file"},
  };
}

class SiNDefTokenizer : public Tokenizer<SiNDefToken::Type>
{
private:
  static const std::string& NumberDelim()
  {
    static const std::string numberDelim(Whitespace() + ")");
    return numberDelim;
  }
  bool m_skipEol = true;

public:
    explicit SiNDefTokenizer(std::string_view str) : Tokenizer{tokenNames(), str, "\"", '\\'} { };

  void setSkipEol(bool skipEol){
      m_skipEol = skipEol;
  }

private:
  Token emitToken() override
  {
  while (!eof())
  {
    const auto startLine = line();
    const auto startColumn = column();
    const auto startLocation = location();
    const auto* c = curPos();
    switch (*c)
    {
    case '"': { // quoted string
      advance();
      c = curPos();
      const auto* e = readQuotedString('"', "\n}");
      return Token{SiNDefToken::String, c, e, offset(c), startLine, startColumn};
    }
    case '\r':
      if (lookAhead() == '\n')
      {
        advance();
      }
      // handle carriage return without consecutive linefeed
      // by falling through into the line feed case
      switchFallthrough();
    case '\n':
      if (!m_skipEol)
      {
        advance();
        return Token{SiNDefToken::Eol, c, c + 1, offset(c), startLine, startColumn};
      }
      switchFallthrough();
    case ' ':
    case '\t':
      discardWhile(Whitespace());
      break;
    case '/':
      if (lookAhead() == '/')
      {
        // parse single line comment starting with //
        advance(2);
        discardUntil("\n\r");
        // do not discard the terminating line break since it might be semantically
        // relevant e.g. for terminating a block entry
        break;
      }
      if (lookAhead() == '*')
      {
        // parse multiline comment delimited by /* and */
        advance(2);
        while (curChar() != '*' || lookAhead() != '/')
        {
          errorIfEof();
          advance();
        }
        advance(2);
        break;
      }
      // fall through into the default case to parse a string that starts with '/'
      switchFallthrough();
    default: // whitespace, integer, decimal or word
      if (const auto* e = readInteger(NumberDelim()))
      {
        return Token{SiNDefToken::Integer, c, e, offset(c), startLine, startColumn};
      }

      if (const auto e = readDecimal(NumberDelim()))
      {
        return Token{SiNDefToken::Decimal, c, e, offset(c), startLine, startColumn};
      }

      if (const auto e = readUntil(Whitespace()))
      {
        return Token{SiNDefToken::String, c, e, offset(c), startLine, startColumn};
      }

      throw ParserException{startLocation, fmt::format("Unexpected character: {}", *c)};
    }
  }
  return Token{SiNDefToken::Eof, nullptr, nullptr, length(), line(), column()};
  }
};

Result<EntityModelData> loadSiNModel(
  std::string name,
  fs::Reader reader,
  const fs::FileSystem& fs,
  Logger& logger)
{
  try
  {
    int ident = reader.readInt<int32_t>();

    if (ident != SiNLayout::Ident)
    {
      return Error{fmt::format("Unknown SiN DEF model ident: {}", ident)};
    }

    // stuff parsed from DEF
    std::string path;
    std::string model;
    std::string skin;
    std::string anim;
    vm::vec3f origin { 0.0f, 0.0f, 0.0f };
    float scale { 1.0f };
    std::unordered_set<int> twosided_groups;

    {
        fs::BufferedReader bufferedReader = reader.buffer();
        SiNDefTokenizer tokenizer(bufferedReader.stringView().substr(4));
        
        tokenizer.setSkipEol(true);

        while (tokenizer.skipAndPeekToken(SiNDefToken::Comment, SiNDefToken::String | SiNDefToken::Eof).hasType(SiNDefToken::String))
        {
            if (tokenizer.eof())
                break;

            std::string token = tokenizer.skipAndNextToken(SiNDefToken::Comment, SiNDefToken::String).data();
            
            tokenizer.setSkipEol(false);

            // start of server params, don't bother
            if (kdl::ci::str_is_prefix(token, "!")) {
                break;
            }
            
            // these take no "arguments"
            if (kdl::ci::str_is_equal("path", token)) {
                path = tokenizer.nextToken(SiNDefToken::String).data();
            } else if (kdl::ci::str_is_equal("scale", token)) {
                scale = tokenizer.nextToken(SiNDefToken::Number).toFloat<float>();
            } else if (kdl::ci::str_is_equal("origin", token)) {
                float x = tokenizer.nextToken(SiNDefToken::Number).toFloat<float>();
                float y = tokenizer.nextToken(SiNDefToken::Number).toFloat<float>();
                float z = tokenizer.nextToken(SiNDefToken::Number).toFloat<float>();
                origin = { x, y, z };
            } else if (kdl::ci::str_is_suffix(token, ".sbm")) {
                model = token;
            } else {
                // these can take optional arguments, so we need to parse all of them
                // and find the final one.
                std::vector<std::string> tokens { token };

                while (true)
                {
                    auto rawToken = tokenizer.nextToken();

                    if (auto type = rawToken.type(); (type == SiNDefToken::Eof || type == SiNDefToken::Eol))
                        break;

                    // NOTE: 'const std::string' returned here so we can't move it :/
                    tokens.push_back(rawToken.data());
                }

                if (kdl::ci::str_is_suffix(tokens.back(), ".tga")) {
                    if (skin.empty()) {
                        skin = tokens.back();
                    }
                } else if (kdl::ci::str_is_suffix(tokens.back(), ".sam")) {
                    if (anim.empty()) {
                        anim = tokens.back();
                    }
                } else if (kdl::ci::str_is_equal(tokens.front(), "id")) {
                    // group IDs are one-indexed for some reason
                    int id = atoi(tokens[1].c_str()) - 1;

                    for (int i = 4; i < tokens.size(); i++) {
                        if (kdl::ci::str_is_equal(tokens[i], "twosided")) {
                            twosided_groups.insert(id);
                            break;
                        }
                    }
                }

                tokenizer.setSkipEol(true);
                continue;
            }

            // easy-out, just skip to the EOL
            while (true)
            {
                if (auto type = tokenizer.peekToken().type(); (type == SiNDefToken::Eof || type == SiNDefToken::Eol))
                    break;

                tokenizer.nextToken();
            }

            tokenizer.setSkipEol(true);
        }
    }
    
	if (path.empty())
    {
        std::filesystem::path p{name};
        p.replace_extension();

        path = p.generic_string();
    }
	if (model.empty())
    {
        model = (std::filesystem::path{name}.replace_extension()
                 .filename())
                 .replace_extension(".sbm").generic_string();
    }

    // ensure both the SBM and SAM are available
    {
        std::filesystem::path sbm = std::filesystem::path{path} / model;
        std::filesystem::path sam = std::filesystem::path{path} / anim;
        
        auto sbmData = fs.openFile(sbm);

        if (!sbmData)
        {
          return Error{fmt::format("can't find SBM: {}", sbm.generic_string())};
        }

        auto samData = fs.openFile(sam);

        if (!samData)
        {
          return Error{fmt::format("can't find SAM: {}", sam.generic_string())};
        }

        // read SBM data
        auto sbmReader = sbmData.value()->reader();

        SinBaseHeader_t sbmHeader;
        sbmReader.read((unsigned char *) &sbmHeader, sizeof(sbmHeader));

        if (sbmHeader.ident != SiNLayout::BaseModelIdent)
        {
          return Error{fmt::format("SiN SBM model ident: {}", ident)};
        }

        if (sbmHeader.version != SiNLayout::BaseModelVersion)
        {
          return Error{fmt::format("SiN SBM model version: {}", ident)};
        }

        std::vector<SinGroup_t> sbmGroups;
        sbmGroups.resize(sbmHeader.num_groups);

        for (auto &group : sbmGroups)
            sbmReader.read((unsigned char *) &group, sizeof(group));

        std::vector<std::vector<SinTriangle_t>> sbmTriangles;
		sbmTriangles.resize(sbmHeader.num_groups);

        size_t num_tris = 0;

		for (int i = 0; i < sbmHeader.num_groups; i++)
		{
			num_tris += sbmGroups[i].num_tris;

            if (twosided_groups.find(sbmGroups[i].id) != twosided_groups.end())
                num_tris += sbmGroups[i].num_tris;

			sbmTriangles[i].resize(sbmGroups[i].num_tris);

            sbmReader.seekFromBegin((sizeof(SinBaseHeader_t) + (sizeof(SinGroup_t) * i)) + sbmGroups[i].ofs_tris);

			for (int t = 0; t < sbmGroups[i].num_tris; t++)
				sbmReader.read((unsigned char *) &sbmTriangles[i][t], sizeof(sbmTriangles[i][t]));
		}
        
        std::vector<SinSt_t> sbmSt;
        sbmSt.resize(sbmHeader.num_st);

        sbmReader.seekFromBegin(sbmHeader.ofs_st);

		for (int i = 0; i < sbmHeader.num_st; i++)
            sbmReader.read((unsigned char *) &sbmSt[i], sizeof(sbmSt[i]));

        // read SAM data
        auto samReader = samData.value()->reader();

        SinAnimHeader_t samHeader;
        samReader.read((unsigned char *) &samHeader, sizeof(samHeader));

        if (samHeader.ident != SiNLayout::AnimModelIdent)
        {
          return Error{fmt::format("SiN SBM model ident: {}", ident)};
        }

        if (samHeader.version != SiNLayout::AnimModelVersion)
        {
          return Error{fmt::format("SiN SBM model version: {}", ident)};
        }

	    // we only care about frame 0
	    SinFrame_t frame;

		samReader.seekFromBegin(samHeader.ofs_frames);
		samReader.read((unsigned char *) &frame, sizeof(frame));

	    std::vector<SinVertex_t> vertices;

		samReader.seekFromBegin(samHeader.ofs_frames + frame.ofs_verts);
		vertices.resize(samHeader.num_xyz);

		for (int i = 0; i < samHeader.num_xyz; i++)
			samReader.read((unsigned char *) &vertices[i], sizeof(vertices[i]));
        
        auto data = EntityModelData{PitchType::Normal, Orientation::Oriented};

        auto& surface = data.addSurface(name, 1);

        // skin
        {
            auto materials = std::vector<gl::Material>{};
            auto loadedSkin = loadSkin((std::filesystem::path{path} / skin), fs, std::nullopt, logger);
            materials.push_back(std::move(loadedSkin));
            surface.setSkins(std::move(materials));
        }

        // mesh
        {
          auto size = gl::IndexRangeMap::Size{};

          for (size_t i = 0; i < num_tris; i++)
              size.inc(gl::PrimType::Triangles);

          auto bounds = vm::bbox3f::builder{};

          auto builder = gl::IndexRangeMapBuilder<EntityModelVertex::Type>{num_tris * 3, size};

		  for (int i = 0; i < sbmGroups.size(); i++)
          {
            bool twosided = twosided_groups.find(sbmGroups[i].id) != twosided_groups.end();
		    for ( int t = 0; t < sbmGroups[i].num_tris; ++t )
		    {
              auto &xyz = sbmTriangles[i][t].index_xyz;
              auto &st = sbmTriangles[i][t].index_st;

              vm::vec3f voffset = (frame.translate + origin) * scale;
              vm::vec3f vscale = frame.scale * scale;

              builder.addTriangle(
                EntityModelVertex {
                  voffset + (vm::vec3f(vertices[xyz[0]].v) * vscale),
                  sbmSt[st[0]]
                },
                EntityModelVertex {
                  voffset + (vm::vec3f(vertices[xyz[1]].v) * vscale),
                  sbmSt[st[1]]
                },
                EntityModelVertex {
                  voffset + (vm::vec3f(vertices[xyz[2]].v) * vscale),
                  sbmSt[st[2]]
                }
              );
              
              if (twosided) {
                builder.addTriangle(
                      EntityModelVertex {
                        voffset + (vm::vec3f(vertices[xyz[2]].v) * vscale),
                        sbmSt[st[2]]
                      },
                      EntityModelVertex {
                        voffset + (vm::vec3f(vertices[xyz[1]].v) * vscale),
                        sbmSt[st[1]]
                      },
                      EntityModelVertex {
                        voffset + (vm::vec3f(vertices[xyz[0]].v) * vscale),
                        sbmSt[st[0]]
                      }
                    );
              }
            }
          }

            bounds.add(std::begin(builder.vertices()), std::end(builder.vertices()), gl::GetVertexComponent<0>());

            auto& modelFrame = data.addFrame("ident", bounds.bounds());
            surface.addMesh(modelFrame, std::move(builder.vertices()), std::move(builder.indices()));
        }

        return data;
    }
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
