/*
 Copyright (C) 2010 Kristian Duske

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

#include "AseLoader.h"

#include "FileLocation.h"
#include "Logger.h"
#include "Result.h"
#include "io/ResourceUtils.h"
#include "mdl/EntityModel.h"
#include "render/MaterialIndexRangeMap.h"
#include "render/MaterialIndexRangeMapBuilder.h"
#include "render/PrimType.h"

#include "kdl/k.h"
#include "kdl/path_utils.h"
#include "kdl/string_format.h"

#include <functional>
#include <string>

namespace tb::io
{

AseTokenizer::AseTokenizer(const std::string_view str)
  : Tokenizer{str, "", 0}
{
}

const std::string AseTokenizer::WordDelims = " \t\n\r:";

Tokenizer<unsigned int>::Token AseTokenizer::emitToken()
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

AseLoader::AseLoader(
  std::string name, const std::string_view str, LoadMaterialFunc loadMaterial)
  : m_name{std::move(name)}
  , m_tokenizer{str}
  , m_loadMaterial{std::move(loadMaterial)}
{
}

bool AseLoader::canParse(const std::filesystem::path& path)
{
  return kdl::str_to_lower(path.extension().string()) == ".ase";
}

Result<mdl::EntityModelData> AseLoader::load(Logger& logger)
{
  try
  {
    auto scene = Scene{};
    parseAseFile(logger, scene);
    return buildModelData(logger, scene);
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

void AseLoader::parseAseFile(Logger& logger, Scene& scene)
{
  expectDirective("3DSMAX_ASCIIEXPORT");
  expect(AseToken::Integer, m_tokenizer.nextToken());

  skipDirective("COMMENT");

  parseScene(logger);
  parseMaterialList(logger, scene.materialPaths);

  while (!m_tokenizer.peekToken().hasType(AseToken::Eof))
  {
    auto geomObject = GeomObject{};
    parseGeomObject(logger, geomObject, scene.materialPaths);
    scene.geomObjects.emplace_back(std::move(geomObject));
  }
}

void AseLoader::parseScene(Logger& /* logger */)
{
  skipDirective("SCENE");
}

void AseLoader::parseMaterialList(
  Logger& logger, std::vector<std::filesystem::path>& paths)
{
  expectDirective("MATERIAL_LIST");

  parseBlock(
    {{"MATERIAL_COUNT",
      std::bind(
        &AseLoader::parseMaterialListMaterialCount,
        this,
        std::ref(logger),
        std::ref(paths))},
     {"MATERIAL",
      std::bind(
        &AseLoader::parseMaterialListMaterial,
        this,
        std::ref(logger),
        std::ref(paths))}});
}

void AseLoader::parseMaterialListMaterialCount(
  Logger& /* logger */, std::vector<std::filesystem::path>& paths)
{
  expectDirective("MATERIAL_COUNT");
  paths.resize(parseSizeArgument());
}

void AseLoader::parseMaterialListMaterial(
  Logger& logger, std::vector<std::filesystem::path>& paths)
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
          &AseLoader::parseMaterialListMaterialMapDiffuse,
          this,
          std::ref(logger),
          std::ref(path))},
       {"MATERIAL_NAME",
        std::bind(
          &AseLoader::parseMaterialListMaterialName,
          this,
          std::ref(logger),
          std::ref(name))}});

    if (path.empty())
    {
      logger.warn() << "Material " << index
                    << " is missing a 'BITMAP' directive, falling back to material name '"
                    << name << "'";
      path = std::filesystem::path(name);
    }
  }
  else
  {
    logger.warn() << "Material index " << index << " is out of bounds.";
    parseBlock({});
  }
}

void AseLoader::parseMaterialListMaterialName(Logger&, std::string& name)
{
  expectDirective("MATERIAL_NAME");
  const auto token = expect(AseToken::String, m_tokenizer.nextToken());
  name = token.data();
}

void AseLoader::parseMaterialListMaterialMapDiffuse(
  Logger& logger, std::filesystem::path& path)
{
  expectDirective("MAP_DIFFUSE");

  parseBlock(
    {{"BITMAP",
      std::bind(
        &AseLoader::parseMaterialListMaterialMapDiffuseBitmap,
        this,
        std::ref(logger),
        std::ref(path))}});
}

void AseLoader::parseMaterialListMaterialMapDiffuseBitmap(
  Logger& /* logger */, std::filesystem::path& path)
{
  expectDirective("BITMAP");
  const auto token = expect(AseToken::String, m_tokenizer.nextToken());
  path = kdl::parse_path(token.data(), K(replace_backslashes));
}

void AseLoader::parseGeomObject(
  Logger& logger,
  GeomObject& geomObject,
  const std::vector<std::filesystem::path>& materialPaths)
{
  expectDirective("GEOMOBJECT");

  parseBlock(
    {{"NODE_NAME",
      std::bind(
        &AseLoader::parseGeomObjectNodeName,
        this,
        std::ref(logger),
        std::ref(geomObject))},
     {"MATERIAL_REF",
      std::bind(
        &AseLoader::parseGeomObjectMaterialRef,
        this,
        std::ref(logger),
        std::ref(geomObject),
        materialPaths.size())},
     {"MESH",
      std::bind(
        &AseLoader::parseGeomObjectMesh,
        this,
        std::ref(logger),
        std::ref(geomObject.mesh))}});
}

void AseLoader::parseGeomObjectNodeName(Logger& /* logger */, GeomObject& geomObject)
{
  expectDirective("NODE_NAME");
  const auto token = expect(AseToken::String, m_tokenizer.nextToken());
  geomObject.name = token.data();
}

void AseLoader::parseGeomObjectMaterialRef(
  Logger& logger, GeomObject& geomObject, const size_t materialCount)
{
  expectDirective("MATERIAL_REF");
  const auto token = m_tokenizer.peekToken();
  geomObject.materialIndex = parseSizeArgument();
  if (geomObject.materialIndex >= materialCount)
  {
    logger.warn() << "Line " << token.line() << ": Material index "
                  << geomObject.materialIndex
                  << " is out of bounds (material count: " << materialCount << ")";
  }
}

void AseLoader::parseGeomObjectMesh(Logger& logger, Mesh& mesh)
{
  expectDirective("MESH");

  parseBlock({
    {"MESH_NUMVERTEX",
     std::bind(
       &AseLoader::parseGeomObjectMeshNumVertex,
       this,
       std::ref(logger),
       std::ref(mesh.vertices))},
    {"MESH_VERTEX_LIST",
     std::bind(
       &AseLoader::parseGeomObjectMeshVertexList,
       this,
       std::ref(logger),
       std::ref(mesh.vertices))},
    {"MESH_NUMFACES",
     std::bind(
       &AseLoader::parseGeomObjectMeshNumFaces,
       this,
       std::ref(logger),
       std::ref(mesh.faces))},
    {"MESH_FACE_LIST",
     std::bind(
       &AseLoader::parseGeomObjectMeshFaceList,
       this,
       std::ref(logger),
       std::ref(mesh.faces))},
    {"MESH_NUMTVERTEX",
     std::bind(
       &AseLoader::parseGeomObjectMeshNumTVertex,
       this,
       std::ref(logger),
       std::ref(mesh.uv))},
    {"MESH_TVERTLIST",
     std::bind(
       &AseLoader::parseGeomObjectMeshTVertexList,
       this,
       std::ref(logger),
       std::ref(mesh.uv))},
    {"MESH_TFACELIST",
     std::bind(
       &AseLoader::parseGeomObjectMeshTFaceList,
       this,
       std::ref(logger),
       std::ref(mesh.faces))},
  });
}

void AseLoader::parseGeomObjectMeshNumVertex(
  Logger& /* logger */, std::vector<vm::vec3f>& vertices)
{
  expectDirective("MESH_NUMVERTEX");
  const auto vertexCount = parseSizeArgument();
  vertices.reserve(vertexCount);
}

void AseLoader::parseGeomObjectMeshVertexList(
  Logger& logger, std::vector<vm::vec3f>& vertices)
{
  expectDirective("MESH_VERTEX_LIST");

  parseBlock({
    {"MESH_VERTEX",
     std::bind(
       &AseLoader::parseGeomObjectMeshVertex,
       this,
       std::ref(logger),
       std::ref(vertices))},
  });
}

void AseLoader::parseGeomObjectMeshVertex(
  Logger& /* logger */, std::vector<vm::vec3f>& vertices)
{
  expectDirective("MESH_VERTEX");
  expectSizeArgument(vertices.size());
  vertices.emplace_back(parseVecArgument());
}

void AseLoader::parseGeomObjectMeshNumFaces(
  Logger& /* logger */, std::vector<MeshFace>& faces)
{
  expectDirective("MESH_NUMFACES");
  const auto faceCount = parseSizeArgument();
  faces.reserve(faceCount);
}

void AseLoader::parseGeomObjectMeshFaceList(Logger& logger, std::vector<MeshFace>& faces)
{
  expectDirective("MESH_FACE_LIST");

  parseBlock({
    {"MESH_FACE",
     std::bind(
       &AseLoader::parseGeomObjectMeshFace, this, std::ref(logger), std::ref(faces))},
  });
}

void AseLoader::parseGeomObjectMeshFace(
  Logger& /* logger */, std::vector<MeshFace>& faces)
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
  expect(AseToken::Integer, m_tokenizer.nextToken());

  faces.emplace_back(MeshFace{
    {MeshFaceVertex{vertexIndexA, 0},
     MeshFaceVertex{vertexIndexB, 0},
     MeshFaceVertex{vertexIndexC, 0}},
    line});
}

void AseLoader::parseGeomObjectMeshNumTVertex(
  Logger& /* logger */, std::vector<vm::vec2f>& uv)
{
  expectDirective("MESH_NUMTVERTEX");
  const auto uvCount = parseSizeArgument();
  uv.reserve(uvCount);
}

void AseLoader::parseGeomObjectMeshTVertexList(Logger& logger, std::vector<vm::vec2f>& uv)
{
  expectDirective("MESH_TVERTLIST");

  parseBlock({
    {"MESH_TVERT",
     std::bind(
       &AseLoader::parseGeomObjectMeshTVertex, this, std::ref(logger), std::ref(uv))},
  });
}

void AseLoader::parseGeomObjectMeshTVertex(
  Logger& /* logger */, std::vector<vm::vec2f>& uv)
{
  expectDirective("MESH_TVERT");
  expectSizeArgument(uv.size());
  const auto tmp = parseVecArgument();
  uv.emplace_back(tmp.x(), 1.0f - tmp.y());
}

void AseLoader::parseGeomObjectMeshTFaceList(Logger& logger, std::vector<MeshFace>& faces)
{
  expectDirective("MESH_TFACELIST");

  parseBlock({
    {"MESH_TFACE",
     std::bind(
       &AseLoader::parseGeomObjectMeshTFace, this, std::ref(logger), std::ref(faces))},
  });
}

void AseLoader::parseGeomObjectMeshTFace(
  Logger& /* logger */, std::vector<MeshFace>& faces)
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

void AseLoader::parseBlock(
  const std::map<std::string, std::function<void(void)>>& handlers)
{
  expect(AseToken::OBrace, m_tokenizer.nextToken());
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
  expect(AseToken::CBrace, m_tokenizer.nextToken());
}

void AseLoader::expectDirective(const std::string& name)
{
  auto token = expect(AseToken::Directive, m_tokenizer.nextToken());
  expect(name, token);
}

void AseLoader::skipDirective(const std::string& name)
{
  auto token = expect(AseToken::Directive, m_tokenizer.peekToken());
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
      expect(AseToken::OBrace, m_tokenizer.nextToken());
      while (!m_tokenizer.peekToken().hasType(AseToken::CBrace))
      {
        skipDirective();
      }
      expect(AseToken::CBrace, m_tokenizer.nextToken());
    }
  }
}

void AseLoader::skipDirective()
{
  expect(AseToken::Directive, m_tokenizer.nextToken());

  // skip arguments
  while (!m_tokenizer.peekToken().hasType(
    AseToken::OBrace | AseToken::CBrace | AseToken::Directive))
  {
    m_tokenizer.nextToken();
  }

  // skip block
  if (m_tokenizer.peekToken().hasType(AseToken::OBrace))
  {
    expect(AseToken::OBrace, m_tokenizer.nextToken());
    while (!m_tokenizer.peekToken().hasType(AseToken::CBrace))
    {
      skipDirective();
    }
    expect(AseToken::CBrace, m_tokenizer.nextToken());
  }
}

void AseLoader::expectArgumentName(const std::string& expected)
{
  const auto token = expect(AseToken::ArgumentName, m_tokenizer.nextToken());
  const auto& actual = token.data();
  if (actual != expected)
  {
    throw ParserException{
      token.location(),
      "Expected argument name '" + expected + "', but got '" + actual + "'"};
  }
}

void AseLoader::expectSizeArgument(const size_t expected)
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

size_t AseLoader::parseSizeArgument()
{
  const auto token = expect(AseToken::Integer, m_tokenizer.nextToken());
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

vm::vec3f AseLoader::parseVecArgument()
{
  return {
    expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>(),
    expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>(),
    expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>()};
}

AseLoader::TokenNameMap AseLoader::tokenNames() const
{
  TokenNameMap result;
  result[AseToken::Directive] = "directive";
  result[AseToken::OBrace] = "'{'";
  result[AseToken::CBrace] = "'}'";
  result[AseToken::String] = "quoted string";
  result[AseToken::Integer] = "integer";
  result[AseToken::Decimal] = "decimal";
  result[AseToken::Keyword] = "keyword";
  result[AseToken::ArgumentName] = "argument name";
  result[AseToken::Colon] = "':'";
  result[AseToken::Eof] = "end of file";
  return result;
}

Result<mdl::EntityModelData> AseLoader::buildModelData(
  Logger& logger, const Scene& scene) const
{
  using Vertex = mdl::EntityModelVertex;

  auto data = mdl::EntityModelData{mdl::PitchType::Normal, mdl::Orientation::Oriented};
  auto& surface = data.addSurface(m_name, 1);

  // Load the materials
  auto materials = kdl::vec_transform(scene.materialPaths, [&](const auto& path) {
    const auto fixedPath = fixMaterialPath(path);
    return m_loadMaterial(fixedPath);
  });

  materials.push_back(m_loadMaterial(DefaultTexturePath));
  surface.setSkins(std::move(materials));

  // Count vertices and build bounds
  auto bounds = vm::bbox3f::builder();
  auto totalVertexCount = size_t(0);
  auto size = render::MaterialIndexRangeMap::Size{};
  for (const auto& geomObject : scene.geomObjects)
  {
    const auto& mesh = geomObject.mesh;
    bounds.add(mesh.vertices.begin(), mesh.vertices.end());

    auto materialIndex = geomObject.materialIndex;
    if (materialIndex >= surface.skinCount() - 1u)
    {
      logger.warn() << "Invalid material index " << materialIndex;
      materialIndex = surface.skinCount() - 1u; // default material
    }

    const auto* material = surface.skin(materialIndex);

    const auto vertexCount = mesh.faces.size() * 3;
    size.inc(material, render::PrimType::Triangles, vertexCount);
    totalVertexCount += vertexCount;
  }

  auto& frame = data.addFrame(m_name, bounds.bounds());

  // Collect vertex data
  auto builder =
    render::MaterialIndexRangeMapBuilder<Vertex::Type>{totalVertexCount, size};
  for (const auto& geomObject : scene.geomObjects)
  {
    const auto& mesh = geomObject.mesh;

    const auto materialIndex = geomObject.materialIndex;
    const auto* material =
      materialIndex < surface.skinCount() ? surface.skin(materialIndex) : nullptr;

    for (const auto& face : mesh.faces)
    {
      if (!checkIndices(logger, face, mesh))
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

bool AseLoader::checkIndices(Logger& logger, const MeshFace& face, const Mesh& mesh) const
{
  for (std::size_t i = 0u; i < 3u; ++i)
  {
    const auto& faceVertex = face.vertices[i];
    if (faceVertex.vertexIndex >= mesh.vertices.size())
    {
      logger.warn() << "Line " << face.line << ": Vertex index " << faceVertex.vertexIndex
                    << " is out of bounds, skipping face";
      return false;
    }
    if (!mesh.uv.empty() && faceVertex.uvIndex >= mesh.uv.size())
    {
      logger.warn() << "Line " << face.line << ": UV index " << faceVertex.uvIndex
                    << " is out of bounds, skipping face";
      return false;
    }
  }
  return true;
}

std::filesystem::path AseLoader::fixMaterialPath(std::filesystem::path path) const
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

} // namespace tb::io
