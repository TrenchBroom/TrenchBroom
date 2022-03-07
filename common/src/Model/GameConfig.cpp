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

#include "GameConfig.h"

#include "Ensure.h"
#include "IO/DiskFileSystem.h"

#include <kdl/optional_io.h>
#include <kdl/overload.h>
#include <kdl/string_utils.h>

#include <vecmath/bbox_io.h>
#include <vecmath/vec_io.h>

#include <cassert>
#include <ostream>
#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
bool operator==(const MapFormatConfig& lhs, const MapFormatConfig& rhs) {
  return lhs.format == rhs.format && lhs.initialMap == rhs.initialMap;
}

bool operator!=(const MapFormatConfig& lhs, const MapFormatConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const MapFormatConfig& config) {
  str << "MapFormatConfig{"
      << "format: " << config.format << ", "
      << "initialMap: " << config.initialMap << "}";
  return str;
}

bool operator==(const PackageFormatConfig& lhs, const PackageFormatConfig& rhs) {
  return lhs.extensions == rhs.extensions && lhs.format == rhs.format;
}

bool operator!=(const PackageFormatConfig& lhs, const PackageFormatConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const PackageFormatConfig& config) {
  str << "PackageFormatConfig{"
      << "extensions: [" << kdl::str_join(config.extensions) << "], "
      << "format: " << config.format << "}";
  return str;
}

bool operator==(const FileSystemConfig& lhs, const FileSystemConfig& rhs) {
  return lhs.searchPath == rhs.searchPath && lhs.packageFormat == rhs.packageFormat;
}

bool operator!=(const FileSystemConfig& lhs, const FileSystemConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const FileSystemConfig& config) {
  str << "FileSystemConfig{"
      << "searchPath: " << config.searchPath << ", "
      << "packageFormat: " << config.packageFormat << "}";
  return str;
}

bool operator==(const TextureFilePackageConfig& lhs, const TextureFilePackageConfig& rhs) {
  return lhs.fileFormat == rhs.fileFormat;
}

bool operator!=(const TextureFilePackageConfig& lhs, const TextureFilePackageConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const TextureFilePackageConfig& config) {
  str << "TextureFilePackageConfig{"
      << "fileFormat: " << config.fileFormat << "}";
  return str;
}

bool operator==(
  const TextureDirectoryPackageConfig& lhs, const TextureDirectoryPackageConfig& rhs) {
  return lhs.rootDirectory == rhs.rootDirectory;
}

bool operator!=(
  const TextureDirectoryPackageConfig& lhs, const TextureDirectoryPackageConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const TextureDirectoryPackageConfig& config) {
  str << "TextureDirectoryPackageConfig{"
      << "rootDirectory: " << config.rootDirectory << "}";
  return str;
}

std::ostream& operator<<(std::ostream& str, const TexturePackageConfig& config) {
  std::visit(
    [&](const auto& c) {
      str << c;
    },
    config);
  return str;
}

IO::Path getRootDirectory(const TexturePackageConfig& texturePackageConfig) {
  return std::visit(
    kdl::overload(
      [](const TextureFilePackageConfig&) {
        return IO::Path{};
      },
      [](const TextureDirectoryPackageConfig& directoryConfig) {
        return directoryConfig.rootDirectory;
      }),
    texturePackageConfig);
}

bool operator==(const TextureConfig& lhs, const TextureConfig& rhs) {
  return lhs.package == rhs.package && lhs.format == rhs.format && lhs.palette == rhs.palette &&
         lhs.property == rhs.property && lhs.shaderSearchPath == rhs.shaderSearchPath &&
         lhs.excludes == rhs.excludes;
}

bool operator!=(const TextureConfig& lhs, const TextureConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const TextureConfig& config) {
  str << "TextureConfig{"
      << "package: " << config.package << ", "
      << "format: " << config.format << ", "
      << "palette: " << config.palette << ", "
      << "property: " << config.property << ", "
      << "shaderSearchPath: " << config.shaderSearchPath << ", "
      << "excludes: [" << kdl::str_join(config.excludes) << "]}";
  return str;
}

bool operator==(const EntityConfig& lhs, const EntityConfig& rhs) {
  return lhs.defFilePaths == rhs.defFilePaths && lhs.modelFormats == rhs.modelFormats &&
         lhs.defaultColor == rhs.defaultColor && lhs.scaleExpression == rhs.scaleExpression;
}

bool operator!=(const EntityConfig& lhs, const EntityConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const EntityConfig& config) {
  str << "EntityConfig{"
      << "defFilePaths: [" << kdl::str_join(config.defFilePaths) << "], "
      << "modelFormats: [" << kdl::str_join(config.modelFormats) << "], "
      << "defaultColor: " << config.defaultColor << ", "
      << "scaleExpression: " << kdl::make_streamable(config.scaleExpression) << "}";
  return str;
}

bool operator==(const FlagConfig& lhs, const FlagConfig& rhs) {
  return lhs.name == rhs.name && lhs.description == rhs.description && lhs.value == rhs.value;
}

bool operator!=(const FlagConfig& lhs, const FlagConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const FlagConfig& config) {
  str << "FlagConfig{"
      << "name: " << config.name << ", "
      << "description: " << config.description << ", "
      << "value: " << config.value << "}";
  return str;
}

int FlagsConfig::flagValue(const std::string& flagName) const {
  for (size_t i = 0; i < flags.size(); ++i) {
    if (flags[i].name == flagName) {
      return flags[i].value;
    }
  }
  return 0;
}

std::string FlagsConfig::flagName(const size_t index) const {
  ensure(index < flags.size(), "index out of range");
  return flags[index].name;
}

std::vector<std::string> FlagsConfig::flagNames(const int mask) const {
  if (mask == 0) {
    return {};
  }

  std::vector<std::string> names;
  for (size_t i = 0; i < flags.size(); ++i) {
    if (mask & (1 << i)) {
      names.push_back(flags[i].name);
    }
  }
  return names;
}

bool operator==(const FlagsConfig& lhs, const FlagsConfig& rhs) {
  return lhs.flags == rhs.flags;
}

bool operator!=(const FlagsConfig& lhs, const FlagsConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const FlagsConfig& config) {
  str << "FlagsConfig{"
      << "flags: [" << kdl::str_join(config.flags) << "]}";
  return str;
}

bool operator==(const FaceAttribsConfig& lhs, const FaceAttribsConfig& rhs) {
  return lhs.surfaceFlags == rhs.surfaceFlags && lhs.contentFlags == rhs.contentFlags &&
         lhs.defaults == rhs.defaults;
}

bool operator!=(const FaceAttribsConfig& lhs, const FaceAttribsConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const FaceAttribsConfig& config) {
  str << "FaceAttribsConfig{"
      << "surfaceFlags: " << config.surfaceFlags << ", "
      << "contentFlags: " << config.contentFlags << ", "
      << "defaults: " << config.defaults << "}";
  return str;
}

bool operator==(const CompilationTool& lhs, const CompilationTool& rhs) {
  return lhs.name == rhs.name && lhs.description == rhs.description;
}

bool operator!=(const CompilationTool& lhs, const CompilationTool& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const CompilationTool& tool) {
  str << "CompilationTool{"
      << "name: " << tool.name << ", "
      << "description: " << tool.description.value_or("") << "}";
  return str;
}

IO::Path GameConfig::findInitialMap(const std::string& formatName) const {
  for (const auto& format : fileFormats) {
    if (format.format == formatName) {
      if (!format.initialMap.isEmpty()) {
        return findConfigFile(format.initialMap);
      } else {
        break;
      }
    }
  }
  return IO::Path("");
}

IO::Path GameConfig::findConfigFile(const IO::Path& filePath) const {
  return path.deleteLastComponent() + filePath;
}

bool operator==(const GameConfig& lhs, const GameConfig& rhs) {
  return lhs.name == rhs.name && lhs.path == rhs.path && lhs.icon == rhs.icon &&
         lhs.experimental == rhs.experimental && lhs.fileFormats == rhs.fileFormats &&
         lhs.fileSystemConfig == rhs.fileSystemConfig && lhs.textureConfig == rhs.textureConfig &&
         lhs.entityConfig == rhs.entityConfig && lhs.faceAttribsConfig == rhs.faceAttribsConfig &&
         lhs.smartTags == rhs.smartTags && lhs.compilationConfig == rhs.compilationConfig &&
         lhs.gameEngineConfig == rhs.gameEngineConfig &&
         lhs.maxPropertyLength == rhs.maxPropertyLength && lhs.softMapBounds == rhs.softMapBounds &&
         lhs.compilationConfigParseFailed == rhs.compilationConfigParseFailed &&
         lhs.gameEngineConfigParseFailed == rhs.gameEngineConfigParseFailed &&
         lhs.compilationTools == rhs.compilationTools;
}

bool operator!=(const GameConfig& lhs, const GameConfig& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const GameConfig& config) {
  str << "GameConfig{\n"
      << "  name: " << config.name << ",\n"
      << "  path: " << config.path << ",\n"
      << "  icon: " << config.icon << ",\n"
      << "  experimental: " << config.experimental << ",\n"
      << "  fileFormats: [" << kdl::str_join(config.fileFormats) << "],\n"
      << "  fileSystemConfig: " << config.fileSystemConfig << ",\n"
      << "  textureConfig: " << config.textureConfig << ",\n"
      << "  entityConfig: " << config.entityConfig << ",\n"
      << "  faceAttribsConfig: " << config.faceAttribsConfig << ",\n"
      << "  smartTags: [" << kdl::str_join(config.smartTags) << "],\n"
      << "  compilationConfig: " << config.compilationConfig << ",\n"
      << "  gameEngineConfig: " << config.gameEngineConfig << ",\n"
      << "  maxPropertyLength: " << config.maxPropertyLength << ",\n"
      << "  softMapBounds: " << kdl::make_streamable(config.softMapBounds) << ",\n"
      << "  compilationTools: [" << kdl::str_join(config.compilationTools) << "]}\n";
  return str;
}
} // namespace Model
} // namespace TrenchBroom
