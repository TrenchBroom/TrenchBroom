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

#include <memory>

namespace TrenchBroom {
namespace Renderer {
class FontDescriptor;
class TextureFont;

class FontFactory {
protected:
  struct Metrics {
    size_t cellSize;
    size_t maxAscend;
    size_t lineHeight;
  };

public:
  virtual ~FontFactory();

  std::unique_ptr<TextureFont> createFont(const FontDescriptor& fontDescriptor);

private:
  virtual std::unique_ptr<TextureFont> doCreateFont(const FontDescriptor& fontDescriptor) = 0;
};
} // namespace Renderer
} // namespace TrenchBroom
