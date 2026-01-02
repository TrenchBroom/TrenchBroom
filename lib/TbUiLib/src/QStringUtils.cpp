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

#include "ui/QStringUtils.h"

#include <QByteArray>
#include <QStringEncoder>

#include "Macros.h"
#include "mdl/MapTextEncoding.h"

namespace tb::ui
{

namespace
{

QStringConverter::Encoding codecForEncoding(const mdl::MapTextEncoding encoding)
{
  switch (encoding)
  {
  case mdl::MapTextEncoding::Quake:
    // Quake uses the full 1-255 range for its bitmap font.
    // So using a "just assume UTF-8" approach would not work here.
    // See: https://github.com/TrenchBroom/TrenchBroom/issues/3122
    return QStringConverter::System;
  case mdl::MapTextEncoding::Utf8:
    return QStringConverter::Utf8;
    switchDefault();
  }
}

} // namespace

QString mapStringToUnicode(const mdl::MapTextEncoding encoding, const std::string& string)
{
  const auto codec = codecForEncoding(encoding);
  auto decode = QStringDecoder{codec};
  return decode(QByteArray::fromStdString(string));
}

std::string mapStringFromUnicode(
  const mdl::MapTextEncoding encoding, const QString& string)
{
  const auto codec = codecForEncoding(encoding);
  auto encode = QStringEncoder{codec};

  return QByteArray{encode(string)}.toStdString();
}

} // namespace tb::ui
