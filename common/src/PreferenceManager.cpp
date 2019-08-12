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

#include "PreferenceManager.h"

#include "Preferences.h"
#include "View/Actions.h"
#include "View/KeyboardShortcut.h"

namespace TrenchBroom {
    // PreferenceSerializerV1

    bool PreferenceSerializerV1::readFromString(const QString& in, bool* out) {
        if (in == QStringLiteral("1")) {
            *out = true;
            return true;
        } else if (in == QStringLiteral("0")) {
            *out = false;
            return true;
        } else {
            return false;
        }
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, Color* out) {
        const std::string inStdString = in.toStdString();

        if (!Color::canParse(inStdString)) {
            return false;
        }
        
        *out = Color::parse(inStdString);
        return true;
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, float* out) {
        auto inCopy = QString(in);
        auto inStream = QTextStream(&inCopy);

        inStream >> *out;

        return (inStream.status() == QTextStream::Ok);
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, int* out) {
        auto inCopy = QString(in);
        auto inStream = QTextStream(&inCopy);

        inStream >> *out;

        return (inStream.status() == QTextStream::Ok);
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, IO::Path* out) {
        *out = IO::Path::fromQString(in);
        return true;
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, View::KeyboardShortcut* out) {
        nonstd::optional<View::KeyboardShortcut> result = 
            View::KeyboardShortcut::fromV1Settings(in);
        
        if (!result.has_value()) {
            return false;
        }
        *out = result.value();
        return true;
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, QString* out) {
        *out = in;
        return true;
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const bool in) {
        if (in) {
            stream << "1";
        } else {
            stream << "0";
        }
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const Color& in) {
        // NOTE: QTextStream's default locale is C, unlike QString::arg()
        stream << in.r() << " "
               << in.g() << " "
               << in.b() << " "
               << in.a();
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const float in) {
        stream << in;
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const int in) {
        stream << in;
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const IO::Path& in) {
        // NOTE: this serializes with "\" separators on Windows and "/" elsewhere!
        stream << in.asQString();
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const View::KeyboardShortcut& in) {
        stream << in.toV1Settings();
    }
    
    void PreferenceSerializerV1::writeToString(QTextStream& stream, const QString& in) {
        stream << in;
    }

    // PreferenceSerializerV2

    bool PreferenceSerializerV2::readFromString(const QString& in, View::KeyboardShortcut* out) {
        *out = View::KeyboardShortcut(QKeySequence(in, QKeySequence::PortableText));
        return true;
    }

    void PreferenceSerializerV2::writeToString(QTextStream& stream, const View::KeyboardShortcut& in) {
        stream << in.keySequence().toString(QKeySequence::PortableText);
    }

    // PreferenceManager

    void PreferenceManager::markAsUnsaved(PreferenceBase* preference) {
        m_unsavedPreferences.insert(preference);
    }

    PreferenceManager& PreferenceManager::instance() {
        static PreferenceManager prefs;
        return prefs;
    }

    bool PreferenceManager::saveInstantly() const {
        return m_saveInstantly;
    }

    PreferenceBase::Set PreferenceManager::saveChanges() {
        PreferenceBase::Set changedPreferences;
        for (auto* pref : m_unsavedPreferences) {
            pref->save();
            preferenceDidChangeNotifier(pref->path());

            changedPreferences.insert(pref);
        }

        m_unsavedPreferences.clear();
        return changedPreferences;
    }

    PreferenceBase::Set PreferenceManager::discardChanges() {
        PreferenceBase::Set changedPreferences;
        for (auto* pref : m_unsavedPreferences) {
            pref->resetToPrevious();
            changedPreferences.insert(pref);
        }

        m_unsavedPreferences.clear();
        return changedPreferences;
    }

    PreferenceManager::PreferenceManager() {
#if defined __APPLE__
        m_saveInstantly = true;
#else
        m_saveInstantly = false;
#endif
    }

    std::map<IO::Path, QString> parseINI(QTextStream* iniStream) {
        IO::Path section;
        std::map<IO::Path, QString> result;

        while (!iniStream->atEnd()) {
            QString line = iniStream->readLine();

            // Trim leading/trailing whitespace
            line = line.trimmed();

            // Unescape escape sequences
            line.replace("\\ ", " ");

            // TODO: Handle comments, if we want to.

            const bool sqBracketAtStart = line.startsWith('[');
            const bool sqBracketAtEnd = line.endsWith(']');

            const bool heading = sqBracketAtStart && sqBracketAtEnd;
            if (heading) {
                const QString sectionString = line.mid(1, line.length() - 2);
                // NOTE: This parses the section
                section = IO::Path::fromQString(sectionString);
                continue;
            }

            //  Not a heading, see if it's a key=value entry
            const int eqIndex = line.indexOf('=');
            if (eqIndex != -1) {
                QString key = line.left(eqIndex);
                QString value = line.mid(eqIndex + 1);

                result[section + IO::Path::fromQString(key)] = value;
                continue;
            }

            // Line was ignored
        }
        return result;
    }

    static void visitNode(std::map<IO::Path, QString>* result, QSettings* settings, const IO::Path& currentPath) {
        // Process key/value pairs at this node
        for (const QString& key : settings->childKeys()) {
            const QString value = settings->value(key).toString();
            const IO::Path keyPath = currentPath + IO::Path::fromQString(key);
            (*result)[keyPath] = value;
        }

        // Vist children
        for (const QString& childGroup : settings->childGroups()) {
            settings->beginGroup(childGroup);
            visitNode(result, settings, currentPath + IO::Path::fromQString(childGroup));
            settings->endGroup();
        }
    }

    std::map<IO::Path, QString> getRegistrySettingsV1() {
        std::map<IO::Path, QString> result;

#if defined(Q_OS_WIN)
        QSettings s("HKEY_CURRENT_USER\\Software\\Kristian Duske\\TrenchBroom", QSettings::Registry32Format);
        visitNode(&result, &s, IO::Path());
#endif

        return result;
    }

    static bool matches(const IO::Path& path, const IO::Path& glob) {
        const size_t pathLen = path.length();
        const size_t globLen = glob.length();

        if (pathLen != globLen) {
            return false;
        }

        const StringList& pathComps = path.components();
        const StringList& globComps = glob.components();

        for (size_t i = 0; i < globLen; ++i) {
            if (globComps[i] == "*") {
                // Wildcard, so we don't care what pathComps[i] is
                continue;
            }
            if (globComps[i] != pathComps[i]) {
                return false;
            }
        }

        return true;
    }

    std::map<IO::Path, QString> migrateV1ToV2(const std::map<IO::Path, QString>& v1Prefs) {
        auto& map = Preferences::staticPreferencesMap();
        auto& actionsMap = View::ActionManager::instance().actionsMap();
        auto& dynaimcPrefPatterns = Preferences::dynaimcPreferencePatterns();

        PreferenceSerializerV1 v1;
        PreferenceSerializerV2 v2;

        std::map<IO::Path, QString> result;

        for (auto [key, val] : v1Prefs) {
            qDebug() << key.asQString() << "=" << val;

            if (auto it = map.find(key); it != map.end()) {
                PreferenceBase* prefBase = it->second;

                auto strMaybe = prefBase->migratePreferenceForThisType(v1, v2, val);

                if (!strMaybe.has_value()) {
                    qDebug() << " failed to migrate pref for " << key.asQString();
                } else {
                    result[key] = *strMaybe;
                }
            } else if (auto it = actionsMap.find(key); it != actionsMap.end()) {
                // assume it's a View::KeyboardShortcut

                auto strMaybe = migratePreference<View::KeyboardShortcut>(v1, v2, val);

                if (!strMaybe.has_value()) {
                    qDebug() << " failed to migrate pref for " << key.asQString();
                } else {
                    result[key] = *strMaybe;
                }
            } else {
                // check dynamic patters
                bool found = false;

                for (DynamicPreferencePatternBase* dynPref : dynaimcPrefPatterns) {
                    if (matches(key, dynPref->pathPattern())) {
                        found = true;

                        qDebug() << "   " << key.asQString() << " matches pattern " << dynPref->pathPattern().asQString();

                        auto strMaybe = dynPref->migratePreferenceForThisType(v1, v2, val);
                        
                        if (!strMaybe.has_value()) {
                            qDebug() << " failed to migrate pref for " << key.asQString();
                        }
                        else {
                            result[key] = *strMaybe;
                        }

                        break;
                    }
                }
                
                if (!found)
                    qDebug() << "   did not find migration for " << key.asQString();
            }            
        }

        return result;
    }
}
