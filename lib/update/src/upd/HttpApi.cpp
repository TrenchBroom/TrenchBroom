/*
 Copyright (C) 2025 Kristian Duske

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

#include "HttpApi.h"

#include "kdl/reflection_impl.h"

namespace upd
{

kdl_reflect_impl(Error);
kdl_reflect_impl(Version);
kdl_reflect_impl(Asset);
kdl_reflect_impl(Release);

bool operator<(const Release& lhs, const Release& rhs)
{
  return lhs.version < rhs.version;
}

HttpApi::~HttpApi() = default;

void HttpApi::getLatestRelease(
  Version currentVersion_,
  const bool includePreReleases,
  GetLatestReleaseCallback callback) const
{
  auto getReleasesCallback = [currentVersion = currentVersion_,
                              getLatestReleaseCallback = std::move(callback)](
                               kdl::result<std::vector<Release>, Error> releases_) {
    getLatestReleaseCallback(
      releases_
      | kdl::transform([&](std::vector<Release> releases) -> std::optional<Release> {
          if (auto iLatestRelease = std::ranges::max_element(releases);
              iLatestRelease != releases.end())
          {
            if (iLatestRelease->version > currentVersion)
            {
              return *iLatestRelease;
            }
          }
          return std::nullopt;
        }));
  };

  getReleases(includePreReleases, std::move(getReleasesCallback));
}

} // namespace upd
