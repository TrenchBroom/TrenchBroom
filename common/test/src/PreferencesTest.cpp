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

#include "Color.h"
#include "PreferenceManager.h"
#include "QtPrettyPrinters.h"
#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"
#include "View/Actions.h"

#include <kdl/vector_utils.h>

#include <vecmath/bbox.h>

#include <iostream>
#include <optional>
#include <string>

#include <QTextStream>
#include <QString>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    static QJsonValue getValue(const std::map<IO::Path, QJsonValue>& map, const IO::Path& key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return QJsonValue(QJsonValue::Undefined);
        }
        return it->second;
    }

    TEST_CASE("PreferencesTest.migrateLocalV1Settings", "[PreferencesTest]") {
        const std::map<IO::Path, QJsonValue> reg = readV1Settings();

        [[maybe_unused]]
        const auto migrated = migrateV1ToV2(reg);

        // Can't really test anything because we can't assume the test system
        // has any settings on it.
    }

    TEST_CASE("PreferencesTest.parseV1", "[PreferencesTest]") {
        const std::map<IO::Path, QJsonValue> parsed = getINISettingsV1("fixture/test/preferences-v1.ini");

        EXPECT_EQ(QJsonValue("108.000000"), getValue(parsed, IO::Path("Controls/Camera/Field of vision")));
        EXPECT_EQ(QJsonValue("82:0:0:0"), getValue(parsed, IO::Path("Controls/Camera/Move down")));
        EXPECT_EQ(QJsonValue("87:0:0:0"), getValue(parsed, IO::Path("Controls/Camera/Move up")));
        EXPECT_EQ(QJsonValue("70:0:0:0"), getValue(parsed, IO::Path("Controls/Camera/Move right")));
        EXPECT_EQ(QJsonValue("83:0:0:0"), getValue(parsed, IO::Path("Controls/Camera/Move left")));
        EXPECT_EQ(QJsonValue("68:0:0:0"), getValue(parsed, IO::Path("Controls/Camera/Move backward")));
        EXPECT_EQ(QJsonValue("69:0:0:0"), getValue(parsed, IO::Path("Controls/Camera/Move forward")));
        EXPECT_EQ(QJsonValue("0.425781"), getValue(parsed, IO::Path("Controls/Camera/Fly move speed")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Move camera in cursor dir")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Use alt to move")));
        EXPECT_EQ(QJsonValue("0.350000"), getValue(parsed, IO::Path("Controls/Camera/Move speed")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Invert mouse wheel")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Invert vertical pan")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Invert horizontal pan")));
        EXPECT_EQ(QJsonValue("0.550000"), getValue(parsed, IO::Path("Controls/Camera/Pan speed")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Invert vertical look")));
        EXPECT_EQ(QJsonValue("1"), getValue(parsed, IO::Path("Controls/Camera/Invert horizontal look")));
        EXPECT_EQ(QJsonValue("0.440000"), getValue(parsed, IO::Path("Controls/Camera/Look speed")));
        EXPECT_EQ(QJsonValue("1.500000"), getValue(parsed, IO::Path("Texture Browser/Icon size")));
        EXPECT_EQ(QJsonValue("14"), getValue(parsed, IO::Path("Renderer/Font size")));
        EXPECT_EQ(QJsonValue("9729"), getValue(parsed, IO::Path("Renderer/Texture mode mag filter")));
        EXPECT_EQ(QJsonValue("9987"), getValue(parsed, IO::Path("Renderer/Texture mode min filter")));
        EXPECT_EQ(QJsonValue("0.925000"), getValue(parsed, IO::Path("Renderer/Brightness")));
        EXPECT_EQ(QJsonValue("0"), getValue(parsed, IO::Path("Renderer/Show axes")));
        EXPECT_EQ(QJsonValue("0.220000"), getValue(parsed, IO::Path("Renderer/Grid/Alpha")));
        EXPECT_EQ(QJsonValue("0.921569 0.666667 0.45098 1"), getValue(parsed, IO::Path("Renderer/Colors/Edges")));
        EXPECT_EQ(QJsonValue("0.321569 0.0470588 0.141176 1"), getValue(parsed, IO::Path("Renderer/Colors/Background")));
        EXPECT_EQ(QJsonValue("0.290196 0.643137 0.486275 1"), getValue(parsed, IO::Path("Rendere/Grid/Color2D")));
        EXPECT_EQ(QJsonValue("2"), getValue(parsed, IO::Path("Views/Map view layout")));
        EXPECT_EQ(QJsonValue("/home/ericwa/Quake Dev"), getValue(parsed, IO::Path("Games/Quake/Path")));
        EXPECT_EQ(QJsonValue("/home/ericwa/foo=bar"), getValue(parsed, IO::Path("Games/Generic/Path")));
        EXPECT_EQ(QJsonValue("/home/ericwa/Quake 3 Arena"), getValue(parsed, IO::Path("Games/Quake 3/Path")));
        EXPECT_EQ(QJsonValue("87:308:307:0"), getValue(parsed, IO::Path("Menu/File/Export/Wavefront OBJ...")));
        EXPECT_EQ(QJsonValue("50:308:307:0"), getValue(parsed, IO::Path("Menu/View/Grid/Set Grid Size 0.125")));
        EXPECT_EQ(QJsonValue("859"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/x")));
        EXPECT_EQ(QJsonValue("473"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/y")));
        EXPECT_EQ(QJsonValue("1024"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/w")));
        EXPECT_EQ(QJsonValue("768"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/h")));
        EXPECT_EQ(QJsonValue("0"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/Maximized")));
        EXPECT_EQ(QJsonValue("0"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/Iconized")));
        EXPECT_EQ(QJsonValue("0"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_l")));
        EXPECT_EQ(QJsonValue("0"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_r")));
        EXPECT_EQ(QJsonValue("37"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_t")));
        EXPECT_EQ(QJsonValue("0"), getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_b")));
        EXPECT_EQ(QJsonValue("6533"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("8306"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("4857"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("4850"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("2742"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("3333"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("-10000"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("3656"), getValue(parsed, IO::Path("Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue("/home/ericwa/unnamed.map"), getValue(parsed, IO::Path("RecentDocuments/0")));
        EXPECT_EQ(QJsonValue("68:307:0:0"), getValue(parsed, IO::Path("Filters/Tags/Detail/Toggle Visible")));
        EXPECT_EQ(QJsonValue("68:0:0:0"), getValue(parsed, IO::Path("Tags/Detail/Enable")));
        EXPECT_EQ(QJsonValue("68:307:306:0"), getValue(parsed, IO::Path("Tags/Detail/Disable")));
        EXPECT_EQ(QJsonValue("72:0:0:0"), getValue(parsed, IO::Path("Entities/monster_hell_knight/Create")));
    }

    static void testV2Prefs(const std::map<IO::Path, QJsonValue>& v2) {
        EXPECT_EQ(QJsonValue(108), getValue(v2, IO::Path("Controls/Camera/Field of vision")));
        EXPECT_EQ(QJsonValue("R"), getValue(v2, IO::Path("Controls/Camera/Move down")));
        EXPECT_EQ(QJsonValue("W"), getValue(v2, IO::Path("Controls/Camera/Move up")));
        EXPECT_EQ(QJsonValue("F"), getValue(v2, IO::Path("Controls/Camera/Move right")));
        EXPECT_EQ(QJsonValue("S"), getValue(v2, IO::Path("Controls/Camera/Move left")));
        EXPECT_EQ(QJsonValue("D"), getValue(v2, IO::Path("Controls/Camera/Move backward")));
        EXPECT_EQ(QJsonValue("E"), getValue(v2, IO::Path("Controls/Camera/Move forward")));
        EXPECT_FLOAT_EQ(0.425781f, static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Fly move speed")).toDouble()));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Move camera in cursor dir")));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Use alt to move")));
        EXPECT_FLOAT_EQ(0.35f, static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Move speed")).toDouble()));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Invert mouse wheel")));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Invert vertical pan")));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Invert horizontal pan")));
        EXPECT_FLOAT_EQ(0.55f, static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Pan speed")).toDouble()));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Invert vertical look")));
        EXPECT_EQ(QJsonValue(true), getValue(v2, IO::Path("Controls/Camera/Invert horizontal look")));
        EXPECT_FLOAT_EQ(0.44f, static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Look speed")).toDouble()));
        EXPECT_FLOAT_EQ(1.5f, static_cast<float>(getValue(v2, IO::Path("Texture Browser/Icon size")).toDouble()));
        EXPECT_EQ(QJsonValue(14), getValue(v2, IO::Path("Renderer/Font size")));
        EXPECT_EQ(QJsonValue(9729), getValue(v2, IO::Path("Renderer/Texture mode mag filter")));
        EXPECT_EQ(QJsonValue(9987), getValue(v2, IO::Path("Renderer/Texture mode min filter")));
        EXPECT_FLOAT_EQ(0.925f, static_cast<float>(getValue(v2, IO::Path("Renderer/Brightness")).toDouble()));
        EXPECT_EQ(QJsonValue(false), getValue(v2, IO::Path("Renderer/Show axes")));
        EXPECT_FLOAT_EQ(0.22f, static_cast<float>(getValue(v2, IO::Path("Renderer/Grid/Alpha")).toDouble()));
        EXPECT_EQ(QJsonValue("0.921569 0.666667 0.45098 1"), getValue(v2, IO::Path("Renderer/Brush edge")));
        EXPECT_EQ(QJsonValue("0.321569 0.0470588 0.141176 1"), getValue(v2, IO::Path("Renderer/Editing views background")));
        EXPECT_EQ(QJsonValue("0.290196 0.643137 0.486275 1"), getValue(v2, IO::Path("Renderer/Grid color (2D views)")));
        EXPECT_EQ(QJsonValue(2), getValue(v2, IO::Path("Views/Map view layout")));
        EXPECT_EQ(QJsonValue("/home/ericwa/Quake Dev"), getValue(v2, IO::Path("Games/Quake/Path")));
        EXPECT_EQ(QJsonValue("/home/ericwa/foo=bar"), getValue(v2, IO::Path("Games/Generic/Path")));
        EXPECT_EQ(QJsonValue("/home/ericwa/Quake 3 Arena"), getValue(v2, IO::Path("Games/Quake 3/Path")));
        EXPECT_EQ(QJsonValue("Ctrl+Alt+W"), getValue(v2, IO::Path("Menu/File/Export/Wavefront OBJ...")));
        EXPECT_EQ(QJsonValue("Ctrl+Alt+2"), getValue(v2, IO::Path("Menu/View/Grid/Set Grid Size 0.125")));
        EXPECT_EQ(QJsonValue("Alt+D"), getValue(v2, IO::Path("Filters/Tags/Detail/Toggle Visible")));
        EXPECT_EQ(QJsonValue("D"), getValue(v2, IO::Path("Tags/Detail/Enable")));
        EXPECT_EQ(QJsonValue("Alt+Shift+D"), getValue(v2, IO::Path("Tags/Detail/Disable")));
        EXPECT_EQ(QJsonValue("H"), getValue(v2, IO::Path("Entities/monster_hell_knight/Create")));

        // We don't bother migrating these ones
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/x")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/y")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/w")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/h")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/Maximized")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/Iconized")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_l")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_r")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_t")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_b")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio")));
        EXPECT_EQ(QJsonValue(QJsonValue::Undefined), getValue(v2, IO::Path("RecentDocuments/0")));
    }

    TEST_CASE("PreferencesTest.migrateV1", "[PreferencesTest]") {
        const std::map<IO::Path, QJsonValue> v1 = getINISettingsV1("fixture/test/preferences-v1.ini");
        const std::map<IO::Path, QJsonValue> v2 = migrateV1ToV2(v1);

        testV2Prefs(v2);

        //EXPECT_TRUE(writeV2SettingsToPath("C:\\Users\\Eric\\Desktop\\Preferences.json", v2));
    }

    TEST_CASE("PreferencesTest.readV2", "[PreferencesTest]") {
        // Invalid JSON -> parse error -> parseV2SettingsFromJSON() is expected to return nullopt
        CHECK(parseV2SettingsFromJSON(QByteArray()).is_error_type<PreferenceErrors::JsonParseError>());
        CHECK(parseV2SettingsFromJSON(QByteArray("abc")).is_error_type<PreferenceErrors::JsonParseError>());
        CHECK(parseV2SettingsFromJSON(QByteArray(R"({"foo": "bar",})")).is_error_type<PreferenceErrors::JsonParseError>());

        // Valid JSON
        CHECK(parseV2SettingsFromJSON(QByteArray(R"({"foo": "bar"})")).is_success());
        CHECK(parseV2SettingsFromJSON(QByteArray("{}")).is_success());

        const PreferencesResult v2 = readV2SettingsFromPath("fixture/test/preferences-v2.json");
        CHECK(v2.is_success());
        v2.visit(kdl::overload(
            [](const std::map<IO::Path, QJsonValue>& prefs) {
               testV2Prefs(prefs);
            },
            [](const PreferenceErrors::NoFilePresent&) {
                FAIL_CHECK();
            },
            [](const PreferenceErrors::JsonParseError&) {
                FAIL_CHECK();
            },
            [](const PreferenceErrors::FileReadError&) {
                FAIL_CHECK();
            }
        ));
    }

    TEST_CASE("PreferencesTest.testWriteReadV2", "[PreferencesTest]") {
        const std::map<IO::Path, QJsonValue> v1 = getINISettingsV1("fixture/test/preferences-v1.ini");
        const std::map<IO::Path, QJsonValue> v2 = migrateV1ToV2(v1);

        const QByteArray v2Serialized = writeV2SettingsToJSON(v2);
        const auto v2Deserialized = parseV2SettingsFromJSON(v2Serialized);

        CHECK(v2Deserialized.is_success());
        v2Deserialized.visit(kdl::overload(
            [&](const std::map<IO::Path, QJsonValue>& prefs) {
                CHECK(v2 == prefs);
            },
            [](const PreferenceErrors::NoFilePresent&) {
                FAIL_CHECK();
            },
            [](const PreferenceErrors::JsonParseError&) {
                FAIL_CHECK();
            },
            [](const PreferenceErrors::FileReadError&) {
                FAIL_CHECK();
            }
        ));
    }

    /**
     * Helper template so we don't need to use out parameters in the tests
     */
    template <class Serializer, class PrimitiveType>
    static std::optional<PrimitiveType> maybeDeserialize(const QJsonValue& string) {
        const Serializer s;
        PrimitiveType result;
        if (s.readFromJSON(string, &result)) {
            return { result };
        }
        return std::nullopt;
    }

    template <class Serializer, class PrimitiveType>
    static QJsonValue serialize(const PrimitiveType& value) {
        const Serializer s;
        return s.writeToJSON(value);
    }

    template <class Serializer, class PrimitiveType>
    static void testSerialize(const QJsonValue& str, const PrimitiveType& value) {
        const auto testDeserializeOption = maybeDeserialize<Serializer, PrimitiveType>(str);
        const QJsonValue testSerialize = serialize<Serializer, PrimitiveType>(value);

        ASSERT_TRUE(testDeserializeOption.has_value());

        EXPECT_EQ(value, testDeserializeOption.value());
        EXPECT_EQ(str, testSerialize);
    }

    TEST_CASE("PreferencesTest.serializeV1Bool", "[PreferencesTest]") {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("").has_value()));
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("-1").has_value()));

        testSerialize<PreferenceSerializerV1, bool>(QJsonValue("0"), false);
        testSerialize<PreferenceSerializerV1, bool>(QJsonValue("1"), true);
    }

    TEST_CASE("PreferencesTest.serializeV1Color", "[PreferencesTest]") {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV1, Color>(QJsonValue("0.921569 0.666667")).has_value())); // must give 3 or 4 components

        testSerialize<PreferenceSerializerV1, Color>(
            QJsonValue("0.921569 0.666667 0.45098 0.5"),
            Color(0.921569f, 0.666667f, 0.45098f, 0.5f));
    }

    TEST_CASE("PreferencesTest.serializeV1float", "[PreferencesTest]") {
        testSerialize<PreferenceSerializerV1, float>(QJsonValue("0.921569"), 0.921569f);
    }

    TEST_CASE("PreferencesTest.serializeV1int", "[PreferencesTest]") {
        testSerialize<PreferenceSerializerV1, int>(QJsonValue("0"), 0);
        testSerialize<PreferenceSerializerV1, int>(QJsonValue("-1"), -1);
        testSerialize<PreferenceSerializerV1, int>(QJsonValue("1000"), 1000);
    }

    TEST_CASE("PreferencesTest.serializeV1Path", "[PreferencesTest]") {
#ifdef _WIN32
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("c:\\foo\\bar"), IO::Path("c:\\foo\\bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("c:\\foo\\bar"), IO::Path("c:/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("\\home\\foo\\bar"), IO::Path("/home/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("\\home\\foo\\bar"), IO::Path("\\home\\foo\\bar"));
#else
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("c:/foo/bar"), IO::Path("c:\\foo\\bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("c:/foo/bar"), IO::Path("c:/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("/home/foo/bar"), IO::Path("/home/foo/bar"));
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue("home/foo/bar"), IO::Path("\\home\\foo\\bar"));
#endif
        testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue(""), IO::Path());
    }

    TEST_CASE("PreferencesTest.serializeV1KeyboardShortcut", "[PreferencesTest]") {
        // These come from wxWidgets TrenchBroom 2019.6, on Windows
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("87:307:306:0"),   QKeySequence::fromString("Alt+Shift+W"));
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("87:307:0:0"),     QKeySequence::fromString("Alt+W"));
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("87:308:307:0"),   QKeySequence::fromString("Ctrl+Alt+W"));
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("87:306:0:0"),     QKeySequence::fromString("Shift+W"));
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("77:308:0:0"),     QKeySequence::fromString("Ctrl+M"));
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("65:308:307:306"), QKeySequence::fromString("Ctrl+Alt+Shift+A"));
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("348:306:0:0"),    QKeySequence::fromString("Shift+F9"));

        // From macOS
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("80:308:0:0"),     QKeySequence::fromString("Ctrl+P")); // "Ctrl" in Qt = Command in macOS
        testSerialize<PreferenceSerializerV1, QKeySequence>(QJsonValue("80:307:0:0"),     QKeySequence::fromString("Alt+P")); // "Alt" in Qt = Alt in macOS
    }

    TEST_CASE("PreferencesTest.serializeV2Bool", "[PreferencesTest]") {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV2, bool>(QJsonValue("")).has_value()));
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV2, bool>(QJsonValue("0")).has_value()));

        testSerialize<PreferenceSerializerV2, bool>(QJsonValue(false), false);
        testSerialize<PreferenceSerializerV2, bool>(QJsonValue(true), true);
    }

    TEST_CASE("PreferencesTest.serializeV2float", "[PreferencesTest]") {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV2, float>(QJsonValue("1.25")).has_value()));

        testSerialize<PreferenceSerializerV2, float>(QJsonValue(1.25), 1.25f);
    }

    TEST_CASE("PreferencesTest.serializeV2int", "[PreferencesTest]") {
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV2, int>(QJsonValue("0")).has_value()));
        EXPECT_FALSE((maybeDeserialize<PreferenceSerializerV2, int>(QJsonValue("-1")).has_value()));

        testSerialize<PreferenceSerializerV2, int>(QJsonValue(0), 0);
        testSerialize<PreferenceSerializerV2, int>(QJsonValue(-1), -1);
        testSerialize<PreferenceSerializerV2, int>(QJsonValue(1000), 1000);
    }

    TEST_CASE("PreferencesTest.serializeV2KeyboardShortcut", "[PreferencesTest]") {
        testSerialize<PreferenceSerializerV2, QKeySequence>(QJsonValue("Alt+Shift+W"),    QKeySequence::fromString("Alt+Shift+W"));
        testSerialize<PreferenceSerializerV2, QKeySequence>(QJsonValue("Meta+W"),         QKeySequence::fromString("Meta+W")); // "Meta" in Qt = Control in macOS
    }

    TEST_CASE("PreferencesTest.testWxViewShortcutsAndMenuShortcutsRecognized", "[PreferencesTest]") {
        // All map view shortcuts, and all binadable menu items before the Qt port
        const std::vector<std::string> preferenceKeys {
            "Controls/Map view/Create brush",
            "Controls/Map view/Toggle clip side",
            "Controls/Map view/Perform clip",
            // The Qt port dropped these - now they're merged with the "Move objects" actions
//            "Controls/Map view/Move vertices up; Move vertices forward",
//            "Controls/Map view/Move vertices down; Move vertices backward",
//            "Controls/Map view/Move vertices left",
//            "Controls/Map view/Move vertices right",
//            "Controls/Map view/Move vertices backward; Move vertices up",
//            "Controls/Map view/Move vertices forward; Move vertices down",
//            "Controls/Map view/Move rotation center up; Move rotation center forward",
//            "Controls/Map view/Move rotation center down; Move rotation center backward",
//            "Controls/Map view/Move rotation center left",
//            "Controls/Map view/Move rotation center right",
//            "Controls/Map view/Move rotation center backward; Move rotation center up",
//            "Controls/Map view/Move rotation center forward; Move rotation center down",
            "Controls/Map view/Move objects up; Move objects forward",
            "Controls/Map view/Move objects down; Move objects backward",
            "Controls/Map view/Move objects left",
            "Controls/Map view/Move objects right",
            "Controls/Map view/Move objects backward; Move objects up",
            "Controls/Map view/Move objects forward; Move objects down",
            "Controls/Map view/Roll objects clockwise",
            "Controls/Map view/Roll objects counter-clockwise",
            "Controls/Map view/Yaw objects clockwise",
            "Controls/Map view/Yaw objects counter-clockwise",
            "Controls/Map view/Pitch objects clockwise",
            "Controls/Map view/Pitch objects counter-clockwise",
            "Controls/Map view/Flip objects horizontally",
            "Controls/Map view/Flip objects vertically",
            "Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward",
            "Controls/Map view/Duplicate and move objects down; Duplicate and move objects backward",
            "Controls/Map view/Duplicate and move objects left",
            "Controls/Map view/Duplicate and move objects right",
            "Controls/Map view/Duplicate and move objects backward; Duplicate and move objects up",
            "Controls/Map view/Duplicate and move objects forward; Duplicate and move objects down",
            "Controls/Map view/Move textures up",
            "Controls/Map view/Move textures up (fine)",
            "Controls/Map view/Move textures up (coarse)",
            "Controls/Map view/Move textures down",
            "Controls/Map view/Move textures down (fine)",
            "Controls/Map view/Move textures down (coarse)",
            "Controls/Map view/Move textures left",
            "Controls/Map view/Move textures left (fine)",
            "Controls/Map view/Move textures left (coarse)",
            "Controls/Map view/Move textures right",
            "Controls/Map view/Move textures right (fine)",
            "Controls/Map view/Move textures right (coarse)",
            "Controls/Map view/Rotate textures clockwise",
            "Controls/Map view/Rotate textures clockwise (fine)",
            "Controls/Map view/Rotate textures clockwise (coarse)",
            "Controls/Map view/Rotate textures counter-clockwise",
            "Controls/Map view/Rotate textures counter-clockwise (fine)",
            "Controls/Map view/Rotate textures counter-clockwise (coarse)",
            "Controls/Map view/Cycle map view",
            "Controls/Map view/Reset camera zoom",
            "Controls/Map view/Cancel",
            "Controls/Map view/Deactivate current tool",
            "Controls/Map view/Make structural",
            "Controls/Map view/View Filter > Toggle show entity classnames",
            "Controls/Map view/View Filter > Toggle show group bounds",
            "Controls/Map view/View Filter > Toggle show brush entity bounds",
            "Controls/Map view/View Filter > Toggle show point entity bounds",
            "Controls/Map view/View Filter > Toggle show point entities",
            "Controls/Map view/View Filter > Toggle show point entity models",
            "Controls/Map view/View Filter > Toggle show brushes",
            "Controls/Map view/View Filter > Show textures",
            "Controls/Map view/View Filter > Hide textures",
            "Controls/Map view/View Filter > Hide faces",
            "Controls/Map view/View Filter > Shade faces",
            "Controls/Map view/View Filter > Use fog",
            "Controls/Map view/View Filter > Show edges",
            "Controls/Map view/View Filter > Show all entity links",
            "Controls/Map view/View Filter > Show transitively selected entity links",
            "Controls/Map view/View Filter > Show directly selected entity links",
            "Controls/Map view/View Filter > Hide entity links",
            "Menu/File/Export/Wavefront OBJ...",
            "Menu/File/Load Point File...",
            "Menu/File/Reload Point File",
            "Menu/File/Unload Point File",
            "Menu/File/Load Portal File...",
            "Menu/File/Reload Portal File",
            "Menu/File/Unload Portal File",
            "Menu/File/Reload Texture Collections",
            "Menu/File/Reload Entity Definitions",
            "Menu/Edit/Repeat",
            "Menu/Edit/Paste at Original Position",
            "Menu/Edit/Clear Repeatable Commands",
            "Menu/Edit/Duplicate",
            "Menu/Edit/Delete",
            "Menu/Edit/Select All",
            "Menu/Edit/Select Siblings",
            "Menu/Edit/Select Touching",
            "Menu/Edit/Select Inside",
            "Menu/Edit/Select Tall",
            "Menu/Edit/Select by Line Number",
            "Menu/Edit/Select None",
            "Menu/Edit/Group",
            "Menu/Edit/Ungroup",
            "Menu/Edit/Tools/Brush Tool",
            "Menu/Edit/Tools/Clip Tool",
            "Menu/Edit/Tools/Rotate Tool",
            "Menu/Edit/Tools/Scale Tool",
            "Menu/Edit/Tools/Shear Tool",
            "Menu/Edit/Tools/Vertex Tool",
            "Menu/Edit/Tools/Edge Tool",
            "Menu/Edit/Tools/Face Tool",
            "Menu/Edit/CSG/Convex Merge",
            "Menu/Edit/CSG/Subtract",
            "Menu/Edit/CSG/Hollow",
            "Menu/Edit/CSG/Intersect",
            "Menu/Edit/Snap Vertices to Integer",
            "Menu/Edit/Snap Vertices to Grid",
            "Menu/Edit/Texture Lock",
            "Menu/Edit/UV Lock",
            "Menu/Edit/Replace Texture...",
            "Menu/View/Grid/Show Grid",
            "Menu/View/Grid/Snap to Grid",
            "Menu/View/Grid/Increase Grid Size",
            "Menu/View/Grid/Decrease Grid Size",
            "Menu/View/Grid/Set Grid Size 0.125",
            "Menu/View/Grid/Set Grid Size 0.25",
            "Menu/View/Grid/Set Grid Size 0.5",
            "Menu/View/Grid/Set Grid Size 1",
            "Menu/View/Grid/Set Grid Size 2",
            "Menu/View/Grid/Set Grid Size 4",
            "Menu/View/Grid/Set Grid Size 8",
            "Menu/View/Grid/Set Grid Size 16",
            "Menu/View/Grid/Set Grid Size 32",
            "Menu/View/Grid/Set Grid Size 64",
            "Menu/View/Grid/Set Grid Size 128",
            "Menu/View/Grid/Set Grid Size 256",
            "Menu/View/Camera/Move to Next Point",
            "Menu/View/Camera/Move to Previous Point",
            "Menu/View/Camera/Focus on Selection",
            "Menu/View/Camera/Move Camera to...",
            "Menu/View/Isolate",
            "Menu/View/Hide",
            "Menu/View/Show All",
            "Menu/View/Switch to Map Inspector",
            "Menu/View/Switch to Entity Inspector",
            "Menu/View/Switch to Face Inspector",
            "Menu/View/Toggle Info Panel",
            "Menu/View/Toggle Inspector",
            "Menu/View/Maximize Current View",
            "Menu/Run/Compile...",
            "Menu/Run/Launch...",
        };

        auto& actionsMap = View::ActionManager::instance().actionsMap();
        for (const std::string& preferenceKey : preferenceKeys) {
            const auto preferencePath = IO::Path(preferenceKey);
            const bool found = (actionsMap.find(preferencePath) != actionsMap.end());
            EXPECT_TRUE(found);

            if (!found) {
                std::cerr << "Couldn't find key: '" << preferenceKey << "'\n";
            }
        }
    }

    TEST_CASE("PreferencesTest.testWxEntityShortcuts", "[PreferencesTest]") {
        auto hellKnight = Assets::PointEntityDefinition("monster_hell_knight", Color(0,0,0), vm::bbox3(), "", {}, Assets::ModelDefinition());
        const auto defs = std::vector<Assets::EntityDefinition*>{&hellKnight};

        const std::vector<std::unique_ptr<View::Action>> actions = View::ActionManager::instance().createEntityDefinitionActions(defs);
        const std::vector<IO::Path> actualPrefPaths = kdl::vec_transform(actions, [](const auto& action) { return IO::Path(action->preferencePath()); });

        // example keys from 2019.6 for "monster_hell_knight" entity
        const std::vector<std::string> preferenceKeys {
            "Entities/monster_hell_knight/Create",
            "Entities/monster_hell_knight/Toggle" // new in 2020.1
        };

        for (const std::string& preferenceKey : preferenceKeys) {
            const bool found = kdl::vec_contains(actualPrefPaths, IO::Path(preferenceKey));
            EXPECT_TRUE(found);

            if (!found) {
                std::cerr << "Couldn't find key: '" << preferenceKey << "'\n";
            }
        }
    }

    TEST_CASE("PreferencesTest.testWxTagShortcuts", "[PreferencesTest]") {
        const auto tags = std::vector<Model::SmartTag>{
            Model::SmartTag("Detail", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1 << 27))
        };
        const std::vector<std::unique_ptr<View::Action>> actions = View::ActionManager::instance().createTagActions(tags);
        const std::vector<IO::Path> actualPrefPaths = kdl::vec_transform(actions, [](const auto& action) { return IO::Path(action->preferencePath()); });

        // example keys from 2019.6 for "Detail" tag
        const std::vector<std::string> preferenceKeys {
            "Filters/Tags/Detail/Toggle Visible",
            "Tags/Detail/Disable",
            "Tags/Detail/Enable",
        };

        for (const std::string& preferenceKey : preferenceKeys) {
            const bool found = kdl::vec_contains(actualPrefPaths, IO::Path(preferenceKey));
            EXPECT_TRUE(found);

            if (!found) {
                std::cerr << "Couldn't find key: '" << preferenceKey << "'\n";
            }
        }
    }
}
