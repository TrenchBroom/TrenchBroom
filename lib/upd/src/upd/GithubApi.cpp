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

#include "GithubApi.h"

namespace upd
{

namespace detail
{
std::variant<Asset, Error> parseAsset(const QJsonValue& json)
{
  const auto nameValue = json["name"];
  const auto urlValue = json["browser_download_url"];
  const auto sizeValue = json["size"];

  if (!nameValue.isString())
  {
    return Error{"invalid asset, expected string for 'name'"};
  }

  if (!urlValue.isString())
  {
    return Error{"invalid asset, expected string for 'browser_download_url'"};
  }

  if (!sizeValue.isDouble())
  {
    return Error{"invalid asset, expected number for 'size'"};
  }

  return Asset{
    nameValue.toString(),
    urlValue.toString(),
    static_cast<size_t>(sizeValue.toDouble()),
  };
}

std::variant<QList<Asset>, Error> parseAssets(const QJsonValue& json)
{
  if (!json.isArray())
  {
    return Error{"invalid assets type, expected array"};
  }

  auto result = QList<Asset>{};
  for (const auto& assetJson : json.toArray())
  {
    const auto asset = parseAsset(assetJson);
    if (const auto* error = std::get_if<Error>(&asset))
    {
      return *error;
    }

    result += std::get<Asset>(asset);
  }
  return result;
}

} // namespace detail

HttpOperation* downloadAsset(
  const HttpClient& httpClient,
  const Asset& asset,
  HttpClient::DownloadCallback downloadCallback,
  HttpClient::ErrorCallback errorCallback)
{
  return httpClient.download(
    asset.url, std::move(downloadCallback), std::move(errorCallback));
}

} // namespace upd
