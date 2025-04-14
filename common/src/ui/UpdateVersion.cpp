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

#include "UpdateVersion.h"

#include <QRegularExpression>

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"

#include <algorithm>
#include <ostream>

namespace tb::ui
{
namespace
{

std::optional<int> getCapture(const QRegularExpressionMatch& match, const int index)
{
  if (!match.hasCaptured(index))
  {
    return std::nullopt;
  }

  auto ok = false;
  const auto value = match.captured(index).toInt(&ok);
  if (!ok)
  {
    return std::nullopt;
  }

  return value;
}

} // namespace

kdl_reflect_impl(SemanticVersion);

std::strong_ordering operator<=>(const SemanticVersion& lhs, const SemanticVersion& rhs)
{
  if (const auto cmp = lhs.major <=> rhs.major; cmp != 0)
  {
    return cmp;
  }
  if (const auto cmp = lhs.minor <=> rhs.minor; cmp != 0)
  {
    return cmp;
  }
  if (const auto cmp = lhs.patch <=> rhs.patch; cmp != 0)
  {
    return cmp;
  }
  if (lhs.rc && rhs.rc)
  {
    return *lhs.rc <=> *rhs.rc;
  }
  return lhs.rc   ? std::strong_ordering::less
         : rhs.rc ? std::strong_ordering::greater
                  : std::strong_ordering::equal;
}

bool operator==(const SemanticVersion& lhs, const SemanticVersion& rhs)
{
  return lhs <=> rhs == 0;
}

kdl_reflect_impl(TemporalVersion);

std::strong_ordering operator<=>(const TemporalVersion& lhs, const TemporalVersion& rhs)
{
  if (const auto cmp = lhs.year <=> rhs.year; cmp != 0)
  {
    return cmp;
  }
  if (const auto cmp = lhs.no <=> rhs.no; cmp != 0)
  {
    return cmp;
  }
  if (lhs.rc && rhs.rc)
  {
    return *lhs.rc <=> *rhs.rc;
  }
  return lhs.rc   ? std::strong_ordering::less
         : rhs.rc ? std::strong_ordering::greater
                  : std::strong_ordering::equal;
}

bool operator==(const TemporalVersion& lhs, const TemporalVersion& rhs)
{
  return lhs <=> rhs == 0;
}

std::ostream& operator<<(std::ostream& lhs, const UpdateVersion& rhs)
{
  std::visit([&lhs](const auto& version) { lhs << version; }, rhs);
  return lhs;
}

std::strong_ordering operator<=>(const UpdateVersion& lhs, const UpdateVersion& rhs)
{
  return std::visit(
    kdl::overload(
      [](const SemanticVersion& l, const SemanticVersion& r) -> std::strong_ordering {
        return l <=> r;
      },
      [](const TemporalVersion& l, const TemporalVersion& r) -> std::strong_ordering {
        return l <=> r;
      },
      [](const SemanticVersion&, const TemporalVersion&) -> std::strong_ordering {
        return std::strong_ordering::less;
      },
      [](const TemporalVersion&, const SemanticVersion&) -> std::strong_ordering {
        return std::strong_ordering::greater;
      }),
    lhs,
    rhs);
}

bool operator==(const UpdateVersion& lhs, const UpdateVersion& rhs)
{
  return lhs <=> rhs == 0;
}

std::optional<UpdateVersion> parseUpdateVersion(const QString& tag)
{
  static const auto temporalPattern =
    QRegularExpression{R"(v(\d{4})\.(\d+)(?:-RC(\d+))?)"};
  static const auto semanticPattern =
    QRegularExpression{R"(v(\d+)\.(\d+)\.(\d+)(?:-RC(\d+))?)"};

  if (const auto temporalMatch = temporalPattern.match(tag); temporalMatch.hasMatch())
  {
    const auto year = getCapture(temporalMatch, 1);
    const auto version = getCapture(temporalMatch, 2);
    const auto rc = getCapture(temporalMatch, 3);

    if (year && version && (!rc || rc))
    {
      return TemporalVersion{*year, *version, rc};
    }
  }
  else if (const auto semanticMatch = semanticPattern.match(tag);
           semanticMatch.hasMatch())
  {
    const auto major = getCapture(semanticMatch, 1);
    const auto minor = getCapture(semanticMatch, 2);
    const auto patch = getCapture(semanticMatch, 3);
    const auto rc = getCapture(semanticMatch, 4);

    if (major && minor && patch && (!rc || rc))
    {
      return SemanticVersion{*major, *minor, *patch, rc};
    }
  }

  return std::nullopt;
}

QString describeUpdateVersion(const UpdateVersion& version)
{
  return std::visit(
    kdl::overload(
      [](const SemanticVersion& semanticVersion) {
        return QString{"v%1.%2.%3%4"}
          .arg(semanticVersion.major)
          .arg(semanticVersion.minor)
          .arg(semanticVersion.patch)
          .arg(semanticVersion.rc ? QString{"-RC%1"}.arg(*semanticVersion.rc) : "");
      },
      [](const TemporalVersion& temporalVersion) {
        return QString{"v%1.%2%3"}
          .arg(temporalVersion.year)
          .arg(temporalVersion.no)
          .arg(temporalVersion.rc ? QString{"-RC%1"}.arg(*temporalVersion.rc) : "");
      }),
    version);
}

namespace
{
auto buildAssetPattern()
{
#if defined(_WIN32)
  return QRegularExpression{R"(TrenchBroom-Win64-AMD64-v\d{4}\.\d+-Release.zip)"};
#elif defined(__APPLE__)
#if defined(__arm64__)
  return QRegularExpression{R"(TrenchBroom-macOS-arm64-v\d{4}\.\d+-Release.zip)"};
#else
  return QRegularExpression{R"(TrenchBroom-macOS-x86_64-v\d{4}\.\d+-Release.zip)"};
#endif
#else
  return QRegularExpression{R"(TrenchBroom-Linux-x86_64-v\d{4}\.\d+-Release.zip)"};
#endif
}
} // namespace

std::optional<upd::Asset> chooseAsset(const QList<upd::Asset>& assets)
{
  static const auto pattern = buildAssetPattern();
  const auto iAsset = std::ranges::find_if(
    assets, [&](const auto& asset) { return pattern.match(asset.name).hasMatch(); });

  return iAsset != assets.end() ? std::optional{*iAsset} : std::nullopt;
}

} // namespace tb::ui