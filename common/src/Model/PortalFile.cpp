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

#include "PortalFile.h"

#include "Exceptions.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"

#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <vecmath/forward.h>
#include <vecmath/polygon.h>
#include <vecmath/vec.h>

#include <fstream>
#include <string>

namespace TrenchBroom {
namespace Model {
PortalFile::PortalFile() = default;
PortalFile::~PortalFile() = default;

PortalFile::PortalFile(const IO::Path& path) {
  load(path);
}

bool PortalFile::canLoad(const IO::Path& path) {
  std::ifstream stream = openPathAsInputStream(path);
  return stream.is_open() && stream.good();
}

const std::vector<vm::polygon3f>& PortalFile::portals() const {
  return m_portals;
}

void PortalFile::load(const IO::Path& path) {
  std::ifstream stream = openPathAsInputStream(path);
  if (!stream.good()) {
    throw FileFormatException("Couldn't open file");
  }

  std::string line;
  int numPortals;

  // read header
  std::getline(stream, line);
  const std::string formatCode = kdl::str_trim(line); // trim off any trailing \r

  if (formatCode == "PRT1") {
    std::getline(stream, line); // number of leafs (ignored)
    std::getline(stream, line); // number of portals
    numPortals = std::stoi(line);
  } else if (formatCode == "PRT2") {
    std::getline(stream, line); // number of leafs (ignored)
    std::getline(stream, line); // number of clusters (ignored)
    std::getline(stream, line); // number of portals
    numPortals = std::stoi(line);
  } else if (formatCode == "PRT1-AM") {
    std::getline(stream, line); // number of clusters (ignored)
    std::getline(stream, line); // number of portals
    numPortals = std::stoi(line);
    std::getline(stream, line); // number of leafs (ignored)
  } else {
    throw FileFormatException("Unknown portal format: " + formatCode);
  }

  if (!stream.good()) {
    throw FileFormatException("Error reading header");
  }

  // read portals
  for (int i = 0; i < numPortals; ++i) {
    std::getline(stream, line);
    const auto components = kdl::str_split(line, "() \n\t\r");

    if (!stream.good() || components.size() < 3) {
      throw FileFormatException("Error reading portal");
    }

    std::vector<vm::vec3f> verts;
    size_t ptr = 3;
    const int numPoints = std::stoi(components.at(0));
    for (int j = 0; j < numPoints; ++j) {
      if (ptr + 2 >= components.size()) {
        throw FileFormatException("Error reading portal");
      }

      const vm::vec3f vert(
        std::stof(components.at(ptr)), std::stof(components.at(ptr + 1)),
        std::stof(components.at(ptr + 2)));
      verts.push_back(vert);
      ptr += 3;
    }

    m_portals.push_back(vm::polygon3f(verts));
  }
}
} // namespace Model
} // namespace TrenchBroom
