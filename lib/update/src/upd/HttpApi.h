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

#pragma once

#include "kdl/reflection_decl.h"
#include "kdl/result.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace upd
{
struct Error
{
  std::string message;
  kdl_reflect_decl(Error, message);
};

struct Version
{
  int major;
  int minor;
  int patch;

  kdl_reflect_decl(Version, major, minor, patch);
};

struct Asset
{
  std::string name;
  std::string url;
  std::size_t size;

  kdl_reflect_decl(Asset, name, url, size);
};

struct Release
{
  Version version;
  bool prerelease;
  std::string name;
  std::vector<Asset> assets;

  friend bool operator<(const Release& lhs, const Release& rhs);

  kdl_reflect_decl(Release, version, prerelease, name, assets);
};

class HttpApi
{
public:
  using GetLatestReleaseCallback =
    std::function<void(kdl::result<std::optional<Release>, Error>)>;
  using DownloadAssetCallback =
    std::function<void(kdl::result<std::filesystem::path, Error>)>;
  using GetReleasesCallback =
    std::function<void(kdl::result<std::vector<Release>, Error>)>;

  virtual ~HttpApi();

  void getLatestRelease(
    Version currentVersion,
    bool includePreReleases,
    GetLatestReleaseCallback callback) const;

  virtual void downloadAsset(
    const Asset& asset,
    const std::filesystem::path& targetFolderPath,
    DownloadAssetCallback callback) const = 0;

private:
  virtual void getReleases(
    bool includePrereleases, GetReleasesCallback callback) const = 0;
};

} // namespace upd
