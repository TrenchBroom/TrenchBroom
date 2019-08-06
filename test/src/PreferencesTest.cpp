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
#include <QString>

#include <optional-lite/optional.hpp>

#include "PreferenceManager.h"
#include "QtPrettyPrinters.h"

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

    /**
     * Helper template so we don't need to use out parameters in the tests
     */
    template <class Serializer, class PrimitiveType>
    static nonstd::optional<PrimitiveType> maybeDeserialize(const QString& string) {
        Serializer s;
        PrimitiveType result;
        if (s.readFromString(string, &result)) {
            return { result };
        }
        return {};
    }

    template <class Serializer, class PrimitiveType>
    static QString serialize(const PrimitiveType& value) {
        Serializer s;
        QString result;
        QTextStream stream(&result);

        s.writeToString(stream, value);

        return result;
    }

    template <class Serializer, class PrimitiveType>
    static void testSerialize(const QString& str, const PrimitiveType& value) {
        const auto testDeserializeOption = maybeDeserialize<Serializer, PrimitiveType>(str);
        const QString testSerialize = serialize<Serializer, PrimitiveType>(value);
        
        ASSERT_TRUE(testDeserializeOption.has_value());

        EXPECT_EQ(value, testDeserializeOption.value());
        EXPECT_EQ(str, testSerialize);
    }

    TEST(PreferencesTest, serializeV1Bool) {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("").has_value()));
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("-1").has_value()));

        testSerialize<PreferenceSerializerV1, bool>("0", false);
        testSerialize<PreferenceSerializerV1, bool>("1", true);
    }

    TEST(PreferencesTest, serializeV1Color) {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, Color>("0.921569 0.666667").has_value())); // must give 3 or 4 components
        
        testSerialize<PreferenceSerializerV1, Color>(
            "0.921569 0.666667 0.45098 0.5", 
            Color(0.921569f, 0.666667f, 0.45098f, 0.5f));
    }

    TEST(PreferencesTest, serializeV1float) {
        testSerialize<PreferenceSerializerV1, float>("0.921569", 0.921569f);
    }

    TEST(PreferencesTest, serializeV1int) {
        testSerialize<PreferenceSerializerV1, int>("0", 0);
        testSerialize<PreferenceSerializerV1, int>("-1", -1);
        testSerialize<PreferenceSerializerV1, int>("1000", 1000);
    }

    TEST(PreferencesTest, serializeV1Path) {
#ifdef _WIN32
        testSerialize<PreferenceSerializerV1, IO::Path>("c:\\foo\\bar", IO::Path("c:\\foo\\bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>("c:\\foo\\bar", IO::Path("c:/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>("\\home\\foo\\bar", IO::Path("/home/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>("\\home\\foo\\bar", IO::Path("\\home\\foo\\bar"));
#else
        testSerialize<PreferenceSerializerV1, IO::Path>("c:/foo/bar", IO::Path("c:\\foo\\bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>("c:/foo/bar", IO::Path("c:/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>("/home/foo/bar", IO::Path("/home/foo/bar"));
        // FIXME: Is this what we want or is it a bug in Path?
        testSerialize<PreferenceSerializerV1, IO::Path>("home/foo/bar", IO::Path("\\home\\foo\\bar"));
#endif
        testSerialize<PreferenceSerializerV1, IO::Path>("", IO::Path());
    }

    TEST(PreferencesTest, serializeV1KeyboardShortcut) {
        // These come from wxWidgets TrenchBroom 2019.6, on Windows
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("87:307:306:0",   View::KeyboardShortcut("Alt+Shift+W"));
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("87:307:0:0",     View::KeyboardShortcut("Alt+W"));
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("87:308:307:0",   View::KeyboardShortcut("Ctrl+Alt+W"));
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("87:306:0:0",     View::KeyboardShortcut("Shift+W"));
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("77:308:0:0",     View::KeyboardShortcut("Ctrl+M"));
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("65:308:307:306", View::KeyboardShortcut("Ctrl+Alt+Shift+A"));
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("348:306:0:0",    View::KeyboardShortcut("Shift+F9"));

        // From macOS
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("80:308:0:0",     View::KeyboardShortcut("Ctrl+P")); // "Ctrl" in Qt = Command in macOS
        testSerialize<PreferenceSerializerV1, View::KeyboardShortcut>("80:307:0:0",     View::KeyboardShortcut("Alt+P")); // "Alt" in Qt = Alt in macOS
    }

    TEST(PreferencesTest, serializeV2KeyboardShortcut) {
        testSerialize<PreferenceSerializerV2, View::KeyboardShortcut>("Alt+Shift+W",    View::KeyboardShortcut("Alt+Shift+W"));
        testSerialize<PreferenceSerializerV2, View::KeyboardShortcut>("Meta+W",         View::KeyboardShortcut("Meta+W")); // "Meta" in Qt = Control in macOS
    }
}
