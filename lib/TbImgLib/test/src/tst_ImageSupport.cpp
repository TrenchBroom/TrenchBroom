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

#include "img/ImageSupport.h"

#include "kd/vector_utils.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::img
{

TEST_CASE("supportedExtensions")
{
  const auto extensions = supportedExtensions();
  CHECK(kdl::vec_contains(extensions, std::string{".png"}));
  CHECK(kdl::vec_contains(extensions, std::string{".jpg"}));
}

TEST_CASE("isSupportedExtension")
{
  CHECK(isSupportedExtension(".jpg"));
  CHECK(isSupportedExtension(".jpeg"));
  CHECK(isSupportedExtension(".JPG"));
  CHECK(!isSupportedExtension("jpg"));
}

} // namespace tb::img
