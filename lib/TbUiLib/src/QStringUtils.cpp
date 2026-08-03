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

#include "base/Macros.h"
#include "mdl/MapTextEncoding.h"

#include <algorithm>
#include <array>

namespace tb::ui
{

namespace
{

// Quake uses the full 1-255 range for its bitmap font, selecting a glyph directly by byte
// value. So using a "just assume UTF-8" approach would not work here. See:
// https://github.com/TrenchBroom/TrenchBroom/issues/3122
//
// We map every byte 0-255 to the matching Unicode code point (and back) losslessly and
// deterministically, rather than using Qt's System codec, which follows the process's
// ANSI code page. On Windows that's now forced to UTF-8 by our application manifest (see
// Utf8CodePage.manifest.in), so System no longer round-trips the raw high-bit glyph bytes
// used by Quake's bitmap font. See:
// https://github.com/TrenchBroom/TrenchBroom/issues/5380
//
// Bytes 0x80-0x9F are the one range where a plain identity mapping (as used by
// ISO-8859-1) falls short: they're the C1 control code range, with no printable glyph,
// but Quake map text in that range was commonly authored on Windows assuming the
// Windows-1252 code page, which assigns those bytes printable characters instead (smart
// quotes, dashes, "Ž", etc). We reproduce that mapping here so such text still displays
// as intended, while keeping the mapping bijective over all 256 byte values so it still
// round-trips losslessly. See:
// https://github.com/TrenchBroom/TrenchBroom/issues/5380#issuecomment-5184717099
constexpr auto kWindows1252HighBytes = std::array<char16_t, 32>{
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, // 0x80-0x87
  0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F, // 0x88-0x8F
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, // 0x90-0x97
  0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178, // 0x98-0x9F
};

QChar quakeByteToUnicode(const unsigned char byte)
{
  return byte >= 0x80 && byte <= 0x9F ? QChar{kWindows1252HighBytes[byte - 0x80u]}
                                      : QChar{char16_t(byte)};
}

char quakeUnicodeToByte(const QChar ch)
{
  const auto code = ch.unicode();

  if (const auto it = std::ranges::find(kWindows1252HighBytes, code);
      it != kWindows1252HighBytes.end())
  {
    return char(0x80u + std::distance(kWindows1252HighBytes.begin(), it));
  }
  return code <= 0xFF ? char(code) : '?';
}

QString mapQuakeStringToUnicode(const std::string& string)
{
  auto result = QString{};
  result.reserve(int(string.size()));
  for (const auto c : string)
  {
    result.append(quakeByteToUnicode(static_cast<unsigned char>(c)));
  }
  return result;
}

std::string mapQuakeStringFromUnicode(const QString& string)
{
  auto result = std::string{};
  result.reserve(size_t(string.size()));
  for (const auto ch : string)
  {
    result.push_back(quakeUnicodeToByte(ch));
  }
  return result;
}

} // namespace

QString mapStringToUnicode(const mdl::MapTextEncoding encoding, const std::string& string)
{
  switch (encoding)
  {
  case mdl::MapTextEncoding::Quake:
    return mapQuakeStringToUnicode(string);
  case mdl::MapTextEncoding::Utf8: {
    auto decode = QStringDecoder{QStringConverter::Utf8};
    return decode(QByteArray::fromStdString(string));
  }
    switchDefault();
  }
}

std::string mapStringFromUnicode(
  const mdl::MapTextEncoding encoding, const QString& string)
{
  switch (encoding)
  {
  case mdl::MapTextEncoding::Quake:
    return mapQuakeStringFromUnicode(string);
  case mdl::MapTextEncoding::Utf8: {
    auto encode = QStringEncoder{QStringConverter::Utf8};
    return QByteArray{encode(string)}.toStdString();
  }
    switchDefault();
  }
}

} // namespace tb::ui
