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

#pragma once

#include "Color.h"
#include "EL/Expression.h"
#include "FloatType.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/CompilationConfig.h"
#include "Model/GameEngineConfig.h"
#include "Model/Tag.h"

#include "kdl/reflection_decl.h"

#include "vm/bbox.h"

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
struct MapFormatConfig
{
  std::string format;
  std::filesystem::path initialMap;

  kdl_reflect_decl(MapFormatConfig, format, initialMap);
};

struct PackageFormatConfig
{
  std::vector<std::string> extensions;
  std::string format;

  kdl_reflect_decl(PackageFormatConfig, extensions, format);
};

struct FileSystemConfig
{
  std::filesystem::path searchPath;
  PackageFormatConfig packageFormat;

  kdl_reflect_decl(FileSystemConfig, searchPath, packageFormat);
};

struct TextureConfig
{
  std::filesystem::path root;
  std::vector<std::string> extensions;
  std::filesystem::path palette;
  std::optional<std::string> property;
  std::filesystem::path shaderSearchPath;
  // Glob patterns used to match texture names for exclusion
  std::vector<std::string> excludes;

  kdl_reflect_decl(
    TextureConfig, root, extensions, palette, property, shaderSearchPath, excludes);
};

struct EntityConfig
{
  std::vector<std::filesystem::path> defFilePaths;
  Color defaultColor;
  std::optional<EL::Expression> scaleExpression;
  bool setDefaultProperties;

  kdl_reflect_decl(
    EntityConfig, defFilePaths, defaultColor, scaleExpression, setDefaultProperties);
};

struct FlagConfig
{
  std::string name;
  std::string description;
  int value;

  kdl_reflect_decl(FlagConfig, name, description, value);
};

struct FlagsConfig
{
  std::vector<FlagConfig> flags;

  kdl_reflect_decl(FlagsConfig, flags);

  int flagValue(const std::string& flagName) const;
  std::string flagName(size_t index) const;
  std::vector<std::string> flagNames(int mask = ~0) const;
};

struct FaceAttribsConfig
{
  FlagsConfig surfaceFlags;
  FlagsConfig contentFlags;
  BrushFaceAttributes defaults{BrushFaceAttributes::NoTextureName};

  kdl_reflect_decl(FaceAttribsConfig, surfaceFlags, contentFlags);
};

struct CompilationTool
{
  std::string name;
  std::optional<std::string> description;

  kdl_reflect_decl(CompilationTool, name, description);
};

struct GameConfig
{
  std::string name;
  std::filesystem::path path;
  std::filesystem::path icon;
  bool experimental;
  std::vector<MapFormatConfig> fileFormats;
  FileSystemConfig fileSystemConfig;
  TextureConfig textureConfig;
  EntityConfig entityConfig;
  FaceAttribsConfig faceAttribsConfig;
  std::vector<SmartTag> smartTags;
  std::optional<vm::bbox3> softMapBounds;
  std::vector<CompilationTool> compilationTools;

  CompilationConfig compilationConfig{};
  GameEngineConfig gameEngineConfig{};
  bool compilationConfigParseFailed{false};
  bool gameEngineConfigParseFailed{false};

  size_t maxPropertyLength{1023};

  kdl_reflect_decl(
    GameConfig,
    name,
    path,
    icon,
    experimental,
    fileFormats,
    fileSystemConfig,
    textureConfig,
    entityConfig,
    faceAttribsConfig,
    smartTags,
    softMapBounds,
    compilationTools,
    compilationConfig,
    gameEngineConfig,
    compilationConfigParseFailed,
    gameEngineConfigParseFailed,
    maxPropertyLength);

  std::filesystem::path findInitialMap(const std::string& formatName) const;
  std::filesystem::path findConfigFile(const std::filesystem::path& filePath) const;
};
} // namespace Model
} // namespace TrenchBroom
