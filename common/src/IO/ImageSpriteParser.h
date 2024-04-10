/*
 Copyright (C) 2021 Kristian Duske

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

#include "IO/EntityModelParser.h"

#include <filesystem>
#include <memory>
#include <string>

namespace TrenchBroom::IO
{
class File;
class FileSystem;

class ImageSpriteParser : public EntityModelParser
{
private:
  std::string m_name;
  std::shared_ptr<File> m_file;
  const FileSystem& m_fs;

public:
  ImageSpriteParser(std::string name, std::shared_ptr<File> file, const FileSystem& fs);

  static bool canParse(const std::filesystem::path& path);

  std::unique_ptr<Assets::EntityModel> initializeModel(Logger& logger) override;
  void loadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;
};
} // namespace TrenchBroom::IO
