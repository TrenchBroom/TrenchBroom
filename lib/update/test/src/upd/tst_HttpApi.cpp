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

#include "upd/HttpApi.h"

#include "Catch2.h"

namespace upd
{

namespace
{
struct MockHttpApi : public HttpApi
{
  void downloadAsset(
    const Asset&,
    const std::filesystem::path&,
    DownloadAssetCallback callback) const override
  {
    callback(mockDownloadAssetResult);
  }

  void getReleases(const bool, GetReleasesCallback callback) const override
  {
    callback(mockGetReleasesResult);
  }

  kdl::result<std::filesystem::path, Error> mockDownloadAssetResult =
    Error{"Not implemented"};

  kdl::result<std::vector<Release>, Error> mockGetReleasesResult =
    Error{"Not implemented"};
};
} // namespace

TEST_CASE("HttpApi")
{
  SECTION("getLatestRelease")
  {
    using T = std::tuple<
      kdl::result<std::vector<Release>, Error>,
      Version,
      kdl::result<std::optional<Release>, Error>>;

    // clang-format off
    const auto 
    [getReleasesResult,      currentVersion,   expectedReleaseResult] = GENERATE(values<T>({
    {Error{"some error"},    Version{1, 0, 0}, Error{"some error"}},
    {std::vector<Release>{}, Version{1, 0, 0}, std::nullopt},
    {std::vector<Release>{
      Release{Version{2, 0, 0}, false, "v2.0.0", {}},
      Release{Version{1, 0, 0}, false, "v1.0.0", {}},
      Release{Version{1, 2, 0}, false, "v1.2.0", {}},
      Release{Version{1, 0, 1}, false, "v1.0.1", {}},
    },                       Version{2, 0, 0}, std::nullopt},
    {std::vector<Release>{
      Release{Version{2, 0, 0}, false, "v2.0.0", {}},
      Release{Version{1, 0, 0}, false, "v1.0.0", {}},
      Release{Version{1, 2, 0}, false, "v1.2.0", {}},
      Release{Version{1, 0, 1}, false, "v1.0.1", {}},
    },                       Version{1, 0, 0}, Release{Version{2, 0, 0}, false, "v2.0.0", {}}},
    {std::vector<Release>{
      Release{Version{2, 0, 0}, false, "v2.0.0", {}},
      Release{Version{1, 0, 0}, false, "v1.0.0", {}},
      Release{Version{1, 2, 0}, false, "v1.2.0", {}},
      Release{Version{1, 0, 1}, false, "v1.0.1", {}},
    },                       Version{1, 1, 0}, Release{Version{2, 0, 0}, false, "v2.0.0", {}}},
    }));
    // clang-format on

    CAPTURE(getReleasesResult, currentVersion);

    auto api = MockHttpApi{};
    api.mockGetReleasesResult = getReleasesResult;

    api.getLatestRelease(
      currentVersion,
      false,
      [&](kdl::result<std::optional<Release>, Error> actualReleaseResult) {
        CHECK(actualReleaseResult == expectedReleaseResult);
      });
  }
}
} // namespace upd
