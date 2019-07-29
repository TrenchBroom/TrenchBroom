/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <gtest/gtest.h>

#include <QFile>
#include <QTextStream>

#include "PreferenceManager.h"

namespace TrenchBroom {
    static QString getValue(const std::map<QString, std::map<QString, QString>>& map,
                            const QString& section,
                            const QString& key) {
        return map.at(section).at(key);
    }

    TEST(PreferencesTest, parseV1) {
        QFile file("fixture/test/preferences-v1.ini");
        ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

        QTextStream in(&file);
        std::map<QString, std::map<QString, QString>> parsed = parseINI(&in);

        // This is not a complete test but just some that are potentially problematic for ini parsers
        EXPECT_EQ("108.000000", getValue(parsed, "Controls/Camera", "Field of vision"));
        EXPECT_EQ("82:0:0:0", getValue(parsed, "Controls/Camera", "Move down"));
        EXPECT_EQ("1.500000", getValue(parsed, "Texture Browser", "Icon size"));
        EXPECT_EQ("/home/ericwa/Quake 3 Arena", getValue(parsed, "Games/Quake 3", "Path"));
        EXPECT_EQ("/home/ericwa/foo=bar", getValue(parsed, "Games/Generic", "Path"));
    }
}
