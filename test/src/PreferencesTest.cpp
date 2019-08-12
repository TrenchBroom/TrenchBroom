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
#include "Preferences.h"
#include "View/Actions.h"

namespace TrenchBroom {
    static QString getValue(const std::map<IO::Path, QString>& map, const IO::Path& key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return "";
        }
        return it->second;
    }

    TEST(PreferencesTest, migrateLocalV1Settings) {
        const std::map<IO::Path, QString> reg = readV1Settings();
        
        [[maybe_unused]]
        const auto migrated = migrateV1ToV2(reg);

        // Can't really test anything because we can't assume the test system
        // has any settings on it.
    }

    TEST(PreferencesTest, parseV1) {
        const std::map<IO::Path, QString> parsed = getINISettingsV1("fixture/test/preferences-v1.ini");

        EXPECT_EQ("108.000000", getValue(parsed, IO::Path("Controls/Camera/Field of vision")));
        EXPECT_EQ("82:0:0:0", getValue(parsed, IO::Path("Controls/Camera/Move down")));
        EXPECT_EQ("87:0:0:0", getValue(parsed, IO::Path("Controls/Camera/Move up")));
        EXPECT_EQ("70:0:0:0", getValue(parsed, IO::Path("Controls/Camera/Move right")));
        EXPECT_EQ("83:0:0:0", getValue(parsed, IO::Path("Controls/Camera/Move left")));
        EXPECT_EQ("68:0:0:0", getValue(parsed, IO::Path("Controls/Camera/Move backward")));
        EXPECT_EQ("69:0:0:0", getValue(parsed, IO::Path("Controls/Camera/Move forward")));
        EXPECT_EQ("0.425781", getValue(parsed, IO::Path("Controls/Camera/Fly move speed")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Move camera in cursor dir")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Use alt to move")));
        EXPECT_EQ("0.350000", getValue(parsed, IO::Path("Controls/Camera/Move speed")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Invert mouse wheel")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Invert vertical pan")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Invert horizontal pan")));
        EXPECT_EQ("0.550000", getValue(parsed, IO::Path("Controls/Camera/Pan speed")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Invert vertical look")));
        EXPECT_EQ("1", getValue(parsed, IO::Path("Controls/Camera/Invert horizontal look")));
        EXPECT_EQ("0.440000", getValue(parsed, IO::Path("Controls/Camera/Look speed")));
        EXPECT_EQ("1.500000", getValue(parsed, IO::Path("Texture Browser/Icon size")));
        EXPECT_EQ("14", getValue(parsed, IO::Path("Renderer/Font size")));
        EXPECT_EQ("9729", getValue(parsed, IO::Path("Renderer/Texture mode mag filter")));
        EXPECT_EQ("9987", getValue(parsed, IO::Path("Renderer/Texture mode min filter")));
        EXPECT_EQ("0.925000", getValue(parsed, IO::Path("Renderer/Brightness")));
        EXPECT_EQ("0", getValue(parsed, IO::Path("Renderer/Show axes")));
        EXPECT_EQ("0.220000", getValue(parsed, IO::Path("Renderer/Grid/Alpha")));
        EXPECT_EQ("0.921569 0.666667 0.45098 1", getValue(parsed, IO::Path("Renderer/Colors/Edges")));
        EXPECT_EQ("0.321569 0.0470588 0.141176 1", getValue(parsed, IO::Path("Renderer/Colors/Background")));
        EXPECT_EQ("0.290196 0.643137 0.486275 1", getValue(parsed, IO::Path("Rendere/Grid/Color2D")));
        EXPECT_EQ("2", getValue(parsed, IO::Path("Views/Map view layout")));
        EXPECT_EQ("/home/ericwa/Quake Dev", getValue(parsed, IO::Path("Games/Quake/Path")));
        EXPECT_EQ("/home/ericwa/foo=bar", getValue(parsed, IO::Path("Games/Generic/Path")));
        EXPECT_EQ("/home/ericwa/Quake 3 Arena", getValue(parsed, IO::Path("Games/Quake 3/Path")));
        EXPECT_EQ("87:308:307:0", getValue(parsed, IO::Path("Menu/File/Export/Wavefront OBJ...")));
        EXPECT_EQ("50:308:307:0", getValue(parsed, IO::Path("Menu/View/Grid/Set Grid Size 0.125")));
        EXPECT_EQ("859", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/x")));
        EXPECT_EQ("473", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/y")));
        EXPECT_EQ("1024", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/w")));
        EXPECT_EQ("768", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/h")));
        EXPECT_EQ("0", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/Maximized")));
        EXPECT_EQ("0", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/Iconized")));
        EXPECT_EQ("0", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_l")));
        EXPECT_EQ("0", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_r")));
        EXPECT_EQ("37", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_t")));
        EXPECT_EQ("0", getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_b")));
        EXPECT_EQ("6533", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio")));
        EXPECT_EQ("8306", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio")));
        EXPECT_EQ("4857", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio")));
        EXPECT_EQ("4850", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio")));
        EXPECT_EQ("2742", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio")));
        EXPECT_EQ("3333", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio")));
        EXPECT_EQ("-10000", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio")));
        EXPECT_EQ("3656", getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio")));
        EXPECT_EQ("/home/ericwa/unnamed.map", getValue(parsed, IO::Path("RecentDocuments/0")));
    }

    static void testV2Prefs(const std::map<IO::Path, QString>& v2) {
        EXPECT_EQ("108", getValue(v2, IO::Path("Controls/Camera/Field of vision")));
        EXPECT_EQ("R", getValue(v2, IO::Path("Controls/Camera/Move down")));
        EXPECT_EQ("W", getValue(v2, IO::Path("Controls/Camera/Move up")));
        EXPECT_EQ("F", getValue(v2, IO::Path("Controls/Camera/Move right")));
        EXPECT_EQ("S", getValue(v2, IO::Path("Controls/Camera/Move left")));
        EXPECT_EQ("D", getValue(v2, IO::Path("Controls/Camera/Move backward")));
        EXPECT_EQ("E", getValue(v2, IO::Path("Controls/Camera/Move forward")));
        EXPECT_EQ("0.425781", getValue(v2, IO::Path("Controls/Camera/Fly move speed")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Move camera in cursor dir")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Use alt to move")));
        EXPECT_EQ("0.35", getValue(v2, IO::Path("Controls/Camera/Move speed")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Invert mouse wheel")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Invert vertical pan")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Invert horizontal pan")));
        EXPECT_EQ("0.55", getValue(v2, IO::Path("Controls/Camera/Pan speed")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Invert vertical look")));
        EXPECT_EQ("1", getValue(v2, IO::Path("Controls/Camera/Invert horizontal look")));
        EXPECT_EQ("0.44", getValue(v2, IO::Path("Controls/Camera/Look speed")));
        EXPECT_EQ("1.5", getValue(v2, IO::Path("Texture Browser/Icon size")));
        EXPECT_EQ("14", getValue(v2, IO::Path("Renderer/Font size")));
        EXPECT_EQ("9729", getValue(v2, IO::Path("Renderer/Texture mode mag filter")));
        EXPECT_EQ("9987", getValue(v2, IO::Path("Renderer/Texture mode min filter")));
        EXPECT_EQ("0.925", getValue(v2, IO::Path("Renderer/Brightness")));
        EXPECT_EQ("0", getValue(v2, IO::Path("Renderer/Show axes")));
        EXPECT_EQ("0.22", getValue(v2, IO::Path("Renderer/Grid/Alpha")));
        EXPECT_EQ("0.921569 0.666667 0.45098 1", getValue(v2, IO::Path("Renderer/Colors/Edges")));
        EXPECT_EQ("0.321569 0.0470588 0.141176 1", getValue(v2, IO::Path("Renderer/Colors/Background")));
        EXPECT_EQ("0.290196 0.643137 0.486275 1", getValue(v2, IO::Path("Rendere/Grid/Color2D")));
        EXPECT_EQ("2", getValue(v2, IO::Path("Views/Map view layout")));
        EXPECT_EQ("/home/ericwa/Quake Dev", getValue(v2, IO::Path("Games/Quake/Path")));
        EXPECT_EQ("/home/ericwa/foo=bar", getValue(v2, IO::Path("Games/Generic/Path")));
        EXPECT_EQ("/home/ericwa/Quake 3 Arena", getValue(v2, IO::Path("Games/Quake 3/Path")));
        EXPECT_EQ("Ctrl+Alt+W", getValue(v2, IO::Path("Menu/File/Export/Wavefront OBJ...")));
        EXPECT_EQ("Ctrl+Alt+2", getValue(v2, IO::Path("Menu/View/Grid/Set Grid Size 0.125")));

        // We don't bother migrating these ones
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/x")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/y")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/w")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/h")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/Maximized")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/Iconized")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_l")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_r")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_t")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_b")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio")));
        EXPECT_EQ("", getValue(v2, IO::Path("RecentDocuments/0")));
    }

    TEST(PreferencesTest, migrateV1) {
        const std::map<IO::Path, QString> v1 = getINISettingsV1("fixture/test/preferences-v1.ini");
        const std::map<IO::Path, QString> v2 = migrateV1ToV2(v1);

        testV2Prefs(v2);

        //EXPECT_TRUE(writeV2SettingsToPath("C:\\Users\\Eric\\Desktop\\preferences.json", v2));
    }

    TEST(PreferencesTest, readV2) {
        const std::map<IO::Path, QString> v2 = readV2SettingsFromPath("fixture/test/preferences-v2.json");
        testV2Prefs(v2);
    }

    /**
     * Helper template so we don't need to use out parameters in the tests
     */
    template <class Serializer, class PrimitiveType>
    static nonstd::optional<PrimitiveType> maybeDeserialize(const QString& string) {
        const Serializer s;
        PrimitiveType result;
        if (s.readFromString(string, &result)) {
            return { result };
        }
        return {};
    }

    template <class Serializer, class PrimitiveType>
    static QString serialize(const PrimitiveType& value) {
        const Serializer s;
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
