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

#include "mdl/ParseGameConfig.h"

#include "base/ParserException.h"
#include "el/EvaluationContext.h"
#include "el/ParseExpression.h"
#include "el/Value.h"
#include "mdl/GameConfig.h"
#include "mdl/SoftMapBounds.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/Tag.h"
#include "mdl/TagAttribute.h"
#include "mdl/TagMatcher.h"
#include "mdl/UvAttributes.h"

#include "kd/flat_set.h"
#include "kd/ranges/to.h"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
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

void checkVersion(const el::EvaluationContext&, const el::Value& version)
{
  const auto validVsns = std::vector<el::IntegerType>{9};
  const auto isValidVersion =
    version.convertibleTo(el::ValueType::Number)
    && std::ranges::find(validVsns, version.integerValue()) != validVsns.end();

  if (!isValidVersion)
  {
    throw ParserException{
      *version.location(),
      fmt::format(
        "Unsupported game configuration version {}; valid versions are: {}",
        version.integerValue(),
        kdl::str_join(validVsns, ", "))};
  }
}

std::vector<CompilationTool> parseCompilationTools(
  const el::EvaluationContext&, const el::Value& value, std::string_view gameName)
{
  if (value == el::Value::Null)
  {
    return {};
  }

  return value.arrayValue() | std::views::transform([&](const auto& entry) {
           auto toolName = entry.at("name").stringValue();

           const auto descriptionValue = entry.atOrDefault("description");
           auto description = descriptionValue != el::Value::Null
                                ? std::optional{descriptionValue.stringValue()}
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
  const el::EvaluationContext&, const el::Value& value)
{
  if (value == el::Value::Null)
  {
    return std::nullopt;
  }

  if (const auto bounds = mdl::parseSoftMapBounds(value.stringValue()))
  {
    return bounds;
  }

  // If a bounds is provided in the config, it must be valid
  throw ParserException{
    *value.location(), fmt::format("Can't parse soft map bounds '{}'", value.asString())};
}

std::vector<TagAttribute> parseTagAttributes(
  const el::EvaluationContext&, const el::Value& value)
{
  return value.arrayValue() | std::views::transform([&](const auto& entry) {
           const auto& name = entry.stringValue();
           if (name != TagAttributes::Transparency.name)
           {
             throw ParserException{
               *value.location(), fmt::format("Unexpected tag attribute '{}'", name)};
           }

           return TagAttributes::Transparency;
         })
         | kdl::ranges::to<std::vector>();
}

int parseFlagValue(
  const el::EvaluationContext&, const el::Value& value, const FlagsConfig& flags)
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
  const el::EvaluationContext&,
  const el::Value& nameValue,
  const std::vector<SmartTag>& tags)
{
  const auto& name = nameValue.stringValue();
  if (std::ranges::any_of(tags, [&](const auto& tag) { return tag.name() == name; }))
  {
    throw ParserException{*nameValue.location(), fmt::format("Duplicate tag '{}'", name)};
  }
}

std::unique_ptr<TagMatcher> parseFaceTagMatcher(
  const el::EvaluationContext& context,
  const el::Value& value,
  const FaceAttribsConfig& faceAttribsConfig)
{
  const auto match = value.at("match").stringValue();
  if (match == "material")
  {
    return std::make_unique<MaterialNameTagMatcher>(value.at("pattern").stringValue());
  }
  if (match == "surfaceparm")
  {
    const auto patternValue = value.at("pattern");
    return patternValue.type() == el::ValueType::String
             ? std::make_unique<SurfaceParmTagMatcher>(patternValue.stringValue())
             : std::make_unique<SurfaceParmTagMatcher>(
                 kdl::flat_set(patternValue.asStringSet()));
  }
  if (match == "contentflag")
  {
    return std::make_unique<ContentFlagsTagMatcher>(
      parseFlagValue(context, value.at("flags"), faceAttribsConfig.contentFlags));
  }
  if (match == "surfaceflag")
  {
    return std::make_unique<SurfaceFlagsTagMatcher>(
      parseFlagValue(context, value.at("flags"), faceAttribsConfig.surfaceFlags));
  }

  throw ParserException{
    *value.location(), fmt::format("Unexpected smart tag match type '{}'", match)};
}

SmartTag parseFaceTag(
  const el::EvaluationContext& context,
  std::string name,
  const el::Value& value,
  const FaceAttribsConfig& faceAttribsConfig)
{
  auto matcher = parseFaceTagMatcher(context, value, faceAttribsConfig);
  auto attribs = parseTagAttributes(context, value.atOrDefault("attribs"));

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
  for (const auto& entry : value.arrayValue())
  {
    const auto nameValue = entry.at("name");
    checkTagName(context, nameValue, result);

    result.push_back(
      parseFaceTag(context, nameValue.stringValue(), entry, faceAttribsConfig));
  }
}


std::unique_ptr<TagMatcher> parseBrushTagMatcher(
  const el::EvaluationContext&, const el::Value& value)
{
  const auto match = value.at("match").stringValue();
  if (match == "classname")
  {
    return std::make_unique<EntityClassNameTagMatcher>(
      value.at("pattern").stringValue(), value.atOrDefault("material").stringValue());
  }

  throw ParserException{
    *value.location(), fmt::format("Unexpected smart tag match type '{}'", match)};
}

SmartTag parseBrushTag(
  const el::EvaluationContext& context, std::string name, const el::Value& value)
{
  auto matcher = parseBrushTagMatcher(context, value);
  auto attribs = parseTagAttributes(context, value.atOrDefault("attribs"));

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
  for (const auto& entry : value.arrayValue())
  {
    const auto nameValue = entry.at("name");
    checkTagName(context, nameValue, result);

    result.push_back(parseBrushTag(context, nameValue.stringValue(), entry));
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

  parseBrushTags(context, value.atOrDefault("brush"), result);
  parseFaceTags(context, value.atOrDefault("brushface"), faceAttribsConfig, result);
  return result;
}

std::tuple<UvAttributes, SurfaceAttributes> parseFaceAttribsDefaults(
  const el::EvaluationContext&,
  const el::Value& value,
  const FlagsConfig& surfaceFlags,
  const FlagsConfig& contentFlags)
{
  auto uvAttributes = UvAttributes{};
  auto surfaceAttributes = SurfaceAttributes{};
  if (value == el::Value::Null)
  {
    return {uvAttributes, surfaceAttributes};
  }

  if (const auto offsetValue = value.atOrDefault("offset");
      offsetValue != el::Value::Null && offsetValue.length() == 2)
  {
    uvAttributes.offset = vm::vec2f{
      float(offsetValue.at(0).numberValue()), float(offsetValue.at(1).numberValue())};
  }

  if (const auto scaleValue = value.atOrDefault("scale");
      scaleValue != el::Value::Null && scaleValue.length() == 2)
  {
    uvAttributes.scale = vm::vec2f{
      float(scaleValue.at(0).numberValue()), float(scaleValue.at(1).numberValue())};
  }

  if (const auto rotationValue = value.atOrDefault("rotation");
      rotationValue != el::Value::Null)
  {
    uvAttributes.rotation = float(rotationValue.numberValue());
  }

  if (const auto surfaceContentsValue = value.atOrDefault("surfaceContents");
      surfaceContentsValue != el::Value::Null)
  {
    int defaultSurfaceContents = 0;
    for (const auto& surfaceContentValue : surfaceContentsValue.arrayValue())
    {
      const auto& name = surfaceContentValue.stringValue();
      defaultSurfaceContents = defaultSurfaceContents | contentFlags.flagValue(name);
    }
    surfaceAttributes.contents = defaultSurfaceContents;
  }

  if (const auto surfaceFlagsValue = value.atOrDefault("surfaceFlags");
      surfaceFlagsValue != el::Value::Null)
  {
    int defaultSurfaceFlags = 0;
    for (const auto& surfaceFlagValue : surfaceFlagsValue.arrayValue())
    {
      const auto& name = surfaceFlagValue.stringValue();
      defaultSurfaceFlags = defaultSurfaceFlags | surfaceFlags.flagValue(name);
    }
    surfaceAttributes.flags = defaultSurfaceFlags;
  }

  if (const auto surfaceValue = value.atOrDefault("surfaceValue");
      surfaceValue != el::Value::Null)
  {
    surfaceAttributes.value = float(surfaceValue.numberValue());
  }

  if (const auto colorValue = value.atOrDefault("color"); colorValue != el::Value::Null)
  {
    const auto color = Color::parse(colorValue.stringValue())
                       | kdl::if_error([&](const auto& e) {
                           throw ParserException{*value.location(), e.msg};
                         })
                       | kdl::value();
    surfaceAttributes.color = color;
  }

  return {uvAttributes, surfaceAttributes};
}

void parseFlag(
  const el::EvaluationContext&,
  const el::Value& value,
  const size_t index,
  std::vector<FlagConfig>& flags)
{
  if (!value.atOrDefault("unused").booleanValue())
  {
    flags.push_back(FlagConfig{
      value.at("name").stringValue(),
      value.atOrDefault("description").stringValue(),
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
    parseFlag(context, value.at(i), i, flags);
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
      {},
      {},
    };
  }

  auto surfaceFlags = parseFlagsConfig(context, value.at("surfaceflags"));
  auto contentFlags = parseFlagsConfig(context, value.at("contentflags"));
  auto [defaultUvAttributes, defaultSurfaceAttributes] = parseFaceAttribsDefaults(
    context, value.atOrDefault("defaults"), surfaceFlags, contentFlags);

  return FaceAttribsConfig{
    std::move(surfaceFlags),
    std::move(contentFlags),
    std::move(defaultUvAttributes),
    std::move(defaultSurfaceAttributes),
  };
}

EntityConfig parseEntityConfig(const el::EvaluationContext&, const el::Value& value)
{
  auto paths = value.at("definitions").arrayValue()
               | std::views::transform(
                 [&](const auto& v) { return std::filesystem::path{v.stringValue()}; })
               | kdl::ranges::to<std::vector>();

  const auto color = Color::parse(value.at("defaultcolor").stringValue())
                     | kdl::if_error([&](const auto& e) {
                         throw ParserException{*value.location(), e.msg};
                       })
                     | kdl::value();

  return EntityConfig{
    std::move(paths),
    color,
    value.atOrDefault("scale").expression(),
    value.atOrDefault("setDefaultProperties").booleanValue(),
  };
}

PackageFormatConfig parsePackageFormatConfig(
  const el::EvaluationContext&, const el::Value& value)
{
  const auto formatValue = value.at("format");
  if (const auto extension = value.atOrDefault("extension"); extension != el::Value::Null)
  {
    return PackageFormatConfig{
      extensionsToPaths({extension.stringValue()}),
      formatValue.stringValue(),
    };
  }
  return PackageFormatConfig{
    extensionsToPaths(value.at("extensions").asStringList()),
    formatValue.stringValue(),
  };
}

std::vector<std::filesystem::path> parseMaterialExtensions(
  const el::EvaluationContext& context, const el::Value& value)
{
  if (const auto extensions = value.atOrDefault("extensions");
      extensions != el::Value::Null)
  {
    // version 8
    return extensionsToPaths(extensions.asStringList());
  }
  // version 7
  return parsePackageFormatConfig(context, value.atOrDefault("format")).extensions;
}

MaterialConfig parseMaterialConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  return MaterialConfig{
    std::filesystem::path{value.at("root").stringValue()},
    parseMaterialExtensions(context, value),
    std::filesystem::path{value.atOrDefault("palette").stringValue()},
    value.contains("attribute") ? std::optional{value.at("attribute").stringValue()}
                                : std::nullopt,
    std::filesystem::path{value.atOrDefault("shaderSearchPath").stringValue()},
    value.atOrDefault("excludes").asStringList(),
  };
}

FileSystemConfig parseFileSystemConfig(
  const el::EvaluationContext& context, const el::Value& value)
{
  return FileSystemConfig{
    std::filesystem::path{value.at("searchpath").stringValue()},
    parsePackageFormatConfig(context, value.at("packageformat")),
  };
}

std::vector<MapFormatConfig> parseMapFormatConfigs(
  const el::EvaluationContext&, const el::Value& value)
{
  return value.arrayValue() | std::views::transform([&](const auto& entry) {
           return MapFormatConfig{
             entry.at("format").stringValue(),
             entry.atOrDefault("initialmap").stringValue(),
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

    checkVersion(context, root.at("version"));

    auto gameName = root.at("name").stringValue();

    auto mapFormatConfigs = parseMapFormatConfigs(context, root.at("fileformats"));
    auto fileSystemConfig = parseFileSystemConfig(context, root.at("filesystem"));
    auto materialConfig = parseMaterialConfig(context, root.at("materials"));
    auto entityConfig = parseEntityConfig(context, root.at("entities"));
    auto faceAttribsConfig =
      parseFaceAttribsConfig(context, root.atOrDefault("faceattribs"));
    auto tags = parseTags(context, root.atOrDefault("tags"), faceAttribsConfig);
    auto softMapBounds = parseSoftMapBounds(context, root.atOrDefault("softMapBounds"));
    auto compilationTools =
      parseCompilationTools(context, root.atOrDefault("compilationTools"), gameName);

    return GameConfig{
      std::move(gameName),
      configFilePath,
      std::filesystem::path{root.atOrDefault("icon").stringValue()},
      root.atOrDefault("experimental").booleanValue(),
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


Result<GameConfig> parseGameConfig(
  std::string_view str, const std::filesystem::path& path)
{
  return el::parseExpression(el::ParseMode::Strict, str)
         | kdl::and_then([&](const auto& expression) -> Result<GameConfig> {
             return el::withEvaluationContext(
               [&](auto& context) { return parseGameConfig(context, expression, path); });
           });
}

} // namespace tb::mdl
