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

#pragma once

#include "Color.h"
#include "el/Expression.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/Tag.h"

#include "kd/reflection_decl.h"

#include "vm/bbox.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{

struct MapFormatConfig
{
  std::string format;
  std::filesystem::path initialMap;

  kdl_reflect_decl(MapFormatConfig, format, initialMap);
};

struct PackageFormatConfig
{
  std::vector<std::filesystem::path> extensions;
  std::string format;

  kdl_reflect_decl(PackageFormatConfig, extensions, format);
};

struct FileSystemConfig
{
  std::filesystem::path searchPath;
  PackageFormatConfig packageFormat;

  kdl_reflect_decl(FileSystemConfig, searchPath, packageFormat);
};

struct MaterialConfig
{
  std::filesystem::path root;
  std::vector<std::filesystem::path> extensions;
  std::filesystem::path palette;
  std::optional<std::string> property;
  std::filesystem::path shaderSearchPath;
  // Glob patterns used to match material names for exclusion
  std::vector<std::string> excludes;

  kdl_reflect_decl(
    MaterialConfig, root, extensions, palette, property, shaderSearchPath, excludes);
};

struct EntityConfig
{
  std::vector<std::filesystem::path> defFilePaths;
  Color defaultColor;
  std::optional<el::ExpressionNode> scaleExpression;
  bool setDefaultProperties = false;

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
  BrushFaceAttributes defaults{BrushFaceAttributes::NoMaterialName};

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
  bool experimental = false;
  std::vector<MapFormatConfig> fileFormats;
  FileSystemConfig fileSystemConfig;
  MaterialConfig materialConfig;
  EntityConfig entityConfig;
  FaceAttribsConfig faceAttribsConfig;
  std::vector<SmartTag> smartTags;
  std::optional<vm::bbox3d> softMapBounds;
  std::vector<CompilationTool> compilationTools;
  bool forceEmptyNewMap = false;

  size_t maxPropertyLength = 1023;

  kdl_reflect_decl(
    GameConfig,
    name,
    path,
    icon,
    experimental,
    fileFormats,
    fileSystemConfig,
    materialConfig,
    entityConfig,
    faceAttribsConfig,
    smartTags,
    softMapBounds,
    compilationTools,
    maxPropertyLength);

  /** Returns a folder name to use for user configuration files. */
  std::filesystem::path configFileFolder() const;

  std::filesystem::path findInitialMap(const std::string& formatName) const;
  std::filesystem::path findConfigFile(const std::filesystem::path& filePath) const;
};

} // namespace tb::mdl
