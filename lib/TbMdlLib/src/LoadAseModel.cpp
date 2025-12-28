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

#include "mdl/LoadAseModel.h"

#include "Logger.h"
#include "Parser.h"
#include "Tokenizer.h"
#include "gl/Material.h" // IWYU pragma: keep
#include "gl/MaterialIndexRangeMap.h"
#include "gl/MaterialIndexRangeMapBuilder.h"
#include "mdl/MaterialUtils.h"

#include "kd/k.h"
#include "kd/path_utils.h"

#include <map>

namespace tb::mdl
{
namespace
{

namespace AseToken
{
using Type = unsigned int;
constexpr Type Directive = 1 << 0;    // Any directive, i.e. *SCENE
constexpr Type OBrace = 1 << 1;       // opening brace: {
constexpr Type CBrace = 1 << 2;       // closing brace: }
constexpr Type String = 1 << 3;       // quoted string: "..."
constexpr Type Integer = 1 << 4;      // integer number
constexpr Type Decimal = 1 << 5;      // decimal number
constexpr Type Keyword = 1 << 6;      // keyword: Filter etc.
constexpr Type ArgumentName = 1 << 7; // argument name: A:, B: etc.
constexpr Type Colon = 1 << 8;        // colon: :
constexpr Type Eof = 1 << 12;         // end of file
} // namespace AseToken


auto tokenNames()
{
  using namespace AseToken;

  return Tokenizer<Type>::TokenNameMap{
    {Directive, "directive"},
    {OBrace, "'{'"},
    {CBrace, "'}'"},
    {String, "quoted string"},
    {Integer, "integer"},
    {Decimal, "decimal"},
    {Keyword, "keyword"},
    {ArgumentName, "argument name"},
    {Colon, "':'"},
    {Eof, "end of file"},
  };
}

class AseTokenizer : public Tokenizer<AseToken::Type>
{
private:
  static constexpr const std::string WordDelims = " \t\n\r:";

public:
  explicit AseTokenizer(std::string_view str)
    : Tokenizer{tokenNames(), str, "", 0}
  {
  }

private:
  Token emitToken() override
  {
    while (!eof())
    {
      auto startLine = line();
      auto startColumn = column();
      const auto* c = curPos();

      switch (*c)
      {
      case '*': {
        advance();
        c = curPos();
        const auto* e = readUntil(WordDelims);
        return Token{AseToken::Directive, c, e, offset(c), startLine, startColumn};
      }
      case '{':
        advance();
        return Token{AseToken::OBrace, c, c + 1, offset(c), startLine, startColumn};
      case '}':
        advance();
        return Token{AseToken::CBrace, c, c + 1, offset(c), startLine, startColumn};
      case ':': {
        advance();
        return Token{AseToken::Colon, c, c + 1, offset(c), startLine, startColumn};
      }
      case '"': { // quoted string
        advance();
        c = curPos();
        const auto* e = readQuotedString();
        return Token{AseToken::String, c, e, offset(c), startLine, startColumn};
      }
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        discardWhile(Whitespace());
        break;
      default: {
        const auto* e = readInteger(WordDelims);
        if (e)
        {
          return Token{AseToken::Integer, c, e, offset(c), startLine, startColumn};
        }

        e = readDecimal(WordDelims);
        if (e)
        {
          return Token{AseToken::Decimal, c, e, offset(c), startLine, startColumn};
        }

        // must be a keyword or argument name
        e = readUntil(WordDelims);
        if (e)
        {
          if (*e == ':')
          {
            // we don't return the colon as a separate token in this case
            advance();
            return Token{AseToken::ArgumentName, c, e, offset(c), startLine, startColumn};
          }
          else
          {
            return Token{AseToken::Keyword, c, e, offset(c), startLine, startColumn};
          }
        }
        throw ParserException{
          FileLocation{startLine, startColumn},
          "Unexpected character: '" + std::string(c, 1) + "'"};
      }
      }
    }
    return Token{AseToken::Eof, nullptr, nullptr, length(), line(), column()};
  }
};

class AseLoader : public Parser<AseToken::Type>
{
private:
  using Token = AseTokenizer::Token;

  struct MeshFaceVertex
  {
    size_t vertexIndex = 0u;
    size_t uvIndex = 0u;
  };

  struct MeshFace
  {
    std::array<MeshFaceVertex, 3> vertices;
    size_t line = 0u;
  };

  struct Mesh
  {
    std::vector<vm::vec3f> vertices;
    std::vector<vm::vec2f> uv;
    std::vector<MeshFace> faces;
  };

  struct GeomObject
  {
    std::string name;
    Mesh mesh;
    size_t materialIndex = 0u;
    size_t line = 0u;
  };

  struct Scene
  {
    std::vector<std::filesystem::path> materialPaths;
    std::vector<GeomObject> geomObjects;
  };

  std::string m_name;
  AseTokenizer m_tokenizer;
  LoadMaterialFunc m_loadMaterial;

  Logger& m_logger;

public:
  AseLoader(
    std::string name, std::string_view str, LoadMaterialFunc loadMaterial, Logger& logger)
    : m_name{std::move(name)}
    , m_tokenizer{str}
    , m_loadMaterial{std::move(loadMaterial)}
    , m_logger{logger}

  {
  }

  Result<EntityModelData> load()
  {
    try
    {
      auto scene = Scene{};
      parseAseFile(scene);
      return buildModelData(scene);
    }
    catch (const ParserException& e)
    {
      return Error{e.what()};
    }
  }


private: // parsing
  void parseAseFile(Scene& scene)
  {
    expectDirective("3DSMAX_ASCIIEXPORT");
    m_tokenizer.nextToken(AseToken::Integer);

    skipDirective("COMMENT");

    parseScene();
    parseMaterialList(scene.materialPaths);

    while (!m_tokenizer.peekToken().hasType(AseToken::Eof))
    {
      auto geomObject = GeomObject{};
      parseGeomObject(geomObject, scene.materialPaths);
      scene.geomObjects.emplace_back(std::move(geomObject));
    }
  }

  // SCENE
  void parseScene() { skipDirective("SCENE"); }

  // MATERIALS
  void parseMaterialList(std::vector<std::filesystem::path>& paths)
  {
    expectDirective("MATERIAL_LIST");

    parseBlock(
      {{"MATERIAL_COUNT",
        std::bind(&AseLoader::parseMaterialListMaterialCount, this, std::ref(paths))},
       {"MATERIAL",
        std::bind(&AseLoader::parseMaterialListMaterial, this, std::ref(paths))}});
  }

  void parseMaterialListMaterialCount(std::vector<std::filesystem::path>& paths)
  {
    expectDirective("MATERIAL_COUNT");
    paths.resize(parseSizeArgument());
  }

  void parseMaterialListMaterial(std::vector<std::filesystem::path>& paths)
  {
    expectDirective("MATERIAL");
    const auto index = parseSizeArgument();
    if (index < paths.size())
    {
      std::string name;
      auto& path = paths[index];

      parseBlock(
        {{"MAP_DIFFUSE",
          std::bind(
            &AseLoader::parseMaterialListMaterialMapDiffuse, this, std::ref(path))},
         {"MATERIAL_NAME",
          std::bind(&AseLoader::parseMaterialListMaterialName, this, std::ref(name))}});

      if (path.empty())
      {
        m_logger.warn()
          << "Material " << index
          << " is missing a 'BITMAP' directive, falling back to material name '" << name
          << "'";
        path = std::filesystem::path(name);
      }
    }
    else
    {
      m_logger.warn() << "Material index " << index << " is out of bounds.";
      parseBlock({});
    }
  }

  void parseMaterialListMaterialName(std::string& name)
  {
    expectDirective("MATERIAL_NAME");
    const auto token = m_tokenizer.nextToken(AseToken::String);
    name = token.data();
  }

  void parseMaterialListMaterialMapDiffuse(std::filesystem::path& path)
  {
    expectDirective("MAP_DIFFUSE");

    parseBlock(
      {{"BITMAP",
        std::bind(
          &AseLoader::parseMaterialListMaterialMapDiffuseBitmap, this, std::ref(path))}});
  }

  void parseMaterialListMaterialMapDiffuseBitmap(std::filesystem::path& path)
  {
    expectDirective("BITMAP");
    const auto token = m_tokenizer.nextToken(AseToken::String);
    path = kdl::parse_path(token.data(), K(convert_separators));
  }

  void parseGeomObject(
    GeomObject& geomObject, const std::vector<std::filesystem::path>& materialPaths)
  {
    expectDirective("GEOMOBJECT");

    parseBlock(
      {{"NODE_NAME",
        std::bind(&AseLoader::parseGeomObjectNodeName, this, std::ref(geomObject))},
       {"MATERIAL_REF",
        std::bind(
          &AseLoader::parseGeomObjectMaterialRef,
          this,
          std::ref(geomObject),
          materialPaths.size())},
       {"MESH",
        std::bind(&AseLoader::parseGeomObjectMesh, this, std::ref(geomObject.mesh))}});
  }

  void parseGeomObjectNodeName(GeomObject& geomObject)
  {
    expectDirective("NODE_NAME");
    const auto token = m_tokenizer.nextToken(AseToken::String);
    geomObject.name = token.data();
  }

  void parseGeomObjectMaterialRef(GeomObject& geomObject, size_t materialCount)
  {
    expectDirective("MATERIAL_REF");
    const auto token = m_tokenizer.peekToken();
    geomObject.materialIndex = parseSizeArgument();
    if (geomObject.materialIndex >= materialCount)
    {
      m_logger.warn() << "Line " << token.line() << ": Material index "
                      << geomObject.materialIndex
                      << " is out of bounds (material count: " << materialCount << ")";
    }
  }

  void parseGeomObjectMesh(Mesh& mesh)
  {
    expectDirective("MESH");

    parseBlock({
      {"MESH_NUMVERTEX",
       std::bind(
         &AseLoader::parseGeomObjectMeshNumVertex, this, std::ref(mesh.vertices))},
      {"MESH_VERTEX_LIST",
       std::bind(
         &AseLoader::parseGeomObjectMeshVertexList, this, std::ref(mesh.vertices))},
      {"MESH_NUMFACES",
       std::bind(&AseLoader::parseGeomObjectMeshNumFaces, this, std::ref(mesh.faces))},
      {"MESH_FACE_LIST",
       std::bind(&AseLoader::parseGeomObjectMeshFaceList, this, std::ref(mesh.faces))},
      {"MESH_NUMTVERTEX",
       std::bind(&AseLoader::parseGeomObjectMeshNumTVertex, this, std::ref(mesh.uv))},
      {"MESH_TVERTLIST",
       std::bind(&AseLoader::parseGeomObjectMeshTVertexList, this, std::ref(mesh.uv))},
      {"MESH_TFACELIST",
       std::bind(&AseLoader::parseGeomObjectMeshTFaceList, this, std::ref(mesh.faces))},
    });
  }

  void parseGeomObjectMeshNumVertex(std::vector<vm::vec3f>& vertices)
  {
    expectDirective("MESH_NUMVERTEX");
    const auto vertexCount = parseSizeArgument();
    vertices.reserve(vertexCount);
  }

  void parseGeomObjectMeshVertexList(std::vector<vm::vec3f>& vertices)
  {
    expectDirective("MESH_VERTEX_LIST");

    parseBlock({
      {"MESH_VERTEX",
       std::bind(&AseLoader::parseGeomObjectMeshVertex, this, std::ref(vertices))},
    });
  }

  void parseGeomObjectMeshVertex(std::vector<vm::vec3f>& vertices)
  {
    expectDirective("MESH_VERTEX");
    expectSizeArgument(vertices.size());
    vertices.emplace_back(parseVecArgument());
  }

  void parseGeomObjectMeshNumFaces(std::vector<MeshFace>& faces)
  {
    expectDirective("MESH_NUMFACES");
    const auto faceCount = parseSizeArgument();
    faces.reserve(faceCount);
  }

  void parseGeomObjectMeshFaceList(std::vector<MeshFace>& faces)
  {
    expectDirective("MESH_FACE_LIST");

    parseBlock({
      {"MESH_FACE",
       std::bind(&AseLoader::parseGeomObjectMeshFace, this, std::ref(faces))},
    });
  }

  void parseGeomObjectMeshFace(std::vector<MeshFace>& faces)
  {
    expectDirective("MESH_FACE");
    expectSizeArgument(faces.size());

    const std::size_t line = m_tokenizer.line();

    // the colon after the face index is sometimes missing
    m_tokenizer.skipToken(AseToken::Colon);

    expectArgumentName("A");
    auto vertexIndexA = parseSizeArgument();

    expectArgumentName("B");
    auto vertexIndexB = parseSizeArgument();

    expectArgumentName("C");
    auto vertexIndexC = parseSizeArgument();

    // skip edges
    expectArgumentName("AB");
    parseSizeArgument();
    expectArgumentName("BC");
    parseSizeArgument();
    expectArgumentName("CA");
    parseSizeArgument();

    // skip other
    expectDirective("MESH_SMOOTHING");

    // this number is optional
    m_tokenizer.skipToken(AseToken::Integer);

    expectDirective("MESH_MTLID");
    m_tokenizer.nextToken(AseToken::Integer);

    faces.emplace_back(MeshFace{
      {MeshFaceVertex{vertexIndexA, 0},
       MeshFaceVertex{vertexIndexB, 0},
       MeshFaceVertex{vertexIndexC, 0}},
      line});
  }

  void parseGeomObjectMeshNumTVertex(std::vector<vm::vec2f>& uv)
  {
    expectDirective("MESH_NUMTVERTEX");
    const auto uvCount = parseSizeArgument();
    uv.reserve(uvCount);
  }

  void parseGeomObjectMeshTVertexList(std::vector<vm::vec2f>& uv)
  {
    expectDirective("MESH_TVERTLIST");

    parseBlock({
      {"MESH_TVERT",
       std::bind(&AseLoader::parseGeomObjectMeshTVertex, this, std::ref(uv))},
    });
  }

  void parseGeomObjectMeshTVertex(std::vector<vm::vec2f>& uv)
  {
    expectDirective("MESH_TVERT");
    expectSizeArgument(uv.size());
    const auto tmp = parseVecArgument();
    uv.emplace_back(tmp.x(), 1.0f - tmp.y());
  }

  void parseGeomObjectMeshTFaceList(std::vector<MeshFace>& faces)
  {
    expectDirective("MESH_TFACELIST");

    parseBlock({
      {"MESH_TFACE",
       std::bind(&AseLoader::parseGeomObjectMeshTFace, this, std::ref(faces))},
    });
  }

  void parseGeomObjectMeshTFace(std::vector<MeshFace>& faces)
  {
    expectDirective("MESH_TFACE");
    const auto token = m_tokenizer.peekToken();
    const auto index = parseSizeArgument();
    if (index >= faces.size())
    {
      throw ParserException(
        token.location(), "Invalid face index " + std::to_string(index));
    }

    for (size_t i = 0; i < 3; ++i)
    {
      faces[index].vertices[i].uvIndex = parseSizeArgument();
    }
  }


  void parseBlock(const std::map<std::string, std::function<void(void)>>& handlers)
  {
    m_tokenizer.nextToken(AseToken::OBrace);
    auto token = m_tokenizer.peekToken();
    while (token.hasType(AseToken::Directive))
    {
      if (const auto it = handlers.find(token.data()); it != handlers.end())
      {
        auto& handler = it->second;
        handler();
      }
      else
      {
        skipDirective();
      }
      token = m_tokenizer.peekToken();
    }
    m_tokenizer.nextToken(AseToken::CBrace);
  }

  void expectDirective(const std::string& name)
  {
    auto token = m_tokenizer.nextToken(AseToken::Directive);
    expect(name, token);
  }

  void skipDirective(const std::string& name)
  {
    auto token = m_tokenizer.peekToken(AseToken::Directive);
    if (token.data() == name)
    {
      m_tokenizer.nextToken();

      // skip arguments
      while (!m_tokenizer.peekToken().hasType(AseToken::OBrace | AseToken::Directive))
      {
        m_tokenizer.nextToken();
      }

      // skip block
      if (m_tokenizer.peekToken().hasType(AseToken::OBrace))
      {
        m_tokenizer.nextToken(AseToken::OBrace);
        while (!m_tokenizer.peekToken().hasType(AseToken::CBrace))
        {
          skipDirective();
        }
        m_tokenizer.nextToken(AseToken::CBrace);
      }
    }
  }

  void skipDirective()
  {
    m_tokenizer.nextToken(AseToken::Directive);

    // skip arguments
    while (!m_tokenizer.peekToken().hasType(
      AseToken::OBrace | AseToken::CBrace | AseToken::Directive))
    {
      m_tokenizer.nextToken();
    }

    // skip block
    if (m_tokenizer.peekToken().hasType(AseToken::OBrace))
    {
      m_tokenizer.nextToken(AseToken::OBrace);
      while (!m_tokenizer.peekToken().hasType(AseToken::CBrace))
      {
        skipDirective();
      }
      m_tokenizer.nextToken(AseToken::CBrace);
    }
  }

  void expectArgumentName(const std::string& expected)
  {
    const auto token = m_tokenizer.nextToken(AseToken::ArgumentName);
    const auto& actual = token.data();
    if (actual != expected)
    {
      throw ParserException{
        token.location(),
        "Expected argument name '" + expected + "', but got '" + actual + "'"};
    }
  }

  void expectSizeArgument(size_t expected)
  {
    const auto token = m_tokenizer.peekToken();
    const auto actual = parseSizeArgument();
    if (actual != expected)
    {
      throw ParserException{
        token.location(),
        "Expected value '" + std::to_string(expected) + "', but got '"
          + std::to_string(actual) + "'"};
    }
  }

  size_t parseSizeArgument()
  {
    const auto token = m_tokenizer.nextToken(AseToken::Integer);
    auto i = token.toInteger<int>();
    if (i < 0)
    {
      throw ParserException{
        token.location(), "Expected positive integer, but got '" + token.data() + "'"};
    }
    else
    {
      return static_cast<size_t>(i);
    }
  }

  vm::vec3f parseVecArgument()
  {
    return {
      m_tokenizer.nextToken(AseToken::Decimal).toFloat<float>(),
      m_tokenizer.nextToken(AseToken::Decimal).toFloat<float>(),
      m_tokenizer.nextToken(AseToken::Decimal).toFloat<float>()};
  }

  Result<EntityModelData> buildModelData(const Scene& scene) const
  {
    using Vertex = EntityModelVertex;

    const auto loadMaterial = [&](const auto& path) {
      const auto fixedPath = fixMaterialPath(path);
      return m_loadMaterial(fixedPath);
    };

    // Load the materials
    auto materials = scene.materialPaths | std::views::transform(loadMaterial)
                     | kdl::ranges::to<std::vector>();

    materials.push_back(m_loadMaterial(DefaultTexturePath));

    auto data = EntityModelData{PitchType::Normal, Orientation::Oriented};
    auto& surface = data.addSurface(m_name, 1);
    surface.setSkins(std::move(materials));

    // Count vertices and build bounds
    auto bounds = vm::bbox3f::builder();
    auto totalVertexCount = size_t(0);
    auto size = gl::MaterialIndexRangeMap::Size{};
    for (const auto& geomObject : scene.geomObjects)
    {
      const auto& mesh = geomObject.mesh;
      bounds.add(mesh.vertices.begin(), mesh.vertices.end());

      auto materialIndex = geomObject.materialIndex;
      if (materialIndex >= surface.skinCount() - 1u)
      {
        m_logger.warn() << "Invalid material index " << materialIndex;
        materialIndex = surface.skinCount() - 1u; // default material
      }

      const auto* material = surface.skin(materialIndex);

      const auto vertexCount = mesh.faces.size() * 3;
      size.inc(material, gl::PrimType::Triangles, vertexCount);
      totalVertexCount += vertexCount;
    }

    auto& frame = data.addFrame(m_name, bounds.bounds());

    // Collect vertex data
    auto builder = gl::MaterialIndexRangeMapBuilder<Vertex::Type>{totalVertexCount, size};
    for (const auto& geomObject : scene.geomObjects)
    {
      const auto& mesh = geomObject.mesh;

      const auto materialIndex = geomObject.materialIndex;
      const auto* material =
        materialIndex < surface.skinCount() ? surface.skin(materialIndex) : nullptr;

      for (const auto& face : mesh.faces)
      {
        if (!checkIndices(face, mesh))
        {
          continue;
        }

        const auto& fv0 = face.vertices[0];
        const auto& fv1 = face.vertices[1];
        const auto& fv2 = face.vertices[2];

        const auto v0 = mesh.vertices[fv0.vertexIndex];
        const auto v1 = mesh.vertices[fv1.vertexIndex];
        const auto v2 = mesh.vertices[fv2.vertexIndex];

        const auto uv0 =
          fv0.uvIndex == 0u && mesh.uv.empty() ? vm::vec2f{0, 0} : mesh.uv[fv0.uvIndex];
        const auto uv1 =
          fv1.uvIndex == 0u && mesh.uv.empty() ? vm::vec2f{0, 0} : mesh.uv[fv1.uvIndex];
        const auto uv2 =
          fv2.uvIndex == 0u && mesh.uv.empty() ? vm::vec2f{0, 0} : mesh.uv[fv2.uvIndex];

        builder.addTriangle(material, Vertex{v2, uv2}, Vertex{v1, uv1}, Vertex{v0, uv0});
      }
    }
    surface.addMesh(frame, std::move(builder.vertices()), std::move(builder.indices()));

    return data;
  }

  bool checkIndices(const MeshFace& face, const Mesh& mesh) const
  {
    for (std::size_t i = 0u; i < 3u; ++i)
    {
      const auto& faceVertex = face.vertices[i];
      if (faceVertex.vertexIndex >= mesh.vertices.size())
      {
        m_logger.warn() << "Line " << face.line << ": Vertex index "
                        << faceVertex.vertexIndex << " is out of bounds, skipping face";
        return false;
      }
      if (!mesh.uv.empty() && faceVertex.uvIndex >= mesh.uv.size())
      {
        m_logger.warn() << "Line " << face.line << ": UV index " << faceVertex.uvIndex
                        << " is out of bounds, skipping face";
        return false;
      }
    }
    return true;
  }


  std::filesystem::path fixMaterialPath(std::filesystem::path path) const
  {
    if (!path.is_absolute())
    {
      // usually the paths appear to be relative to the map file, but this will just yield
      // a valid path if we kick off the ".." parts
      while (!path.empty() && kdl::path_front(path) == "..")
      {
        path = kdl::path_pop_front(path);
      }
    }
    return path;
  }
};

} // namespace

bool canLoadAseModel(const std::filesystem::path& path)
{
  return kdl::path_has_extension(kdl::path_to_lower(path), ".ase");
}

Result<EntityModelData> loadAseModel(
  std::string name, std::string_view str, LoadMaterialFunc loadMaterial, Logger& logger)
{
  auto loader = AseLoader{std::move(name), str, loadMaterial, logger};
  return loader.load();
}

} // namespace tb::mdl
