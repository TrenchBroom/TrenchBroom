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

#include "GameConfigParser.h"

#include "Exceptions.h"
#include "el/EvaluationContext.h"
#include "el/Value.h"
#include "io/ParserException.h"
#include "mdl/GameConfig.h"
#include "mdl/Tag.h"
#include "mdl/TagAttribute.h"
#include "mdl/TagMatcher.h"

#include "kdl/range_to_vector.h"
#include "kdl/vector_utils.h"

#include "vm/vec_io.h"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace tb::io
{
namespace
{
std::string prependDot(const std::string& extension)
{
  return !extension.empty() && extension.front() != '.' ? "." + extension : extension;
}

std::vector<std::filesystem::path> extensionsToPaths(const std::vector<std::string>& strs)
{
  return strs | std::views::transform([](const auto& str) {
           return std::filesystem::path{prependDot(str)};
         })
         | kdl::to_vector;
}

void checkVersion(const el::Value& version, const el::EvaluationContext& context)
{
  const auto validVsns = std::vector<el::IntegerType>{9};
  const auto isValidVersion =
    version.convertibleTo(el::ValueType::Number)
    && std::find(validVsns.begin(), validVsns.end(), version.integerValue())
         != validVsns.end();

  if (!isValidVersion)
  {
    throw ParserException{
      *context.location(version),
      fmt::format(
        "Unsupported game configuration version {}; valid versions are: {}",
        version.convertTo(el::ValueType::String).stringValue(),
        kdl::str_join(validVsns, ", "))};
  }
}

std::vector<mdl::CompilationTool> parseCompilationTools(
  const el::Value& value, const el::EvaluationContext& context)
{
  if (value == el::Value::Null)
  {
    return {};
  }

  expectType(value, context, el::typeForName("Array"));

  auto result = std::vector<mdl::CompilationTool>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    expectStructure(
      value.at(i),
      context,
      R"([
        {'name': 'String'},
        {'description': 'String'}
      ])");

    if (value.at(i).at("description") != el::Value::Null)
    {
      result.push_back(mdl::CompilationTool{
        value.at(i).at("name").stringValue(),
        value.at(i).at("description").stringValue(),
      });
    }
    else
    {
      result.push_back(mdl::CompilationTool{
        value.at(i).at("name").stringValue(),
        std::nullopt,
      });
    }
  }

  return result;
}

std::optional<vm::bbox3d> parseSoftMapBounds(
  const el::Value& value, const el::EvaluationContext& context)
{
  if (value == el::Value::Null)
  {
    return std::nullopt;
  }

  const auto bounds = parseSoftMapBoundsString(value.stringValue());
  if (!bounds.has_value())
  {
    // If a bounds is provided in the config, it must be valid
    throw ParserException{
      *context.location(value),
      fmt::format("Can't parse soft map bounds '{}'", value.asString())};
  }
  return bounds;
}

std::vector<mdl::TagAttribute> parseTagAttributes(
  const el::Value& value, const el::EvaluationContext& context)
{
  auto result = std::vector<mdl::TagAttribute>{};
  if (value == el::Value::Null)
  {
    return result;
  }

  result.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i)
  {
    const auto& entry = value.at(i);
    const auto& name = entry.stringValue();

    if (name == mdl::TagAttributes::Transparency.name)
    {
      result.push_back(mdl::TagAttributes::Transparency);
    }
    else
    {
      throw ParserException{
        *context.location(value), fmt::format("Unexpected tag attribute '{}'", name)};
    }
  }

  return result;
}

int parseFlagValue(const el::Value& value, const mdl::FlagsConfig& flags)
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
  const el::Value& nameValue,
  const el::EvaluationContext& context,
  const std::vector<mdl::SmartTag>& tags)
{
  const auto& name = nameValue.stringValue();
  for (const auto& tag : tags)
  {
    if (tag.name() == name)
    {
      throw ParserException{
        *context.location(nameValue), fmt::format("Duplicate tag '{}'", name)};
    }
  }
}

void parseSurfaceParmTag(
  std::string name,
  const el::Value& value,
  const el::EvaluationContext& context,
  std::vector<mdl::SmartTag>& result)
{
  auto attribs = parseTagAttributes(value.at("attribs"), context);
  auto matcher = std::unique_ptr<mdl::SurfaceParmTagMatcher>{};
  if (value.at("pattern").type() == el::ValueType::String)
  {
    matcher =
      std::make_unique<mdl::SurfaceParmTagMatcher>(value.at("pattern").stringValue());
  }
  else if (value.at("pattern").type() == el::ValueType::Array)
  {
    matcher = std::make_unique<mdl::SurfaceParmTagMatcher>(
      kdl::vector_set{value.at("pattern").asStringSet()});
  }
  else
  {
    // Generate the type exception specifying Array as the
    // expected type, since String is really a legacy type for
    // backward compatibility.
    expectMapEntry(value, context, "pattern", el::ValueType::Array);
  }
  result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
}

void parseFaceTags(
  const el::Value& value,
  const el::EvaluationContext& context,
  const mdl::FaceAttribsConfig& faceAttribsConfig,
  std::vector<mdl::SmartTag>& result)
{
  if (value == el::Value::Null)
  {
    return;
  }

  for (size_t i = 0; i < value.length(); ++i)
  {
    const auto& entry = value.at(i);

    expectStructure(
      entry,
      context,
      R"([
        {'name': 'String', 'match': 'String'},
        {'attribs': 'Array', 'pattern': 'String', 'flags': 'Array' }
      ])");
    checkTagName(entry.at("name"), context, result);

    const auto match = entry.at("match").stringValue();
    if (match == "material")
    {
      expectMapEntry(entry, context, "pattern", el::ValueType::String);
      result.emplace_back(
        entry.at("name").stringValue(),
        parseTagAttributes(entry.at("attribs"), context),
        std::make_unique<mdl::MaterialNameTagMatcher>(entry.at("pattern").stringValue()));
    }
    else if (match == "surfaceparm")
    {
      parseSurfaceParmTag(entry.at("name").stringValue(), entry, context, result);
    }
    else if (match == "contentflag")
    {
      expectMapEntry(entry, context, "flags", el::ValueType::Array);
      result.emplace_back(
        entry.at("name").stringValue(),
        parseTagAttributes(entry.at("attribs"), context),
        std::make_unique<mdl::ContentFlagsTagMatcher>(
          parseFlagValue(entry.at("flags"), faceAttribsConfig.contentFlags)));
    }
    else if (match == "surfaceflag")
    {
      expectMapEntry(entry, context, "flags", el::ValueType::Array);
      result.emplace_back(
        entry.at("name").stringValue(),
        parseTagAttributes(entry.at("attribs"), context),
        std::make_unique<mdl::SurfaceFlagsTagMatcher>(
          parseFlagValue(entry.at("flags"), faceAttribsConfig.surfaceFlags)));
    }
    else
    {
      throw ParserException{
        *context.location(entry),
        fmt::format("Unexpected smart tag match type '{}'", match)};
    }
  }
}

void parseBrushTags(
  const el::Value& value,
  const el::EvaluationContext& context,
  std::vector<mdl::SmartTag>& result)
{
  if (value == el::Value::Null)
  {
    return;
  }

  for (size_t i = 0; i < value.length(); ++i)
  {
    const auto entry = value.at(i);

    expectStructure(
      entry,
      context,
      R"([
        {'name': 'String', 'match': 'String'},
        {'attribs': 'Array', 'pattern': 'String', 'material': 'String' }
      ])");
    checkTagName(entry.at("name"), context, result);

    const auto match = entry.at("match").stringValue();
    if (match == "classname")
    {
      result.emplace_back(
        entry.at("name").stringValue(),
        parseTagAttributes(entry.at("attribs"), context),
        std::make_unique<mdl::EntityClassNameTagMatcher>(
          entry.at("pattern").stringValue(), entry.at("material").stringValue()));
    }
    else
    {
      throw ParserException{
        *context.location(entry),
        fmt::format("Unexpected smart tag match type '{}'", match)};
    }
  }
}

std::vector<mdl::SmartTag> parseTags(
  const el::Value& value,
  const el::EvaluationContext& context,
  const mdl::FaceAttribsConfig& faceAttribsConfig)
{
  auto result = std::vector<mdl::SmartTag>{};
  if (value == el::Value::Null)
  {
    return result;
  }

  expectStructure(
    value,
    context,
    R"([
      {},
      {'brush': 'Array', 'brushface': 'Array'}
    ])");

  parseBrushTags(value.at("brush"), context, result);
  parseFaceTags(value.at("brushface"), context, faceAttribsConfig, result);
  return result;
}

mdl::BrushFaceAttributes parseFaceAttribsDefaults(
  const el::Value& value,
  const el::EvaluationContext& context,
  const mdl::FlagsConfig& surfaceFlags,
  const mdl::FlagsConfig& contentFlags)
{
  auto defaults = mdl::BrushFaceAttributes{mdl::BrushFaceAttributes::NoMaterialName};
  if (value == el::Value::Null)
  {
    return defaults;
  }

  expectStructure(
    value,
    context,
    R"([
      {},
      {'materialName': 'String', 'offset': 'Array', 'scale': 'Array', 'rotation': 'Number', 'surfaceContents': 'Array', 'surfaceFlags': 'Array', 'surfaceValue': 'Number', 'color': 'String'}
    ])");

  if (value.at("materialName") != el::Value::Null)
  {
    defaults = mdl::BrushFaceAttributes{value.at("materialName").stringValue()};
  }
  if (value.at("offset") != el::Value::Null && value.at("offset").length() == 2)
  {
    const auto offset = value.at("offset");
    defaults.setOffset(vm::vec2f(offset.at(0).numberValue(), offset.at(1).numberValue()));
  }
  if (value.at("scale") != el::Value::Null && value.at("scale").length() == 2)
  {
    const auto scale = value.at("scale");
    defaults.setScale(vm::vec2f(scale.at(0).numberValue(), scale.at(1).numberValue()));
  }
  if (value.at("rotation") != el::Value::Null)
  {
    defaults.setRotation(float(value.at("rotation").numberValue()));
  }
  if (value.at("surfaceContents") != el::Value::Null)
  {
    int defaultSurfaceContents = 0;
    for (size_t i = 0; i < value.at("surfaceContents").length(); ++i)
    {
      auto name = value.at("surfaceContents").at(i).stringValue();
      defaultSurfaceContents = defaultSurfaceContents | contentFlags.flagValue(name);
    }
    defaults.setSurfaceContents(defaultSurfaceContents);
  }
  if (value.at("surfaceFlags") != el::Value::Null)
  {
    int defaultSurfaceFlags = 0;
    for (size_t i = 0; i < value.at("surfaceFlags").length(); ++i)
    {
      auto name = value.at("surfaceFlags").at(i).stringValue();
      defaultSurfaceFlags = defaultSurfaceFlags | surfaceFlags.flagValue(name);
    }
    defaults.setSurfaceFlags(defaultSurfaceFlags);
  }
  if (value.at("surfaceValue") != el::Value::Null)
  {
    defaults.setSurfaceValue(float(value.at("surfaceValue").numberValue()));
  }
  if (value.at("color") != el::Value::Null)
  {
    defaults.setColor(Color::parse(value.at("color").stringValue()).value_or(Color{}));
  }

  return defaults;
}

void parseFlag(
  const el::Value& value,
  const el::EvaluationContext& context,
  const size_t index,
  std::vector<mdl::FlagConfig>& flags)
{
  if (value.at("unused").booleanValue())
  {
    expectStructure(
      value,
      context,
      R"([
        {},
        {'name': 'String', 'description': 'String', 'unused': 'Boolean'}
      ])");
  }
  else
  {
    expectStructure(
      value,
      context,
      R"([
      {'name': 'String'},
      {'description': 'String', 'unused': 'Boolean'}
      ])");

    flags.push_back(mdl::FlagConfig{
      value.at("name").stringValue(),
      value.at("description").stringValue(),
      1 << index,
    });
  }
}

mdl::FlagsConfig parseFlagsConfig(
  const el::Value& value, const el::EvaluationContext& context)
{
  using mdl::GameConfig;

  if (value == el::Value::Null)
  {
    return {};
  }

  auto flags = std::vector<mdl::FlagConfig>{};
  flags.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    parseFlag(value.at(i), context, i, flags);
  }

  return mdl::FlagsConfig{flags};
}

mdl::FaceAttribsConfig parseFaceAttribsConfig(
  const el::Value& value, const el::EvaluationContext& context)
{
  if (value == el::Value::Null)
  {
    return mdl::FaceAttribsConfig{
      {},
      {},
      mdl::BrushFaceAttributes{mdl::BrushFaceAttributes::NoMaterialName},
    };
  }

  expectStructure(
    value,
    context,
    R"([
      {'surfaceflags': 'Array', 'contentflags': 'Array'},
      {'defaults': 'Map'}
    ])");

  auto surfaceFlags = parseFlagsConfig(value.at("surfaceflags"), context);
  auto contentFlags = parseFlagsConfig(value.at("contentflags"), context);
  auto defaults =
    parseFaceAttribsDefaults(value.at("defaults"), context, surfaceFlags, contentFlags);

  return mdl::FaceAttribsConfig{
    std::move(surfaceFlags),
    std::move(contentFlags),
    defaults,
  };
}

mdl::EntityConfig parseEntityConfig(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    R"([
      {'definitions': 'Array', 'defaultcolor': 'String'},
      // scale is an expression
      {'modelformats': 'Array', 'scale': '*', 'setDefaultProperties': 'Boolean'}
    ])");

  return mdl::EntityConfig{
    kdl::vec_transform(
      value.at("definitions").asStringList(),
      [](const auto& str) { return std::filesystem::path{str}; }),
    Color::parse(value.at("defaultcolor").stringValue()).value_or(Color{}),
    context.expression(value.at("scale")),
    value.at("setDefaultProperties").booleanValue(),
  };
}

mdl::PackageFormatConfig parsePackageFormatConfig(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectMapEntry(value, context, "format", el::typeForName("String"));
  const auto formatValue = value.at("format");
  expectType(formatValue, context, el::typeForName("String"));

  if (value.at("extension") != el::Value::Null)
  {
    expectType(value.at("extension"), context, el::typeForName("String"));

    return mdl::PackageFormatConfig{
      extensionsToPaths({value.at("extension").stringValue()}),
      formatValue.stringValue(),
    };
  }
  else if (value.at("extensions") != el::Value::Null)
  {
    expectType(value.at("extensions"), context, el::typeForName("Array"));

    return mdl::PackageFormatConfig{
      extensionsToPaths(value.at("extensions").asStringList()),
      formatValue.stringValue(),
    };
  }
  throw ParserException{
    *context.location(value),
    "Expected map entry 'extension' of type 'String' or 'extensions' of type 'Array'"};
}

std::vector<std::filesystem::path> parseMaterialExtensions(
  const el::Value& value, const el::EvaluationContext& context)
{
  if (value.at("extensions") != el::Value::Null)
  {
    // version 8
    return extensionsToPaths(value.at("extensions").asStringList());
  }
  // version 7
  return parsePackageFormatConfig(value.at("format"), context).extensions;
}

mdl::MaterialConfig parseMaterialConfig(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    R"([
      {'root': 'String'},
      {'extensions': 'String', 'format': 'Map', 'attribute': 'String', 'palette': 'String', 'shaderSearchPath': 'String', 'excludes': 'Array'}
    ])");

  return mdl::MaterialConfig{
    std::filesystem::path{value.at("root").stringValue()},
    parseMaterialExtensions(value, context),
    std::filesystem::path{value.at("palette").stringValue()},
    value.at("attribute") != el::Value::Null
      ? std::optional{value.at("attribute").stringValue()}
      : std::nullopt,
    std::filesystem::path{value.at("shaderSearchPath").stringValue()},
    value.at("excludes").asStringList(),
  };
}

mdl::FileSystemConfig parseFileSystemConfig(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectStructure(
    value,
    context,
    R"([
      {'searchpath': 'String', 'packageformat': 'Map'},
      {}
    ])");

  return mdl::FileSystemConfig{
    std::filesystem::path{value.at("searchpath").stringValue()},
    parsePackageFormatConfig(value.at("packageformat"), context),
  };
}

std::vector<mdl::MapFormatConfig> parseMapFormatConfigs(
  const el::Value& value, const el::EvaluationContext& context)
{
  expectType(value, context, el::typeForName("Array"));

  auto result = std::vector<mdl::MapFormatConfig>{};
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    expectStructure(
      value.at(i),
      context,
      R"([
        {'format': 'String'},
        {'initialmap': 'String'}
      ])");

    result.push_back(mdl::MapFormatConfig{
      value.at(i).at("format").stringValue(),
      std::filesystem::path{value.at(i).at("initialmap").stringValue()},
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

Result<mdl::GameConfig> GameConfigParser::parse()
{
  using mdl::GameConfig;

  return parseConfigFile()
         | kdl::and_then([&](const auto& expression) -> Result<mdl::GameConfig> {
             try
             {
               auto context = el::EvaluationContext{};
               const auto root = expression.evaluate(context);

               expectType(root, context, el::ValueType::Map);

               expectStructure(
                 root,
                 context,
                 R"([
               {'version': 'Number', 'name': 'String', 'fileformats': 'Array', 'filesystem': 'Map', 'materials': 'Map', 'entities': 'Map'},
               {'icon': 'String', 'experimental': 'Boolean', 'faceattribs': 'Map', 'tags': 'Map', 'softMapBounds': 'String'}
             ])");

               const auto& version = root.at("version");
               checkVersion(version, context);
               m_version = version.integerValue();

               auto mapFormatConfigs =
                 parseMapFormatConfigs(root.at("fileformats"), context);
               auto fileSystemConfig =
                 parseFileSystemConfig(root.at("filesystem"), context);
               auto materialConfig = parseMaterialConfig(root.at("materials"), context);
               auto entityConfig = parseEntityConfig(root.at("entities"), context);
               auto faceAttribsConfig =
                 parseFaceAttribsConfig(root.at("faceattribs"), context);
               auto tags = parseTags(root.at("tags"), context, faceAttribsConfig);
               auto softMapBounds = parseSoftMapBounds(root.at("softMapBounds"), context);
               auto compilationTools =
                 parseCompilationTools(root.at("compilationTools"), context);

               return GameConfig{
                 root.at("name").stringValue(),
                 m_path,
                 std::filesystem::path{root.at("icon").stringValue()},
                 root.at("experimental").booleanValue(),
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
             catch (const Exception& e)
             {
               return Error{e.what()};
             }
           });
}

std::optional<vm::bbox3d> parseSoftMapBoundsString(const std::string& string)
{
  if (const auto v = vm::parse<double, 6u>(string))
  {
    return vm::bbox3d{{(*v)[0], (*v)[1], (*v)[2]}, {(*v)[3], (*v)[4], (*v)[5]}};
  }
  return std::nullopt;
}

std::string serializeSoftMapBoundsString(const vm::bbox3d& bounds)
{
  auto result = std::stringstream{};
  result << bounds.min << " " << bounds.max;
  return result.str();
}

} // namespace tb::io
