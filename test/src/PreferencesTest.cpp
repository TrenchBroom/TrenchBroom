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
    static void testSerializedDeserializedPair(const QString& str, const PrimitiveType& value) {
        const PrimitiveType testDeserialize = maybeDeserialize<PreferenceSerializerV1, PrimitiveType>(str).value();
        const QString testSerialize = serialize<Serializer, PrimitiveType>(value);

        qDebug() << testSerialize;
        
        EXPECT_EQ(value, testDeserialize);
        EXPECT_EQ(str, testSerialize);
    }

    TEST(PreferencesTest, serializeV1Bool) {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("").has_value()));
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("-1").has_value()));

        testSerializedDeserializedPair<PreferenceSerializerV1, bool>(QStringLiteral("0"), false);
        testSerializedDeserializedPair<PreferenceSerializerV1, bool>(QStringLiteral("1"), true);
    }

    TEST(PreferencesTest, serializeV1Color) {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, Color>("0.921569 0.666667").has_value())); // must give 3 or 4 components
        
        testSerializedDeserializedPair<PreferenceSerializerV1, Color>(
            QStringLiteral("0.921569 0.666667 0.45098 0.5"), 
            Color(0.921569f, 0.666667f, 0.45098f, 0.5f));
    }

    TEST(PreferencesTest, serializeV1float) {
        testSerializedDeserializedPair<PreferenceSerializerV1, float>(QStringLiteral("0.921569"), 0.921569f);
    }

    TEST(PreferencesTest, serializeV1int) {
        testSerializedDeserializedPair<PreferenceSerializerV1, int>(QStringLiteral("0"), 0);
        testSerializedDeserializedPair<PreferenceSerializerV1, int>(QStringLiteral("-1"), -1);
        testSerializedDeserializedPair<PreferenceSerializerV1, int>(QStringLiteral("1000"), 1000);
    }

    TEST(PreferencesTest, serializeV1Path) {
        testSerializedDeserializedPair<PreferenceSerializerV1, IO::Path>(QStringLiteral("c:\\foo\\bar"), IO::Path("c:\\foo\\bar"));
        testSerializedDeserializedPair<PreferenceSerializerV1, IO::Path>(QStringLiteral(""), IO::Path());
    }

    TEST(PreferencesTest, serializeV1KeyboardShortcut) {
        testSerializedDeserializedPair<PreferenceSerializerV1, View::KeyboardShortcut>(
            QStringLiteral("87:307:306:0"), 
            View::KeyboardShortcut(QKeySequence("Alt+Shift+W", QKeySequence::PortableText)));
    }
}
