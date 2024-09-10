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

#include "GameConfigParser.h"

#include "EL/EvaluationContext.h"
#include "EL/EvaluationTrace.h"
#include "EL/ExpressionNode.h"
#include "EL/Value.h"
#include "Exceptions.h"
#include "FloatType.h"
#include "Model/GameConfig.h"
#include "Model/Tag.h"
#include "Model/TagAttribute.h"
#include "Model/TagMatcher.h"

#include "kdl/vector_utils.h"

#include "vm/vec_io.h"

#include <fmt/format.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{
namespace
{
std::string prependDot(const std::string& extension)
{
  return !extension.empty() && extension.front() != '.' ? "." + extension : extension;
}

std::vector<std::string> prependDot(const std::vector<std::string>& extensions)
{
  return kdl::vec_transform(
    extensions, [](const auto& extension) { return prependDot(extension); });
}

void checkVersion(const EL::Value& version, const EL::EvaluationTrace& trace)
{
  const auto validVsns = std::vector<EL::IntegerType>{9};
  const auto isValidVersion =
    version.convertibleTo(EL::ValueType::Number)
    && std::find(validVsns.begin(), validVsns.end(), version.integerValue())
         != validVsns.end();

  if (!isValidVersion)
  {
    throw ParserException{
      *trace.getLocation(version),
      fmt::format(
        "Unsupported game configuration version {}; valid versions are: {}",
        version.convertTo(EL::ValueType::String).stringValue(),
        kdl::str_join(validVsns, ", "))};
  }
}

std::vector<Model::CompilationTool> parseCompilationTools(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  if (value == EL::Value::Null)
  {
    return {};
  }

  expectType(value, trace, EL::typeForName("Array"));

  auto result = std::vector<Model::CompilationTool>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    expectStructure(
      value[i],
      trace,
      R"([
        {'name': 'String'},
        {'description': 'String'}
      ])");

    if (value[i]["description"] != EL::Value::Null)
    {
      result.push_back(Model::CompilationTool{
        value[i]["name"].stringValue(),
        value[i]["description"].stringValue(),
      });
    }
    else
    {
      result.push_back(Model::CompilationTool{
        value[i]["name"].stringValue(),
        std::nullopt,
      });
    }
  }

  return result;
}

std::optional<vm::bbox3> parseSoftMapBounds(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  if (value == EL::Value::Null)
  {
    return std::nullopt;
  }

  const auto bounds = parseSoftMapBoundsString(value.stringValue());
  if (!bounds.has_value())
  {
    // If a bounds is provided in the config, it must be valid
    throw ParserException{
      *trace.getLocation(value),
      fmt::format("Can't parse soft map bounds '{}'", value.asString())};
  }
  return bounds;
}

std::vector<Model::TagAttribute> parseTagAttributes(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  auto result = std::vector<Model::TagAttribute>{};
  if (value == EL::Value::Null)
  {
    return result;
  }

  result.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i)
  {
    const auto& entry = value[i];
    const auto& name = entry.stringValue();

    if (name == Model::TagAttributes::Transparency.name())
    {
      result.push_back(Model::TagAttributes::Transparency);
    }
    else
    {
      throw ParserException{
        *trace.getLocation(value), fmt::format("Unexpected tag attribute '{}'", name)};
    }
  }

  return result;
}

int parseFlagValue(const EL::Value& value, const Model::FlagsConfig& flags)
{
  const auto flagSet = value.asStringSet();
  int flagValue = 0;
  for (const std::string& currentName : flagSet)
  {
    const auto currentValue = flags.flagValue(currentName);
    flagValue = flagValue | currentValue;
  }
  return flagValue;
}

void checkTagName(
  const EL::Value& nameValue,
  const EL::EvaluationTrace& trace,
  const std::vector<Model::SmartTag>& tags)
{
  const auto& name = nameValue.stringValue();
  for (const auto& tag : tags)
  {
    if (tag.name() == name)
    {
      throw ParserException{
        *trace.getLocation(nameValue), fmt::format("Duplicate tag '{}'", name)};
    }
  }
}

void parseSurfaceParmTag(
  std::string name,
  const EL::Value& value,
  const EL::EvaluationTrace& trace,
  std::vector<Model::SmartTag>& result)
{
  auto attribs = parseTagAttributes(value["attribs"], trace);
  auto matcher = std::unique_ptr<Model::SurfaceParmTagMatcher>{};
  if (value["pattern"].type() == EL::ValueType::String)
  {
    matcher =
      std::make_unique<Model::SurfaceParmTagMatcher>(value["pattern"].stringValue());
  }
  else if (value["pattern"].type() == EL::ValueType::Array)
  {
    matcher =
      std::make_unique<Model::SurfaceParmTagMatcher>(value["pattern"].asStringSet());
  }
  else
  {
    // Generate the type exception specifying Array as the
    // expected type, since String is really a legacy type for
    // backward compatibility.
    expectMapEntry(value, trace, "pattern", EL::ValueType::Array);
  }
  result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
}

void parseFaceTags(
  const EL::Value& value,
  const EL::EvaluationTrace& trace,
  const Model::FaceAttribsConfig& faceAttribsConfig,
  std::vector<Model::SmartTag>& result)
{
  if (value == EL::Value::Null)
  {
    return;
  }

  for (size_t i = 0; i < value.length(); ++i)
  {
    const auto& entry = value[i];

    expectStructure(
      entry,
      trace,
      R"([
        {'name': 'String', 'match': 'String'},
        {'attribs': 'Array', 'pattern': 'String', 'flags': 'Array' }
      ])");
    checkTagName(entry["name"], trace, result);

    const auto match = entry["match"].stringValue();
    if (match == "material")
    {
      expectMapEntry(entry, trace, "pattern", EL::ValueType::String);
      result.emplace_back(
        entry["name"].stringValue(),
        parseTagAttributes(entry["attribs"], trace),
        std::make_unique<Model::MaterialNameTagMatcher>(entry["pattern"].stringValue()));
    }
    else if (match == "surfaceparm")
    {
      parseSurfaceParmTag(entry["name"].stringValue(), entry, trace, result);
    }
    else if (match == "contentflag")
    {
      expectMapEntry(entry, trace, "flags", EL::ValueType::Array);
      result.emplace_back(
        entry["name"].stringValue(),
        parseTagAttributes(entry["attribs"], trace),
        std::make_unique<Model::ContentFlagsTagMatcher>(
          parseFlagValue(entry["flags"], faceAttribsConfig.contentFlags)));
    }
    else if (match == "surfaceflag")
    {
      expectMapEntry(entry, trace, "flags", EL::ValueType::Array);
      result.emplace_back(
        entry["name"].stringValue(),
        parseTagAttributes(entry["attribs"], trace),
        std::make_unique<Model::SurfaceFlagsTagMatcher>(
          parseFlagValue(entry["flags"], faceAttribsConfig.surfaceFlags)));
    }
    else
    {
      throw ParserException{
        *trace.getLocation(entry),
        fmt::format("Unexpected smart tag match type '{}'", match)};
    }
  }
}

void parseBrushTags(
  const EL::Value& value,
  const EL::EvaluationTrace& trace,
  std::vector<Model::SmartTag>& result)
{
  if (value == EL::Value::Null)
  {
    return;
  }

  for (size_t i = 0; i < value.length(); ++i)
  {
    const auto entry = value[i];

    expectStructure(
      entry,
      trace,
      R"([
        {'name': 'String', 'match': 'String'},
        {'attribs': 'Array', 'pattern': 'String', 'material': 'String' }
      ])");
    checkTagName(entry["name"], trace, result);

    const auto match = entry["match"].stringValue();
    if (match == "classname")
    {
      result.emplace_back(
        entry["name"].stringValue(),
        parseTagAttributes(entry["attribs"], trace),
        std::make_unique<Model::EntityClassNameTagMatcher>(
          entry["pattern"].stringValue(), entry["material"].stringValue()));
    }
    else
    {
      throw ParserException{
        *trace.getLocation(entry),
        fmt::format("Unexpected smart tag match type '{}'", match)};
    }
  }
}

std::vector<Model::SmartTag> parseTags(
  const EL::Value& value,
  const EL::EvaluationTrace& trace,
  const Model::FaceAttribsConfig& faceAttribsConfig)
{
  auto result = std::vector<Model::SmartTag>{};
  if (value == EL::Value::Null)
  {
    return result;
  }

  expectStructure(
    value,
    trace,
    R"([
      {},
      {'brush': 'Array', 'brushface': 'Array'}
    ])");

  parseBrushTags(value["brush"], trace, result);
  parseFaceTags(value["brushface"], trace, faceAttribsConfig, result);
  return result;
}

Model::BrushFaceAttributes parseFaceAttribsDefaults(
  const EL::Value& value,
  const EL::EvaluationTrace& trace,
  const Model::FlagsConfig& surfaceFlags,
  const Model::FlagsConfig& contentFlags)
{
  auto defaults = Model::BrushFaceAttributes{Model::BrushFaceAttributes::NoMaterialName};
  if (value == EL::Value::Null)
  {
    return defaults;
  }

  expectStructure(
    value,
    trace,
    R"([
      {},
      {'materialName': 'String', 'offset': 'Array', 'scale': 'Array', 'rotation': 'Number', 'surfaceContents': 'Array', 'surfaceFlags': 'Array', 'surfaceValue': 'Number', 'color': 'String'}
    ])");

  if (value["materialName"] != EL::Value::Null)
  {
    defaults = Model::BrushFaceAttributes{value["materialName"].stringValue()};
  }
  if (value["offset"] != EL::Value::Null && value["offset"].length() == 2)
  {
    const auto offset = value["offset"];
    defaults.setOffset(vm::vec2f(offset[0].numberValue(), offset[1].numberValue()));
  }
  if (value["scale"] != EL::Value::Null && value["scale"].length() == 2)
  {
    const auto scale = value["scale"];
    defaults.setScale(vm::vec2f(scale[0].numberValue(), scale[1].numberValue()));
  }
  if (value["rotation"] != EL::Value::Null)
  {
    defaults.setRotation(float(value["rotation"].numberValue()));
  }
  if (value["surfaceContents"] != EL::Value::Null)
  {
    int defaultSurfaceContents = 0;
    for (size_t i = 0; i < value["surfaceContents"].length(); ++i)
    {
      auto name = value["surfaceContents"][i].stringValue();
      defaultSurfaceContents = defaultSurfaceContents | contentFlags.flagValue(name);
    }
    defaults.setSurfaceContents(defaultSurfaceContents);
  }
  if (value["surfaceFlags"] != EL::Value::Null)
  {
    int defaultSurfaceFlags = 0;
    for (size_t i = 0; i < value["surfaceFlags"].length(); ++i)
    {
      auto name = value["surfaceFlags"][i].stringValue();
      defaultSurfaceFlags = defaultSurfaceFlags | surfaceFlags.flagValue(name);
    }
    defaults.setSurfaceFlags(defaultSurfaceFlags);
  }
  if (value["surfaceValue"] != EL::Value::Null)
  {
    defaults.setSurfaceValue(float(value["surfaceValue"].numberValue()));
  }
  if (value["color"] != EL::Value::Null)
  {
    defaults.setColor(Color::parse(value["color"].stringValue()).value_or(Color{}));
  }

  return defaults;
}

void parseFlag(
  const EL::Value& value,
  const EL::EvaluationTrace& trace,
  const size_t index,
  std::vector<Model::FlagConfig>& flags)
{
  if (value["unused"].booleanValue())
  {
    expectStructure(
      value,
      trace,
      R"([
        {},
        {'name': 'String', 'description': 'String', 'unused': 'Boolean'}
      ])");
  }
  else
  {
    expectStructure(
      value,
      trace,
      R"([
      {'name': 'String'},
      {'description': 'String', 'unused': 'Boolean'}
      ])");

    flags.push_back(Model::FlagConfig{
      value["name"].stringValue(),
      value["description"].stringValue(),
      1 << index,
    });
  }
}

Model::FlagsConfig parseFlagsConfig(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  using Model::GameConfig;

  if (value == EL::Value::Null)
  {
    return {};
  }

  auto flags = std::vector<Model::FlagConfig>{};
  flags.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    parseFlag(value[i], trace, i, flags);
  }

  return Model::FlagsConfig{flags};
}

Model::FaceAttribsConfig parseFaceAttribsConfig(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  if (value == EL::Value::Null)
  {
    return Model::FaceAttribsConfig{
      {},
      {},
      Model::BrushFaceAttributes{Model::BrushFaceAttributes::NoMaterialName},
    };
  }

  expectStructure(
    value,
    trace,
    R"([
      {'surfaceflags': 'Array', 'contentflags': 'Array'},
      {'defaults': 'Map'}
    ])");

  auto surfaceFlags = parseFlagsConfig(value["surfaceflags"], trace);
  auto contentFlags = parseFlagsConfig(value["contentflags"], trace);
  auto defaults =
    parseFaceAttribsDefaults(value["defaults"], trace, surfaceFlags, contentFlags);

  return Model::FaceAttribsConfig{
    std::move(surfaceFlags),
    std::move(contentFlags),
    defaults,
  };
}

Model::EntityConfig parseEntityConfig(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    R"([
      {'definitions': 'Array', 'defaultcolor': 'String'},
      // scale is an expression
      {'modelformats': 'Array', 'scale': '*', 'setDefaultProperties': 'Boolean'}
    ])");

  return Model::EntityConfig{
    kdl::vec_transform(
      value["definitions"].asStringList(),
      [](const auto& str) { return std::filesystem::path{str}; }),
    Color::parse(value["defaultcolor"].stringValue()).value_or(Color{}),
    trace.getExpression(value["scale"]),
    value["setDefaultProperties"].booleanValue(),
  };
}

Model::PackageFormatConfig parsePackageFormatConfig(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectMapEntry(value, trace, "format", EL::typeForName("String"));
  const auto formatValue = value["format"];
  expectType(formatValue, trace, EL::typeForName("String"));

  if (value["extension"] != EL::Value::Null)
  {
    expectType(value["extension"], trace, EL::typeForName("String"));

    return Model::PackageFormatConfig{
      {prependDot(value["extension"].stringValue())},
      formatValue.stringValue(),
    };
  }
  else if (value["extensions"] != EL::Value::Null)
  {
    expectType(value["extensions"], trace, EL::typeForName("Array"));

    return Model::PackageFormatConfig{
      prependDot(value["extensions"].asStringList()),
      formatValue.stringValue(),
    };
  }
  throw ParserException{
    *trace.getLocation(value),
    "Expected map entry 'extension' of type 'String' or 'extensions' of type 'Array'"};
}

std::vector<std::string> parseMaterialExtensions(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  if (value["extensions"] != EL::Value::Null)
  {
    // version 8
    return prependDot(value["extensions"].asStringList());
  }
  // version 7
  return parsePackageFormatConfig(value["format"], trace).extensions;
}

Model::MaterialConfig parseMaterialConfig(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    R"([
      {'root': 'String'},
      {'extensions': 'String', 'format': 'Map', 'attribute': 'String', 'palette': 'String', 'shaderSearchPath': 'String', 'excludes': 'Array'}
    ])");

  return Model::MaterialConfig{
    std::filesystem::path{value["root"].stringValue()},
    parseMaterialExtensions(value, trace),
    std::filesystem::path{value["palette"].stringValue()},
    value["attribute"] != EL::Value::Null
      ? std::optional{value["attribute"].stringValue()}
      : std::nullopt,
    std::filesystem::path{value["shaderSearchPath"].stringValue()},
    value["excludes"].asStringList(),
  };
}

Model::FileSystemConfig parseFileSystemConfig(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectStructure(
    value,
    trace,
    R"([
      {'searchpath': 'String', 'packageformat': 'Map'},
      {}
    ])");

  return Model::FileSystemConfig{
    std::filesystem::path{value["searchpath"].stringValue()},
    parsePackageFormatConfig(value["packageformat"], trace),
  };
}

std::vector<Model::MapFormatConfig> parseMapFormatConfigs(
  const EL::Value& value, const EL::EvaluationTrace& trace)
{
  expectType(value, trace, EL::typeForName("Array"));

  auto result = std::vector<Model::MapFormatConfig>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    expectStructure(
      value[i],
      trace,
      R"([
        {'format': 'String'},
        {'initialmap': 'String'}
      ])");

    result.push_back(Model::MapFormatConfig{
      value[i]["format"].stringValue(),
      std::filesystem::path{value[i]["initialmap"].stringValue()},
    });
  }

  return result;
}

} // namespace

GameConfigParser::GameConfigParser(
  std::string_view str, const std::filesystem::path& path)
  : ConfigParserBase{std::move(str), path}
  , m_version{0}
{
}

Model::GameConfig GameConfigParser::parse()
{
  using Model::GameConfig;

  const auto evaluationContext = EL::EvaluationContext{};
  auto trace = EL::EvaluationTrace{};

  const auto root = parseConfigFile().evaluate(evaluationContext, trace);
  expectType(root, trace, EL::ValueType::Map);

  const auto& version = root["version"];
  checkVersion(version, trace);
  m_version = version.integerValue();

  expectStructure(
    root,
    trace,
    R"([
      {'version': 'Number', 'name': 'String', 'fileformats': 'Array', 'filesystem': 'Map', 'materials': 'Map', 'entities': 'Map'},
      {'icon': 'String', 'experimental': 'Boolean', 'faceattribs': 'Map', 'tags': 'Map', 'softMapBounds': 'String'}
    ])");

  auto mapFormatConfigs = parseMapFormatConfigs(root["fileformats"], trace);
  auto fileSystemConfig = parseFileSystemConfig(root["filesystem"], trace);
  auto materialConfig = parseMaterialConfig(root["materials"], trace);
  auto entityConfig = parseEntityConfig(root["entities"], trace);
  auto faceAttribsConfig = parseFaceAttribsConfig(root["faceattribs"], trace);
  auto tags = parseTags(root["tags"], trace, faceAttribsConfig);
  auto softMapBounds = parseSoftMapBounds(root["softMapBounds"], trace);
  auto compilationTools = parseCompilationTools(root["compilationTools"], trace);

  return {
    root["name"].stringValue(),
    m_path,
    std::filesystem::path{root["icon"].stringValue()},
    root["experimental"].booleanValue(),
    std::move(mapFormatConfigs),
    std::move(fileSystemConfig),
    std::move(materialConfig),
    std::move(entityConfig),
    std::move(faceAttribsConfig),
    std::move(tags),
    std::move(softMapBounds),
    std::move(compilationTools),
  };
}

std::optional<vm::bbox3> parseSoftMapBoundsString(const std::string& string)
{
  if (const auto v = vm::parse<double, 6u>(string))
  {
    return vm::bbox3{{(*v)[0], (*v)[1], (*v)[2]}, {(*v)[3], (*v)[4], (*v)[5]}};
  }
  return std::nullopt;
}

std::string serializeSoftMapBoundsString(const vm::bbox3& bounds)
{
  auto result = std::stringstream{};
  result << bounds.min << " " << bounds.max;
  return result.str();
}
} // namespace TrenchBroom::IO
