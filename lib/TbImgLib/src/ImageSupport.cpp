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

#include "FreeImage.h"
#include "InitFreeImage.h"

#include "kd/contracts.h"
#include "kd/path_utils.h"
#include "kd/ranges/to.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

namespace tb::img
{

namespace
{

std::vector<std::string> getSupportedExtensions()
{
  auto result = std::vector<std::string>{};

  const auto count = FreeImage_GetFIFCount();
  contract_assert(count >= 0);

  for (int i = 0; i < count; ++i)
  {
    const auto format = static_cast<FREE_IMAGE_FORMAT>(i);
    if (FreeImage_IsPluginEnabled(format))
    {
      const auto extensionListStr =
        kdl::str_to_lower(std::string{FreeImage_GetFIFExtensionList(format)});
      kdl::vec_append(
        result,
        kdl::str_split(extensionListStr, ",")
          | std::views::transform([](const auto& extension) { return "." + extension; })
          | kdl::ranges::to<std::vector>());
    }
  }

  return result;
}

const std::vector<std::string>& cachedSupportedExtensions()
{
  InitFreeImage::initialize();
  static const auto extensions = getSupportedExtensions();
  return extensions;
}

} // namespace

std::vector<std::string> supportedExtensions()
{
  return cachedSupportedExtensions();
}

bool isSupportedExtension(const std::filesystem::path& extension)
{
  return kdl::vec_contains(cachedSupportedExtensions(), kdl::path_to_lower(extension));
}

} // namespace tb::img
