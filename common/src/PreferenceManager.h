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

#ifndef TrenchBroom_PreferenceManager
#define TrenchBroom_PreferenceManager

#include "Ensure.h"
#include "Notifier.h"
#include "Preference.h"

#include <kdl/vector_set.h>
#include <kdl/result.h>

#include <map>
#include <memory>
#include <vector>

#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QThread>
#include <QJsonParseError>

class QTextStream;
class QFileSystemWatcher;

namespace TrenchBroom {
    class Color;

    /**
     * Used by wxWidgets versions of TB
     */
    class PreferenceSerializerV1 : public PrefSerializer {
    public:
        bool readFromJSON(const QJsonValue& in, bool* out) const override;
        bool readFromJSON(const QJsonValue& in, Color* out) const override;
        bool readFromJSON(const QJsonValue& in, float* out) const override;
        bool readFromJSON(const QJsonValue& in, int* out) const override;
        bool readFromJSON(const QJsonValue& in, IO::Path* out) const override;
        bool readFromJSON(const QJsonValue& in, QKeySequence* out) const override;
        bool readFromJSON(const QJsonValue& in, QString* out) const override;

        QJsonValue writeToJSON(bool in) const override;
        QJsonValue writeToJSON(const Color& in) const override;
        QJsonValue writeToJSON(float in) const override;
        QJsonValue writeToJSON(int in) const override;
        QJsonValue writeToJSON(const IO::Path& in) const override;
        QJsonValue writeToJSON(const QKeySequence& in) const override;
        QJsonValue writeToJSON(const QString& in) const override;
    };

    /**
     * Used by Qt version of TrenchBroom
     *
     * - bool serializes to JSON bool
     * - float and int serializes to JSON double
     * - QKeySequence serializes to JSON string, but with a different format than wxWidgets
     * - other types are not overridden (Color, IO::Path, QString) so serialize to JSON string using
     *   the same format as wxWidgets
     */
    class PreferenceSerializerV2 : public PreferenceSerializerV1 {
    public:
        bool readFromJSON(const QJsonValue& in, bool* out) const override;
        bool readFromJSON(const QJsonValue& in, float* out) const override;
        bool readFromJSON(const QJsonValue& in, int* out) const override;
        bool readFromJSON(const QJsonValue& in, QKeySequence* out) const override;        

        QJsonValue writeToJSON(bool in) const override;
        QJsonValue writeToJSON(float in) const override;
        QJsonValue writeToJSON(int in) const override;
        QJsonValue writeToJSON(const QKeySequence& in) const override;
    };

    class PreferenceManager : public QObject {
        Q_OBJECT
    private:
        using UnsavedPreferences = kdl::vector_set<PreferenceBase*>;
        using DynamicPreferences = std::map<IO::Path, std::unique_ptr<PreferenceBase>>;

        QString m_preferencesFilePath;
        bool m_saveInstantly;
        UnsavedPreferences m_unsavedPreferences;
        DynamicPreferences m_dynamicPreferences;
        /**
         * This should always be in sync with what is on disk.
         * Preference objects may have different values if there are unsaved changes.
         * There may also be values in here we don't know how to deserialize; we write them back to disk.
         */
        std::map<IO::Path, QJsonValue> m_cache;
        QFileSystemWatcher* m_fileSystemWatcher;
        /**
         * If true, don't try to read/write preferences anymore.
         * This gets set to true if there is a JSON parse error, so
         * we don't clobber the file if the user makes a mistake while editing it by hand.
         */
        bool m_fileReadWriteDisabled;

        void markAsUnsaved(PreferenceBase* preference);
    public:
        static PreferenceManager& instance();

        Notifier<const IO::Path&> preferenceDidChangeNotifier;

        bool saveInstantly() const;
        void saveChanges();
        void discardChanges();
    private:
        void showErrorAndDisableFileReadWrite(const QString& reason, const QString& suggestion);
        void loadCacheFromDisk();
        void invalidatePreferences();
        void loadPreferenceFromCache(PreferenceBase* pref);
        void savePreferenceToCache(PreferenceBase* pref);
    public:
        template <typename T>
        Preference<T>& dynamicPreference(const IO::Path& path, T&& defaultValue) {
            auto it = m_dynamicPreferences.find(path);
            if (it == std::end(m_dynamicPreferences)) {
                bool success = false;
                std::tie(it, success) = m_dynamicPreferences.emplace(path, std::make_unique<Preference<T>>(path, std::forward<T>(defaultValue)));
                assert(success); unused(success);
            }

            const auto& prefPtr = it->second;
            auto* prefBase = prefPtr.get();
            auto* pref = dynamic_cast<Preference<T>*>(prefBase);
            ensure(pref != nullptr, ("Preference " + path.asString() + " must be of the expected type").c_str());
            return *pref;
        }

        /**
         * Public API for getting the value of a preference.
         */
        template <typename T>
        const T& get(Preference<T>& preference) {
            ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

            // Only load from disk the first time it's accessed
            if (!preference.valid()) {
                loadPreferenceFromCache(&preference);
            }

            return preference.value();
        }

        /**
         * Public API for setting the value of a preference.
         */
        template <typename T>
        bool set(Preference<T>& preference, const T& value) {
            ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

            const T previousValue = get(preference);
            if (previousValue == value) {
                return false;
            }

            preference.setValue(value);
            preference.setValid(true);
            markAsUnsaved(&preference);

            if (saveInstantly()) {
                saveChanges();
                preferenceDidChangeNotifier(preference.path());
            }

            return true;
        }

        template <typename T>
        void resetToDefault(Preference<T>& preference) {
            set(preference, preference.defaultValue());
        }
    private:
        PreferenceManager();
        deleteCopyAndMove(PreferenceManager)
    };

    template <typename T>
    const T& pref(Preference<T>& preference) {
        PreferenceManager& prefs = PreferenceManager::instance();
        return prefs.get(preference);
    }

    namespace PreferenceErrors {
        struct NoFilePresent  {};
        struct JsonParseError { QJsonParseError jsonError; };
        struct FileReadError  {};
    }

    using PreferencesResult = kdl::result<std::map<IO::Path, QJsonValue>, // Success case
                                          PreferenceErrors::NoFilePresent,
                                          PreferenceErrors::JsonParseError,
                                          PreferenceErrors::FileReadError>;
    
    // V1 settings
    std::map<IO::Path, QJsonValue> parseINI(QTextStream* iniStream);
    std::map<IO::Path, QJsonValue> getINISettingsV1(const QString& path);
    std::map<IO::Path, QJsonValue> readV1Settings();
    std::map<IO::Path, QJsonValue> migrateV1ToV2(const std::map<IO::Path, QJsonValue>& v1Prefs);

    // V2 settings
    QString v2SettingsPath();
    PreferencesResult readV2SettingsFromPath(const QString& path);
    PreferencesResult readV2Settings();

    bool writeV2SettingsToPath(const QString& path, const std::map<IO::Path, QJsonValue>& v2Prefs);
    PreferencesResult parseV2SettingsFromJSON(const QByteArray& jsonData);
    QByteArray writeV2SettingsToJSON(const std::map<IO::Path, QJsonValue>& v2Prefs);

    // Migration
    bool migrateSettingsFromV1IfPathDoesNotExist(const QString& destinationPath);
}

#endif /* defined(TrenchBroom_PreferenceManager) */
