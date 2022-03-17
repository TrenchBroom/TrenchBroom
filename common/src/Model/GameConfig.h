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
#include "IO/Path.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/CompilationConfig.h"
#include "Model/GameEngineConfig.h"
#include "Model/Tag.h"

#include <vecmath/bbox.h>

#include <kdl/reflection_decl.h>

#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom {
namespace Model {
struct MapFormatConfig {
  std::string format;
  IO::Path initialMap;

  kdl_reflect_decl(MapFormatConfig, format, initialMap);
};

struct PackageFormatConfig {
  std::vector<std::string> extensions;
  std::string format;

  kdl_reflect_decl(PackageFormatConfig, extensions, format);
};

struct FileSystemConfig {
  IO::Path searchPath;
  PackageFormatConfig packageFormat;

  kdl_reflect_decl(FileSystemConfig, searchPath, packageFormat);
};

struct TextureFilePackageConfig {
  PackageFormatConfig fileFormat;

  kdl_reflect_decl(TextureFilePackageConfig, fileFormat);
};

struct TextureDirectoryPackageConfig {
  IO::Path rootDirectory;

  kdl_reflect_decl(TextureDirectoryPackageConfig, rootDirectory);
};

using TexturePackageConfig = std::variant<TextureFilePackageConfig, TextureDirectoryPackageConfig>;
std::ostream& operator<<(std::ostream& str, const TexturePackageConfig& config);

IO::Path getRootDirectory(const TexturePackageConfig& texturePackageConfig);

struct TextureConfig {
  TexturePackageConfig package;
  PackageFormatConfig format;
  IO::Path palette;
  std::string property;
  IO::Path shaderSearchPath;
  std::vector<std::string> excludes; // Glob patterns used to match texture names for exclusion

  kdl_reflect_decl(TextureConfig, package, format, palette, property, shaderSearchPath, excludes);
};

struct EntityConfig {
  std::vector<IO::Path> defFilePaths;
  std::vector<std::string> modelFormats;
  Color defaultColor;
  std::optional<EL::Expression> scaleExpression;

  kdl_reflect_decl(EntityConfig, defFilePaths, modelFormats, defaultColor, scaleExpression);
};

struct FlagConfig {
  std::string name;
  std::string description;
  int value;

  kdl_reflect_decl(FlagConfig, name, description, value);
};

struct FlagsConfig {
  std::vector<FlagConfig> flags;

  kdl_reflect_decl(FlagsConfig, flags);

  int flagValue(const std::string& flagName) const;
  std::string flagName(size_t index) const;
  std::vector<std::string> flagNames(int mask = ~0) const;
};

struct FaceAttribsConfig {
  FlagsConfig surfaceFlags;
  FlagsConfig contentFlags;

  kdl_reflect_decl(FaceAttribsConfig, surfaceFlags, contentFlags);

  BrushFaceAttributes defaults{BrushFaceAttributes::NoTextureName};
};

struct CompilationTool {
  std::string name;
  std::optional<std::string> description;

  kdl_reflect_decl(CompilationTool, name, description);
};

struct GameConfig {
  std::string name;
  IO::Path path;
  IO::Path icon;
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
    GameConfig, name, path, icon, experimental, fileFormats, fileSystemConfig, textureConfig,
    entityConfig, faceAttribsConfig, smartTags, softMapBounds, compilationTools, compilationConfig,
    gameEngineConfig, compilationConfigParseFailed, gameEngineConfigParseFailed, maxPropertyLength);

  IO::Path findInitialMap(const std::string& formatName) const;
  IO::Path findConfigFile(const IO::Path& filePath) const;
};
} // namespace Model
} // namespace TrenchBroom
