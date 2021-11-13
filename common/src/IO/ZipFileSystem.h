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

#include "IO/ImageFileSystem.h"

#include <memory>

#include <miniz/miniz.h>

namespace TrenchBroom {
namespace IO {
class Path;

class ZipFileSystem : public ImageFileSystem {
private:
  mz_zip_archive m_archive;

private:
  class ZipCompressedFile : public FileEntry {
  private:
    ZipFileSystem* m_owner;
    mz_uint m_fileIndex;

  public:
    ZipCompressedFile(ZipFileSystem* owner, mz_uint fileIndex);

  private:
    std::shared_ptr<File> doOpen() const override;
  };
  friend class ZipCompressedFile;

public:
  explicit ZipFileSystem(const Path& path);
  ZipFileSystem(std::shared_ptr<FileSystem> next, const Path& path);
  ~ZipFileSystem() override;

private:
  void doReadDirectory() override;

private:
  std::string filename(mz_uint fileIndex);
};
} // namespace IO
} // namespace TrenchBroom
