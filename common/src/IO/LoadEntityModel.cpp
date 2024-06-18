/*
 Copyright (C) 2024 Kristian Duske

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

#include "LoadEntityModel.h"

#include "Assets/EntityModel.h"
#include "Assets/Palette.h"
#include "Error.h"
#include "IO/AseParser.h"
#include "IO/AssimpParser.h"
#include "IO/Bsp29Parser.h"
#include "IO/DkmParser.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/ImageSpriteParser.h"
#include "IO/Md2Parser.h"
#include "IO/Md3Parser.h"
#include "IO/MdlParser.h"
#include "IO/MdxParser.h"
#include "IO/SprParser.h"
#include "Model/GameConfig.h"
#include "Result.h"

#include <kdl/result.h>

namespace TrenchBroom::IO
{

namespace
{

auto loadPalette(const FileSystem& fs, const Model::MaterialConfig& materialConfig)
{
  const auto& path = materialConfig.palette;
  return fs.openFile(path)
         | kdl::and_then([&](auto file) { return Assets::loadPalette(*file, path); });
}

} // namespace

Result<Assets::EntityModel> loadEntityModel(
  const FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const std::filesystem::path& path,
  const LoadMaterialFunc& loadMaterial,
  Logger& logger)
{
  return fs.openFile(path) | kdl::and_then([&](auto file) -> Result<Assets::EntityModel> {
           const auto modelName = path.filename().string();
           auto reader = file->reader().buffer();

           if (IO::MdlParser::canParse(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      auto parser = IO::MdlParser{modelName, reader, palette};
                      return parser.initializeModel(logger);
                    });
           }
           if (IO::Md2Parser::canParse(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      auto parser = IO::Md2Parser{modelName, reader, palette, fs};
                      return parser.initializeModel(logger);
                    });
           }
           if (IO::Bsp29Parser::canParse(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      auto parser = IO::Bsp29Parser{modelName, reader, palette, fs};
                      return parser.initializeModel(logger);
                    });
           }
           if (IO::SprParser::canParse(path, reader))
           {
             return loadPalette(fs, materialConfig) | kdl::and_then([&](auto palette) {
                      auto parser = IO::SprParser{modelName, reader, palette};
                      return parser.initializeModel(logger);
                    });
           }
           if (IO::Md3Parser::canParse(path, reader))
           {
             auto parser = IO::Md3Parser{modelName, reader, loadMaterial};
             return parser.initializeModel(logger);
           }
           if (IO::MdxParser::canParse(path, reader))
           {
             auto parser = IO::MdxParser{modelName, reader, fs};
             return parser.initializeModel(logger);
           }
           if (IO::DkmParser::canParse(path, reader))
           {
             auto parser = IO::DkmParser{modelName, reader, fs};
             return parser.initializeModel(logger);
           }
           if (IO::AseParser::canParse(path))
           {
             auto parser = IO::AseParser{modelName, reader.stringView(), fs};
             return parser.initializeModel(logger);
           }
           if (IO::ImageSpriteParser::canParse(path))
           {
             auto parser = IO::ImageSpriteParser{modelName, file, fs};
             return parser.initializeModel(logger);
           }
           if (IO::AssimpParser::canParse(path))
           {
             auto parser = IO::AssimpParser{path, fs};
             return parser.initializeModel(logger);
           }
           return Error{"Unknown model format: '" + path.string() + "'"};
         });
}

} // namespace TrenchBroom::IO
