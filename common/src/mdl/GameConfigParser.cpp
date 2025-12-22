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

#include "ParserException.h"
#include "el/EvaluationContext.h"
#include "el/Value.h"
#include "mdl/GameConfig.h"
#include "mdl/Tag.h"
#include "mdl/TagAttribute.h"
#include "mdl/TagMatcher.h"

#include "kd/ranges/to.h"

#include "vm/vec_io.h"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace tb::mdl
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
         | kdl::ranges::to<std::vector>();
}

void checkVersion(const el::EvaluationContext& context, const el::Value& version)
{
  const auto validVsns = std::vector<el::IntegerType>{9};
  const auto isValidVersion =
    version.convertibleTo(el::ValueType::Number)
    && std::ranges::find(validVsns, version.integerValue(context)) != validVsns.end();

  if (!isValidVersion)
  {
    throw ParserException{
      *context.location(version),
      fmt::format(
        "Unsupported game configuration version {}; valid versions are: {}",
        version.integerValue(context),
        kdl::str_join(validVsns, ", "))};
  }
}

std::vector<CompilationTool> parseCompilationTools(
  const el::EvaluationContext& context, const el::Value& value, std::string_view gameName)
{
  if (value == el::Value::Null)
  {
    return {};
  }

  return value.arrayValue(context) | std::views::transform([&](const auto& entry) {
           auto toolName = entry.at(context, "name").stringValue(context);

           const auto descriptionValue = entry.atOrDefault(context, "description");
           auto description = descriptionValue != el::Value::Null
                                ? std::optional{descriptionValue.stringValue(context)}
                                : std::nullopt;
           auto pathPreference = Preference<std::filesystem::path>{
             std::filesystem::path{"Games"} / gameName / "Tool Path" / toolName, {}};

           return CompilationTool{
             std::move(toolName),
             std::move(description),
             std::move(pathPreference),
           };
         })
         | kdl::ranges::to<std::vector>();
}

std::optional<vm::bbox3d> parseSoftMapBounds(
  const el::EvaluationContext& context, const el::Value& value)
{
  if (value == el::Value::Null)
  {
    return std::nullopt;
  }

  if (const auto bounds = parseSoftMapBoundsString(value.stringValue(context)))
  {
    return bounds;
  }

  // If a bounds is provided in the config, it must be valid
  throw ParserException{
    *context.location(value),
    fmt::format("Can't parse soft map bounds '{}'", value.asString())};
}

std::vector<TagAttribute> parseTagAttributes(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& entry) {
           const auto& name = entry.stringValue(context);
           if (name != TagAttributes::Transparency.name)
           {
             throw ParserException{
               *context.location(value),
               fmt::format("Unexpected tag attribute '{}'", name)};
           }

           return TagAttributes::Transparency;
         })
         | kdl::ranges::to<std::vector>();
}

int parseFlagValue(
  const el::EvaluationContext& context, const el::Value& value, const FlagsConfig& flags)
{
  const auto flagSet = value.asStringSet(context);
  int flagValue = 0;
  for (const std::string& currentName : flagSet)
  {
    const auto currentValue = flags.flagValue(currentName);
    flagValue = flagValue | currentValue;
  }
  return flagValue;
}

void checkTagName(
  const el::EvaluationContext& context,
  const el::Value& nameValue,
  const std::vector<SmartTag>& tags)
{
  const auto& name = nameValue.stringValue(context);
  if (std::ranges::any_of(tags, [&](const auto& tag) { return tag.name() == name; }))
  {
    throw ParserException{
      *context.location(nameValue), fmt::format("Duplicate tag '{}'", name)};
  }
}

std::unique_ptr<TagMatcher> parseFaceTagMatcher(
  const el::EvaluationContext& context,
  const el::Value& value,
  const FaceAttribsConfig& faceAttribsConfig)
{
  const auto match = value.at(context, "match").stringValue(context);
  if (match == "material")
  {
    return std::make_unique<MaterialNameTagMatcher>(
      value.at(context, "pattern").stringValue(context));
  }
  if (match == "surfaceparm")
  {
    const auto patternValue = value.at(context, "pattern");
    return patternValue.type() == el::ValueType::String
             ? std::make_unique<SurfaceParmTagMatcher>(patternValue.stringValue(context))
             : std::make_unique<SurfaceParmTagMatcher>(
                 kdl::vector_set{patternValue.asStringSet(context)});
  }
  if (match == "contentflag")
  {
    return std::make_unique<ContentFlagsTagMatcher>(parseFlagValue(
      context, value.at(context, "flags"), faceAttribsConfig.contentFlags));
  }
  if (match == "surfaceflag")
  {
    return std::make_unique<SurfaceFlagsTagMatcher>(parseFlagValue(
      context, value.at(context, "flags"), faceAttribsConfig.surfaceFlags));
  }

  throw ParserException{
    *context.location(value), fmt::format("Unexpected smart tag match type '{}'", match)};
}

SmartTag parseFaceTag(
  const el::EvaluationContext& context,
  std::string name,
  const el::Value& value,
  const FaceAttribsConfig& faceAttribsConfig)
{
  auto matcher = parseFaceTagMatcher(context, value, faceAttribsConfig);
  auto attribs = parseTagAttributes(context, value.atOrDefault(context, "attribs"));

  return {
    std::move(name),
    std::move(attribs),
    std::move(matcher),
  };
}

void parseFaceTags(
  const el::EvaluationContext& context,
  const el::Value& value,
  const FaceAttribsConfig& faceAttribsConfig,
  std::vector<SmartTag>& result)
{
  for (const auto& entry : value.arrayValue(context))
  {
    const auto nameValue = entry.at(context, "name");
    checkTagName(context, nameValue, result);

    result.push_back(
      parseFaceTag(context, nameValue.stringValue(context), entry, faceAttribsConfig));
  }
}


std::unique_ptr<TagMatcher> parseBrushTagMatcher(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto match = value.at(context, "match").stringValue(context);
  if (match == "classname")
  {
    return std::make_unique<EntityClassNameTagMatcher>(
      value.at(context, "pattern").stringValue(context),
      value.atOrDefault(context, "material").stringValue(context));
  }

  throw ParserException{
    *context.location(value), fmt::format("Unexpected smart tag match type '{}'", match)};
}

SmartTag parseBrushTag(
  const el::EvaluationContext& context, std::string name, const el::Value& value)
{
  auto matcher = parseBrushTagMatcher(context, value);
  auto attribs = parseTagAttributes(context, value.atOrDefault(context, "attribs"));

  return {
    std::move(name),
    std::move(attribs),
    std::move(matcher),
  };
}

void parseBrushTags(
  const el::EvaluationContext& context,
  const el::Value& value,
  std::vector<SmartTag>& result)
{
  for (const auto& entry : value.arrayValue(context))
  {
    const auto nameValue = entry.at(context, "name");
    checkTagName(context, nameValue, result);

    result.push_back(parseBrushTag(context, nameValue.stringValue(context), entry));
  }
}

std::vector<SmartTag> parseTags(
  const el::EvaluationContext& context,
  const el::Value& value,
  const FaceAttribsConfig& faceAttribsConfig)
{
  auto result = std::vector<SmartTag>{};
  if (value == el::Value::Null)
  {
    return result;
  }

  parseBrushTags(context, value.atOrDefault(context, "brush"), result);
  parseFaceTags(
    context, value.atOrDefault(context, "brushface"), faceAttribsConfig, result);
  return result;
}

BrushFaceAttributes parseFaceAttribsDefaults(
  const el::EvaluationContext& context,
  const el::Value& value,
  const FlagsConfig& surfaceFlags,
  const FlagsConfig& contentFlags)
{
  auto defaults = BrushFaceAttributes{BrushFaceAttributes::NoMaterialName};
  if (value == el::Value::Null)
  {
    return defaults;
  }

  if (const auto materialNameValue = value.atOrDefault(context, "materialName");
      materialNameValue != el::Value::Null)
  {
    defaults = BrushFaceAttributes{materialNameValue.stringValue(context)};
  }

  if (const auto offsetValue = value.atOrDefault(context, "offset");
      offsetValue != el::Value::Null && offsetValue.length() == 2)
  {
    defaults.setOffset(vm::vec2f{
      float(offsetValue.at(context, 0).numberValue(context)),
      float(offsetValue.at(context, 1).numberValue(context))});
  }

  if (const auto scaleValue = value.atOrDefault(context, "scale");
      scaleValue != el::Value::Null && scaleValue.length() == 2)
  {
    defaults.setScale(vm::vec2f{
      float(scaleValue.at(context, 0).numberValue(context)),
      float(scaleValue.at(context, 1).numberValue(context))});
  }

  if (const auto rotationValue = value.atOrDefault(context, "rotation");
      rotationValue != el::Value::Null)
  {
    defaults.setRotation(float(rotationValue.numberValue(context)));
  }

  if (const auto surfaceContentsValue = value.atOrDefault(context, "surfaceContents");
      surfaceContentsValue != el::Value::Null)
  {
    int defaultSurfaceContents = 0;
    for (const auto& surfaceContentValue : surfaceContentsValue.arrayValue(context))
    {
      const auto& name = surfaceContentValue.stringValue(context);
      defaultSurfaceContents = defaultSurfaceContents | contentFlags.flagValue(name);
    }
    defaults.setSurfaceContents(defaultSurfaceContents);
  }

  if (const auto surfaceFlagsValue = value.atOrDefault(context, "surfaceFlags");
      surfaceFlagsValue != el::Value::Null)
  {
    int defaultSurfaceFlags = 0;
    for (const auto& surfaceFlagValue : surfaceFlagsValue.arrayValue(context))
    {
      const auto& name = surfaceFlagValue.stringValue(context);
      defaultSurfaceFlags = defaultSurfaceFlags | surfaceFlags.flagValue(name);
    }
    defaults.setSurfaceFlags(defaultSurfaceFlags);
  }

  if (const auto surfaceValue = value.atOrDefault(context, "surfaceValue");
      surfaceValue != el::Value::Null)
  {
    defaults.setSurfaceValue(float(surfaceValue.numberValue(context)));
  }

  if (const auto colorValue = value.atOrDefault(context, "color");
      colorValue != el::Value::Null)
  {
    const auto color = Color::parse(colorValue.stringValue(context))
                       | kdl::if_error([&](const auto& e) {
                           throw ParserException{*context.location(value), e.msg};
                         })
                       | kdl::value();
    defaults.setColor(color);
  }

  return defaults;
}

void parseFlag(
  const el::EvaluationContext& context,
  const el::Value& value,
  const size_t index,
  std::vector<FlagConfig>& flags)
{
  if (!value.atOrDefault(context, "unused").booleanValue(context))
  {
    flags.push_back(FlagConfig{
      value.at(context, "name").stringValue(context),
      value.atOrDefault(context, "description").stringValue(context),
      1 << index,
    });
  }
}

FlagsConfig parseFlagsConfig(const el::EvaluationContext& context, const el::Value& value)
{
  auto flags = std::vector<FlagConfig>{};
  flags.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i)
  {
    parseFlag(context, value.at(context, i), i, flags);
  }

  return FlagsConfig{flags};
}

FaceAttribsConfig parseFaceAttribsConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  if (value == el::Value::Null)
  {
    return FaceAttribsConfig{
      {},
      {},
      BrushFaceAttributes{BrushFaceAttributes::NoMaterialName},
    };
  }

  auto surfaceFlags = parseFlagsConfig(context, value.at(context, "surfaceflags"));
  auto contentFlags = parseFlagsConfig(context, value.at(context, "contentflags"));
  auto defaults = parseFaceAttribsDefaults(
    context, value.atOrDefault(context, "defaults"), surfaceFlags, contentFlags);

  return FaceAttribsConfig{
    std::move(surfaceFlags),
    std::move(contentFlags),
    std::move(defaults),
  };
}

EntityConfig parseEntityConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  auto paths = value.at(context, "definitions").arrayValue(context)
               | std::views::transform([&](const auto& v) {
                   return std::filesystem::path{v.stringValue(context)};
                 })
               | kdl::ranges::to<std::vector>();

  const auto color = Color::parse(value.at(context, "defaultcolor").stringValue(context))
                     | kdl::if_error([&](const auto& e) {
                         throw ParserException{*context.location(value), e.msg};
                       })
                     | kdl::value();

  return EntityConfig{
    std::move(paths),
    color,
    context.expression(value.atOrDefault(context, "scale")),
    value.atOrDefault(context, "setDefaultProperties").booleanValue(context),
  };
}

PackageFormatConfig parsePackageFormatConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  const auto formatValue = value.at(context, "format");
  if (const auto extension = value.atOrDefault(context, "extension");
      extension != el::Value::Null)
  {
    return PackageFormatConfig{
      extensionsToPaths({extension.stringValue(context)}),
      formatValue.stringValue(context),
    };
  }
  return PackageFormatConfig{
    extensionsToPaths(value.at(context, "extensions").asStringList(context)),
    formatValue.stringValue(context),
  };
}

std::vector<std::filesystem::path> parseMaterialExtensions(
  const el::EvaluationContext& context, const el::Value& value)
{
  if (const auto extensions = value.atOrDefault(context, "extensions");
      extensions != el::Value::Null)
  {
    // version 8
    return extensionsToPaths(extensions.asStringList(context));
  }
  // version 7
  return parsePackageFormatConfig(context, value.atOrDefault(context, "format"))
    .extensions;
}

MaterialConfig parseMaterialConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  return MaterialConfig{
    std::filesystem::path{value.at(context, "root").stringValue(context)},
    parseMaterialExtensions(context, value),
    std::filesystem::path{value.atOrDefault(context, "palette").stringValue(context)},
    value.contains(context, "attribute")
      ? std::optional{value.at(context, "attribute").stringValue(context)}
      : std::nullopt,
    std::filesystem::path{
      value.atOrDefault(context, "shaderSearchPath").stringValue(context)},
    value.atOrDefault(context, "excludes").asStringList(context),
  };
}

FileSystemConfig parseFileSystemConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  return FileSystemConfig{
    std::filesystem::path{value.at(context, "searchpath").stringValue(context)},
    parsePackageFormatConfig(context, value.at(context, "packageformat")),
  };
}

std::vector<MapFormatConfig> parseMapFormatConfigs(
  const el::EvaluationContext& context, const el::Value& value)
{
  return value.arrayValue(context) | std::views::transform([&](const auto& entry) {
           return MapFormatConfig{
             entry.at(context, "format").stringValue(context),
             entry.atOrDefault(context, "initialmap").stringValue(context),
           };
         })
         | kdl::ranges::to<std::vector>();
}

Result<GameConfig> parseGameConfig(
  el::EvaluationContext& context,
  const el::ExpressionNode& expression,
  const std::filesystem::path& configFilePath)
{
  try
  {
    const auto root = expression.evaluate(context);

    checkVersion(context, root.at(context, "version"));

    auto gameName = root.at(context, "name").stringValue(context);

    auto mapFormatConfigs =
      parseMapFormatConfigs(context, root.at(context, "fileformats"));
    auto fileSystemConfig =
      parseFileSystemConfig(context, root.at(context, "filesystem"));
    auto materialConfig = parseMaterialConfig(context, root.at(context, "materials"));
    auto entityConfig = parseEntityConfig(context, root.at(context, "entities"));
    auto faceAttribsConfig =
      parseFaceAttribsConfig(context, root.atOrDefault(context, "faceattribs"));
    auto tags = parseTags(context, root.atOrDefault(context, "tags"), faceAttribsConfig);
    auto softMapBounds =
      parseSoftMapBounds(context, root.atOrDefault(context, "softMapBounds"));
    auto compilationTools = parseCompilationTools(
      context, root.atOrDefault(context, "compilationTools"), gameName);

    return GameConfig{
      std::move(gameName),
      configFilePath,
      std::filesystem::path{root.atOrDefault(context, "icon").stringValue(context)},
      root.atOrDefault(context, "experimental").booleanValue(context),
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
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

} // namespace

GameConfigParser::GameConfigParser(const std::string_view str, std::filesystem::path path)
  : m_elParser{el::ELParser::Mode::Strict, str}
  , m_path{std::move(path)}
{
}

Result<GameConfig> GameConfigParser::parse()
{
  return m_elParser.parse()
         | kdl::and_then([&](const auto& expression) -> Result<GameConfig> {
             return el::withEvaluationContext([&](auto& context) {
               return parseGameConfig(context, expression, m_path);
             });
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

} // namespace tb::mdl
