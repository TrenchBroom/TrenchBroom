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
#include <QMessageBox>

#include <string>
#include <vector>

namespace TrenchBroom {
    // PreferenceSerializerV1

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, bool* out) const {
        if (!in.isString()) {
            return false;
        }
        const QString inString = in.toString();

        if (inString == QStringLiteral("1")) {
            *out = true;
            return true;
        } else if (inString == QStringLiteral("0")) {
            *out = false;
            return true;
        } else {
            return false;
        }
    }

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, Color* out) const {
        if (!in.isString()) {
            return false;
        }

        if (const auto color = Color::parse(in.toString().toStdString())) {
            *out = *color;
            return true;
        }

        return false;
    }

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, float* out) const {
        if (!in.isString()) {
            return false;
        }
        QString inCopy = in.toString();
        auto inStream = QTextStream(&inCopy);

        inStream >> *out;

        return (inStream.status() == QTextStream::Ok);
    }

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, int* out) const {
        if (!in.isString()) {
            return false;
        }
        QString inCopy = in.toString();
        auto inStream = QTextStream(&inCopy);

        inStream >> *out;

        return (inStream.status() == QTextStream::Ok);
    }

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, IO::Path* out) const {
        if (!in.isString()) {
            return false;
        }
        *out = IO::pathFromQString(in.toString());
        return true;
    }

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, QKeySequence* out) const {
        if (!in.isString()) {
            return false;
        }
        std::optional<QKeySequence> result =
            View::keySequenceFromV1Settings(in.toString());

        if (!result.has_value()) {
            return false;
        }
        *out = result.value();
        return true;
    }

    bool PreferenceSerializerV1::readFromJSON(const QJsonValue& in, QString* out) const {
        if (!in.isString()) {
            return false;
        }
        *out = in.toString();
        return true;
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const bool in) const {
        if (in) {
            return QJsonValue("1");
        } else {
            return QJsonValue("0");
        }
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const Color& in) const {
        // NOTE: QTextStream's default locale is C, unlike QString::arg()
        QString string;
        QTextStream stream(&string);
        stream << in.r() << " "
               << in.g() << " "
               << in.b() << " "
               << in.a();

        return QJsonValue(string);
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const float in) const {
        QString string;
        QTextStream stream(&string);
        stream << in;

        return QJsonValue(string);
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const int in) const {
        QString string;
        QTextStream stream(&string);
        stream << in;

        return QJsonValue(string);
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const IO::Path& in) const {
        QString string;
        QTextStream stream(&string);
        // NOTE: this serializes with "\" separators on Windows and "/" elsewhere!
        stream << IO::pathAsQString(in);

        return QJsonValue(string);
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const QKeySequence& in) const {
        QString string;
        QTextStream stream(&string);
        stream << View::keySequenceToV1Settings(in);

        return QJsonValue(string);
    }

    QJsonValue PreferenceSerializerV1::writeToJSON(const QString& in) const {
        QString string;
        QTextStream stream(&string);
        stream << in;

        return QJsonValue(string);
    }

    // PreferenceSerializerV2

    bool PreferenceSerializerV2::readFromJSON(const QJsonValue& in, bool* out) const {
        if (!in.isBool()) {
            return false;
        }
        *out = in.toBool();
        return true;
    }

    bool PreferenceSerializerV2::readFromJSON(const QJsonValue& in, float* out) const {
        if (!in.isDouble()) {
            return false;
        }
        *out = static_cast<float>(in.toDouble());
        return true;
    }

    bool PreferenceSerializerV2::readFromJSON(const QJsonValue& in, int* out) const {
        if (!in.isDouble()) {
            return false;
        }
        *out = static_cast<int>(in.toDouble());
        return true;
    }

    bool PreferenceSerializerV2::readFromJSON(const QJsonValue& in, QKeySequence* out) const {
        if (!in.isString()) {
            return false;
        }
        *out = QKeySequence(in.toString(), QKeySequence::PortableText);
        return true;
    }

    QJsonValue PreferenceSerializerV2::writeToJSON(const bool in) const {
        return QJsonValue(in);
    }

    QJsonValue PreferenceSerializerV2::writeToJSON(const float in) const {
        return QJsonValue(static_cast<double>(in));
    }

    QJsonValue PreferenceSerializerV2::writeToJSON(const int in) const {
        return QJsonValue(in);
    }

    QJsonValue PreferenceSerializerV2::writeToJSON(const QKeySequence& in) const {
        return QJsonValue(in.toString(QKeySequence::PortableText));
    }

    // PreferenceManager

    void PreferenceManager::markAsUnsaved(PreferenceBase* preference) {
        m_unsavedPreferences.insert(preference);
    }

    PreferenceManager::PreferenceManager()
    : QObject(),
    m_preferencesFilePath(v2SettingsPath()),
    m_fileSystemWatcher(nullptr),
    m_fileReadWriteDisabled(false) {
#if defined __APPLE__
        m_saveInstantly = true;
#else
        m_saveInstantly = false;
#endif

        if (!migrateSettingsFromV1IfPathDoesNotExist(m_preferencesFilePath)) {
            showErrorAndDisableFileReadWrite(tr("An error occurrend while attempting to migrate the preferences to:"), tr("ensure the directory is writable"));
        }

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
        if (m_unsavedPreferences.empty()) {
            return;
        }

        for (auto* pref : m_unsavedPreferences) {
            savePreferenceToCache(pref);
            preferenceDidChangeNotifier(pref->path());
        }
        m_unsavedPreferences.clear();

        if (m_fileReadWriteDisabled) {
            return;
        }

        if (!writeV2SettingsToPath(m_preferencesFilePath, m_cache)) {
            showErrorAndDisableFileReadWrite(tr("An error occurrend while attempting to save the preferences file:"), tr("ensure the directory is writable"));
        }
    }

    void PreferenceManager::discardChanges() {
        m_unsavedPreferences.clear();
        invalidatePreferences();
    }

    static std::vector<IO::Path>
    changedKeysForMapDiff(const std::map<IO::Path, QJsonValue>& before,
                          const std::map<IO::Path, QJsonValue>& after) {
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

    void PreferenceManager::showErrorAndDisableFileReadWrite(const QString& reason, const QString& suggestion) {
        m_fileReadWriteDisabled = true;

        const QString message = tr("%1\n\n"
                                   "%2\n\nPlease correct the problem (%3) and restart TrenchBroom.\n"
                                   "Further settings changes will not be saved this session.")
                                .arg(reason)
                                .arg(m_preferencesFilePath)
                                .arg(suggestion);

        auto dialog = QMessageBox(QMessageBox::Icon::Critical, tr("TrenchBroom"),
                                   message,
                                   QMessageBox::Ok);
        dialog.exec();
    }

    /**
     * Reloads m_cache from the .json file,
     * marks all Preference<T> objects as needing deserialization next time they're accessed, and emits
     * preferenceDidChangeNotifier as needed.
     */
    void PreferenceManager::loadCacheFromDisk() {
        if (m_fileReadWriteDisabled) {
            return;
        }

        const std::map<IO::Path, QJsonValue> oldPrefs = m_cache;

        // Reload m_cache
        readV2SettingsFromPath(m_preferencesFilePath)
            .and_then([&](std::map<IO::Path, QJsonValue>&& prefs) {
                    m_cache = std::move(prefs);
            }).handle_errors(kdl::overload(
                [&] (const PreferenceErrors::FileReadError&) {
                    // This happens e.g. if you don't have read permissions for m_preferencesFilePath
                    showErrorAndDisableFileReadWrite(tr("A file IO error occurred while attempting to read the preference file:"), tr("ensure the file is readable"));
                },
                [&] (const PreferenceErrors::JsonParseError&) {
                    showErrorAndDisableFileReadWrite(tr("A JSON parsing error occurred while reading the preference file:"), tr("fix the JSON, or backup and delete the file"));
                },
                [&] (const PreferenceErrors::NoFilePresent&) {
                    m_cache = {};
                }
            ));

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

        const QJsonValue jsonValue = it->second;
        if (!pref->loadFromJSON(format, jsonValue)) {
            // FIXME: Log to TB console
            const QVariant variantValue = jsonValue.toVariant();
            qDebug() << "Failed to load preference " << IO::pathAsQString(pref->path(), "/")
                     << " from JSON value: " << variantValue.toString() << " (" << variantValue.typeName() << ")";

            pref->resetToDefault();

            // Replace the invalid value in the cache with the default
            savePreferenceToCache(pref);

            // FIXME: trigger writing to disk
        }
        pref->setValid(true);
    }

    void PreferenceManager::savePreferenceToCache(PreferenceBase* pref) {
        if (pref->isDefault()) {
            // Just remove the key/value from the cache if it's already at the default value
            auto it = m_cache.find(pref->path());
            if (it != m_cache.end()) {
                m_cache.erase(it);
            }
            return;
        }

        PreferenceSerializerV2 format;
        const QJsonValue jsonValue = pref->writeToJSON(format);

        m_cache[pref->path()] = jsonValue;
    }

    // helpers

    void togglePref(Preference<bool>& preference) {
        PreferenceManager& prefs = PreferenceManager::instance();
        prefs.set(preference, !prefs.get(preference));
        prefs.saveChanges();
    }

    // V1 settings

    std::map<IO::Path, QJsonValue> parseINI(QTextStream* iniStream) {
        IO::Path section;
        std::map<IO::Path, QJsonValue> result;

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

                result[section + IO::pathFromQString(key)] = QJsonValue(value);
                continue;
            }

            // Line was ignored
        }
        return result;
    }

#if defined(Q_OS_WIN)
    /**
     * Helper for reading the Windows registry QSettings into a std::map<IO::Path, QJsonValue>
     */
    static void visitNode(std::map<IO::Path, QJsonValue>& result, QSettings& settings, const IO::Path& currentPath) {
        // Process key/value pairs at this node
        for (const QString& key : settings.childKeys()) {
            const QString value = settings.value(key).toString();
            const IO::Path keyPath = currentPath + IO::pathFromQString(key);
            result[keyPath] = QJsonValue(value);
        }

        // Vist children
        for (const QString& childGroup : settings.childGroups()) {
            settings.beginGroup(childGroup);
            visitNode(result, settings, currentPath + IO::pathFromQString(childGroup));
            settings.endGroup();
        }
    }

    static std::map<IO::Path, QJsonValue> getRegistrySettingsV1() {
        std::map<IO::Path, QJsonValue> result;

        QSettings settings("HKEY_CURRENT_USER\\Software\\Kristian Duske\\TrenchBroom", QSettings::Registry32Format);
        visitNode(result, settings, IO::Path());

        return result;
    }
#endif

    std::map<IO::Path, QJsonValue> getINISettingsV1(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {};
        }

        QTextStream in(&file);
        const std::map<IO::Path, QJsonValue> result = parseINI(&in);

        return result;
    }

    std::map<IO::Path, QJsonValue> readV1Settings() {
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

    std::map<IO::Path, QJsonValue> migrateV1ToV2(const std::map<IO::Path, QJsonValue>& v1Prefs) {
        auto& map = Preferences::staticPreferencesMap();
        auto& actionsMap = View::ActionManager::instance().actionsMap();
        auto& dynaimcPrefPatterns = Preferences::dynaimcPreferencePatterns();

        const PreferenceSerializerV1 v1;
        const PreferenceSerializerV2 v2;

        std::map<IO::Path, QJsonValue> result;

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

                        const std::optional<QJsonValue> strMaybe = dynPref->migratePreferenceForThisType(v1, v2, val);
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

    PreferencesResult readV2SettingsFromPath(const QString& path) {
        QFile file(path);
        if (!file.exists()) {
            return PreferenceErrors::NoFilePresent{};
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return PreferenceErrors::FileReadError{};
        }

        return parseV2SettingsFromJSON(file.readAll());
    }

    bool writeV2SettingsToPath(const QString& path, const std::map<IO::Path, QJsonValue>& v2Prefs) {
        const QByteArray serialized = writeV2SettingsToJSON(v2Prefs);

        const QString settingsDir = QFileInfo(path).path();
        if (!QDir().mkpath(settingsDir)) {
            return false;
        }

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

    PreferencesResult readV2Settings() {
        const QString path = v2SettingsPath();
        return readV2SettingsFromPath(path);
    }

    PreferencesResult parseV2SettingsFromJSON(const QByteArray& jsonData) {
        auto error = QJsonParseError();
        const QJsonDocument document = QJsonDocument::fromJson(jsonData, &error);

        if (error.error != QJsonParseError::NoError || !document.isObject()) {
            return PreferenceErrors::JsonParseError{error};
        }

        const QJsonObject object = document.object();
        std::map<IO::Path, QJsonValue> result;
        for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
            const QString key = it.key();
            const QJsonValue value = it.value();

            result[IO::pathFromQString(key)] = value;
        }
        return result;
    }

    QByteArray writeV2SettingsToJSON(const std::map<IO::Path, QJsonValue>& v2Prefs) {
        QJsonObject rootObject;
        for (auto [key, val] : v2Prefs) {
            rootObject[IO::pathAsQString(key, "/")] = val;
        }

        QJsonDocument document(rootObject);
        return document.toJson(QJsonDocument::Indented);
    }

    bool migrateSettingsFromV1IfPathDoesNotExist(const QString& destinationPath) {
        // Check if the Preferences.json exists, migrate if not

        QFileInfo prefsFileInfo(destinationPath);
        if (prefsFileInfo.exists()) {
            return true;
        }

        const std::map<IO::Path, QJsonValue> v2Prefs = migrateV1ToV2(readV1Settings());

        return writeV2SettingsToPath(destinationPath, v2Prefs);
    }
}
