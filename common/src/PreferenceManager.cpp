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
#include "IO/PathQt.h"
#include "IO/SystemPaths.h"
#include "View/Actions.h"
#include "View/KeyboardShortcut.h"

#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#if defined(Q_OS_WIN)
#include <QSettings>
#endif
#include <QStandardPaths>
#include <QStringBuilder>

#include <string>
#include <vector>

namespace TrenchBroom {
    // PreferenceSerializerV1

    bool PreferenceSerializerV1::readFromString(const QString& in, bool* out) const {
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

    bool PreferenceSerializerV1::readFromString(const QString& in, Color* out) const {
        const std::string inStdString = in.toStdString();

        if (!Color::canParse(inStdString)) {
            return false;
        }

        *out = Color::parse(inStdString);
        return true;
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, float* out) const {
        auto inCopy = QString(in);
        auto inStream = QTextStream(&inCopy);

        inStream >> *out;

        return (inStream.status() == QTextStream::Ok);
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, int* out) const {
        auto inCopy = QString(in);
        auto inStream = QTextStream(&inCopy);

        inStream >> *out;

        return (inStream.status() == QTextStream::Ok);
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, IO::Path* out) const {
        *out = IO::pathFromQString(in);
        return true;
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, QKeySequence* out) const {
        nonstd::optional<QKeySequence> result =
            View::keySequenceFromV1Settings(in);

        if (!result.has_value()) {
            return false;
        }
        *out = result.value();
        return true;
    }

    bool PreferenceSerializerV1::readFromString(const QString& in, QString* out) const {
        *out = in;
        return true;
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const bool in) const {
        if (in) {
            stream << "1";
        } else {
            stream << "0";
        }
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const Color& in) const {
        // NOTE: QTextStream's default locale is C, unlike QString::arg()
        stream << in.r() << " "
               << in.g() << " "
               << in.b() << " "
               << in.a();
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const float in) const {
        stream << in;
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const int in) const {
        stream << in;
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const IO::Path& in) const {
        // NOTE: this serializes with "\" separators on Windows and "/" elsewhere!
        stream << IO::pathAsQString(in);
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const QKeySequence& in) const {
        stream << View::keySequenceToV1Settings(in);
    }

    void PreferenceSerializerV1::writeToString(QTextStream& stream, const QString& in) const {
        stream << in;
    }

    // PreferenceSerializerV2

    bool PreferenceSerializerV2::readFromString(const QString& in, QKeySequence* out) const {
        *out = QKeySequence(in, QKeySequence::PortableText);
        return true;
    }

    void PreferenceSerializerV2::writeToString(QTextStream& stream, const QKeySequence& in) const {
        stream << in.toString(QKeySequence::PortableText);
    }

    // PreferenceManager

    void PreferenceManager::markAsUnsaved(PreferenceBase* preference) {
        m_unsavedPreferences.insert(preference);
    }

    PreferenceManager::PreferenceManager()
    : QObject(),
    m_preferencesFilePath(v2SettingsPath()),
    m_fileSystemWatcher(nullptr) {
#if defined __APPLE__
        m_saveInstantly = true;
#else
        m_saveInstantly = false;
#endif

        migrateSettingsFromV1IfPathDoesNotExist(m_preferencesFilePath);

        this->loadCacheFromDisk();

        m_fileSystemWatcher = new QFileSystemWatcher(this);
        if (m_fileSystemWatcher->addPath(m_preferencesFilePath)) {
            connect(m_fileSystemWatcher, &QFileSystemWatcher::QFileSystemWatcher::fileChanged, this, [this](){
                this->loadCacheFromDisk();
            });
        }
    }

    PreferenceManager& PreferenceManager::instance() {
        ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

        static PreferenceManager prefs;
        return prefs;
    }

    bool PreferenceManager::saveInstantly() const {
        return m_saveInstantly;
    }

    void PreferenceManager::saveChanges() {
        for (auto* pref : m_unsavedPreferences) {
            savePreferenceToCache(pref);
            preferenceDidChangeNotifier(pref->path());
        }
        m_unsavedPreferences.clear();

        assertResult(writeV2SettingsToPath(m_preferencesFilePath, m_cache))
    }

    void PreferenceManager::discardChanges() {
        m_unsavedPreferences.clear();
        invalidatePreferences();
    }

    static std::vector<IO::Path>
    changedKeysForMapDiff(const std::map<IO::Path, QString>& before,
                          const std::map<IO::Path, QString>& after) {
        kdl::vector_set<IO::Path> result;

        // removes
        for (auto& [k, v] : before) {
            unused(v);
            if (after.find(k) == after.end()) {
                // removal
                result.insert(k);
            }
        }

        // adds/updates
        for (auto& [k, v] : after) {
            auto beforeIt = before.find(k);
            if (beforeIt == before.end()) {
                // add
                result.insert(k);
            } else {
                // check for update?
                if (beforeIt->second != v) {
                    result.insert(k);
                }
            }
        }

        return result.release_data();
    }

    /**
     * Reloads m_cache from the .json file,
     * marks all Preference<T> objects as needing deserialization next time they're accessed, and emits
     * preferenceDidChangeNotifier as needed.
     */
    void PreferenceManager::loadCacheFromDisk() {
        const std::map<IO::Path, QString> oldPrefs = m_cache;

        // Reload m_cache
        m_cache = readV2SettingsFromPath(m_preferencesFilePath);
        invalidatePreferences();

        // Emit preferenceDidChangeNotifier for any changed preferences
        const std::vector<IO::Path> changedKeys = changedKeysForMapDiff(oldPrefs, m_cache);
        for (const auto& changedPath : changedKeys) {
            preferenceDidChangeNotifier(changedPath);
        }
    }

    void PreferenceManager::invalidatePreferences() {
        // Force all currently known Preference<T> objects to deserialize from m_cache next time they are accessed
        // Note, because new Preference<T> objects can be created at runtime,
        // we need this sort of lazy loading system.
        for (auto* pref : Preferences::staticPreferences()) {
            pref->setValid(false);
        }
        for (auto& [path, prefPtr] : m_dynamicPreferences) {
            unused(path);
            prefPtr->setValid(false);
        }
    }

    /**
     * Updates the given PreferenceBase from m_cache.
     */
    void PreferenceManager::loadPreferenceFromCache(PreferenceBase* pref) {
        PreferenceSerializerV2 format;

        auto it = m_cache.find(pref->path());
        if (it == m_cache.end()) {
            // no value set, use the default value
            pref->resetToDefault();
            pref->setValid(true);
            return;
        }

        const QString stringValue = it->second;
        assertResult(pref->loadFromString(format, stringValue))
        pref->setValid(true);
    }

    void PreferenceManager::savePreferenceToCache(PreferenceBase* pref) {
        PreferenceSerializerV2 format;
        const QString stringValue = pref->writeToString(format);

        m_cache[pref->path()] = stringValue;
    }

    // V1 settings

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
                section = IO::pathFromQString(sectionString);
                continue;
            }

            //  Not a heading, see if it's a key=value entry
            const int eqIndex = line.indexOf('=');
            if (eqIndex != -1) {
                QString key = line.left(eqIndex);
                QString value = line.mid(eqIndex + 1);

                result[section + IO::pathFromQString(key)] = value;
                continue;
            }

            // Line was ignored
        }
        return result;
    }

#if defined(Q_OS_WIN)
    /**
     * Helper for reading the Windows registry QSettings into a std::map<IO::Path, QString>
     */
    static void visitNode(std::map<IO::Path, QString>& result, QSettings& settings, const IO::Path& currentPath) {
        // Process key/value pairs at this node
        for (const QString& key : settings.childKeys()) {
            const QString value = settings.value(key).toString();
            const IO::Path keyPath = currentPath + IO::pathFromQString(key);
            result[keyPath] = value;
        }

        // Vist children
        for (const QString& childGroup : settings.childGroups()) {
            settings.beginGroup(childGroup);
            visitNode(result, settings, currentPath + IO::pathFromQString(childGroup));
            settings.endGroup();
        }
    }

    static std::map<IO::Path, QString> getRegistrySettingsV1() {
        std::map<IO::Path, QString> result;

        QSettings settings("HKEY_CURRENT_USER\\Software\\Kristian Duske\\TrenchBroom", QSettings::Registry32Format);
        visitNode(result, settings, IO::Path());

        return result;
    }
#endif

    std::map<IO::Path, QString> getINISettingsV1(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {};
        }

        QTextStream in(&file);
        const std::map<IO::Path, QString> result = parseINI(&in);

        return result;
    }

    std::map<IO::Path, QString> readV1Settings() {
        [[maybe_unused]]
        const QString linuxPath = QDir::homePath() % QLatin1String("/.TrenchBroom/.preferences");

        [[maybe_unused]]
        const QString macOSPath = QStandardPaths::locate(QStandardPaths::ConfigLocation,
            QString::fromLocal8Bit("TrenchBroom Preferences"),
            QStandardPaths::LocateOption::LocateFile);

#if defined(Q_OS_WIN)
        return getRegistrySettingsV1();
#elif defined __linux__ || defined __FreeBSD__
        return getINISettingsV1(linuxPath);
#elif defined __APPLE__
        return getINISettingsV1(macOSPath);
#else
        return {};
#endif
    }

    static bool matches(const IO::Path& path, const IO::Path& glob) {
        const size_t pathLen = path.length();
        const size_t globLen = glob.length();

        if (pathLen != globLen) {
            return false;
        }

        const std::vector<std::string>& pathComps = path.components();
        const std::vector<std::string>& globComps = glob.components();

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

        const PreferenceSerializerV1 v1;
        const PreferenceSerializerV2 v2;

        std::map<IO::Path, QString> result;

        for (auto [key, val] : v1Prefs) {

            // try Preferences::staticPreferencesMap()
            {
                const auto it = map.find(key);
                if (it != map.end()) {
                    PreferenceBase* prefBase = it->second;

                    const auto strMaybe = prefBase->migratePreferenceForThisType(v1, v2, val);
                    if (strMaybe.has_value()) {
                        result[key] = *strMaybe;
                    }
                    continue;
                }
            }

            // try ActionManager::actionsMap()
            {
                const auto it = actionsMap.find(key);
                if (it != actionsMap.end()) {
                    // assume it's a QKeySequence

                    const auto strMaybe = migratePreference<QKeySequence>(v1, v2, val);
                    if (strMaybe.has_value()) {
                        result[key] = *strMaybe;
                    }
                    continue;
                }
            }

            // try Preferences::dynaimcPreferencePatterns()
            {
                bool found = false;
                for (DynamicPreferencePatternBase* dynPref : dynaimcPrefPatterns) {
                    if (matches(key, dynPref->pathPattern())) {
                        found = true;

                        const auto strMaybe = dynPref->migratePreferenceForThisType(v1, v2, val);
                        if (strMaybe.has_value()) {
                            result[key] = *strMaybe;
                        }

                        break;
                    }
                }

                if (found) {
                    continue;
                }
            }
        }

        return result;
    }

    QString v2SettingsPath() {
        return IO::pathAsQString(IO::SystemPaths::userDataDirectory() + IO::Path("Preferences.json"));
    }

    std::map<IO::Path, QString> readV2SettingsFromPath(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {};
        }

        return parseV2SettingsFromJSON(file.readAll());
    }

    bool writeV2SettingsToPath(const QString& path, const std::map<IO::Path, QString>& v2Prefs) {
        const QByteArray serialized = writeV2SettingsToJSON(v2Prefs);

        QSaveFile saveFile(path);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            return false;
        }

        const qint64 written = saveFile.write(serialized);
        if (written != static_cast<qint64>(serialized.size())) {
            return false;
        }

        return saveFile.commit();
    }

    std::map<IO::Path, QString> readV2Settings() {
        const QString path = v2SettingsPath();
        return readV2SettingsFromPath(path);
    }

    std::map<IO::Path, QString> parseV2SettingsFromJSON(const QByteArray& jsonData) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(jsonData, &error);

        if (error.error != QJsonParseError::NoError || !document.isObject()) {
            return {};
        }

        const QJsonObject object = document.object();
        std::map<IO::Path, QString> result;
        for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
            const QString key = it.key();
            const QJsonValue value = it.value();

            result[IO::pathFromQString(key)] = value.toString();
        }
        return result;
    }

    QByteArray writeV2SettingsToJSON(const std::map<IO::Path, QString>& v2Prefs) {
        QJsonObject rootObject;
        for (auto [key, val] : v2Prefs) {
            rootObject[IO::pathAsQString(key, "/")] = val;
        }

        QJsonDocument document(rootObject);
        return document.toJson(QJsonDocument::Indented);
    }

    void migrateSettingsFromV1IfPathDoesNotExist(const QString& destinationPath) {
        // Check if the Preferences.json exists, migrate if not

        QFileInfo prefsFileInfo(destinationPath);
        if (prefsFileInfo.exists()) {
            return;
        }

        const std::map<IO::Path, QString> v2Prefs = migrateV1ToV2(readV1Settings());
        writeV2SettingsToPath(destinationPath, v2Prefs);
    }
}
