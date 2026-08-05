/*
 Copyright (C) 2026 Kristian Duske

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

#include <QString>

#include "mdl/MapTextEncoding.h"
#include "ui/QStringUtils.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("QStringUtils")
{
  SECTION("mapStringToUnicode")
  {
    SECTION("Utf8 encoding decodes multi-byte UTF-8 sequences")
    {
      CHECK(
        mapStringToUnicode(mdl::MapTextEncoding::Utf8, "K\xC3\xB6ln")
        == QString::fromUtf8("K\xC3\xB6ln"));
    }

    SECTION("Quake encoding maps every byte 1-255 to the matching code point")
    {
      // Bytes with the high bit set (128-255) select Quake's colored bitmap font
      // glyphs and must not be interpreted as part of a UTF-8 sequence.
      // See: https://github.com/TrenchBroom/TrenchBroom/issues/3122
      //      https://github.com/TrenchBroom/TrenchBroom/issues/5380
      const auto raw = std::string{"The \xC5\xCE\xC5\xCD\xD9 flag"};
      const auto expected = QString{u"The ÅÎÅÍÙ flag"};
      CHECK(mapStringToUnicode(mdl::MapTextEncoding::Quake, raw) == expected);
    }

    SECTION("Quake encoding maps bytes 0x80-0x9F to their Windows-1252 characters")
    {
      // Bytes 0x80-0x9F have no printable glyph under a plain identity mapping (they're
      // the C1 control code range), but Quake map text in that range was commonly
      // authored assuming the Windows-1252 code page, e.g. byte 0x8E is intended as "Ž".
      // See:
      // https://github.com/TrenchBroom/TrenchBroom/issues/5380#issuecomment-5184717099
      CHECK(mapStringToUnicode(mdl::MapTextEncoding::Quake, "\x8E") == QString{u"Ž"});
      CHECK(
        mapStringToUnicode(mdl::MapTextEncoding::Quake, "\x80\x93\x94")
        == QString{u"€“”"});
    }
  }

  SECTION("mapStringFromUnicode")
  {
    SECTION("Utf8 encoding round-trips through mapStringToUnicode")
    {
      const auto raw = std::string{"K\xC3\xB6ln"};
      const auto unicode = mapStringToUnicode(mdl::MapTextEncoding::Utf8, raw);
      CHECK(mapStringFromUnicode(mdl::MapTextEncoding::Utf8, unicode) == raw);
    }

    SECTION("Quake encoding maps Windows-1252 characters back to their original byte")
    {
      CHECK(
        mapStringFromUnicode(mdl::MapTextEncoding::Quake, QString{u"Ž"})
        == std::string{"\x8E"});
    }

    SECTION("Quake encoding round-trips every byte 1-255 losslessly")
    {
      // Covers the full byte range, including 0x80-0x9F, where a plain identity mapping
      // and the Windows-1252 mapping diverge. See:
      // https://github.com/TrenchBroom/TrenchBroom/issues/5380
      auto raw = std::string{};
      for (auto byte = 1; byte <= 255; ++byte)
      {
        raw.push_back(char(byte));
      }
      const auto unicode = mapStringToUnicode(mdl::MapTextEncoding::Quake, raw);
      CHECK(mapStringFromUnicode(mdl::MapTextEncoding::Quake, unicode) == raw);
    }
  }
}

} // namespace tb::ui
