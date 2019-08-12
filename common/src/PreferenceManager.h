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

#include "Color.h"
#include "Notifier.h"
#include "Preference.h"
#include "StringUtils.h"
#include "View/KeyboardShortcut.h"

#include <QApplication>
#include <QThread>
#include <QString>
#include <QByteArray>

#include <map>
#include <memory>
#include <set>

class QTextStream;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    /**
     * Used by wxWidgets versions of TB
     */
    class PreferenceSerializerV1 : public PrefSerializer {
    public:
        bool readFromString(const QString& in, bool* out) const override;
        bool readFromString(const QString& in, Color* out) const override;
        bool readFromString(const QString& in, float* out) const override;
        bool readFromString(const QString& in, int* out) const override;
        bool readFromString(const QString& in, IO::Path* out) const override;
        bool readFromString(const QString& in, View::KeyboardShortcut* out) const override;
        bool readFromString(const QString& in, QString* out) const override;

        void writeToString(QTextStream& stream, const bool in) const override;
        void writeToString(QTextStream& stream, const Color& in) const override;
        void writeToString(QTextStream& stream, const float in) const override;
        void writeToString(QTextStream& stream, const int in) const override;
        void writeToString(QTextStream& stream, const IO::Path& in) const override;
        void writeToString(QTextStream& stream, const View::KeyboardShortcut& in) const override;
        void writeToString(QTextStream& stream, const QString& in) const override;
    };

    class PreferenceSerializerV2 : public PreferenceSerializerV1 {
    public:
        bool readFromString(const QString& in, View::KeyboardShortcut* out) const override;
        void writeToString(QTextStream& stream, const View::KeyboardShortcut& in) const override;
    };

    class PreferenceManager {
    private:
        using UnsavedPreferences = std::set<PreferenceBase*>;
        using DynamicPreferences = std::map<IO::Path, std::unique_ptr<PreferenceBase>>;

        bool m_saveInstantly;
        UnsavedPreferences m_unsavedPreferences;
        DynamicPreferences m_dynamicPreferences;

        void markAsUnsaved(PreferenceBase* preference);
    public:
        static PreferenceManager& instance();

        Notifier<const IO::Path&> preferenceDidChangeNotifier;

        bool saveInstantly() const;
        PreferenceBase::Set saveChanges();
        PreferenceBase::Set discardChanges();

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
            ensure(pref != nullptr, "Preference " + path.asString() + " must be of the expected type");
            return *pref;
        }

        template <typename T>
        const T& get(const Preference<T>& preference) const {
            ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

            // Only load from disk the first time it's accessed
            if (!preference.initialized()) {
                preference.load();
            }

            return preference.value();
        }

        template <typename T>
        bool set(Preference<T>& preference, const T& value) {
            ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

            const T previousValue = preference.value();
            if (previousValue == value) {
                return false;
            }

            preference.setValue(value);
            if (saveInstantly()) {
                preference.save();
                preferenceDidChangeNotifier(preference.path());
            } else {
                markAsUnsaved(&preference);
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
    const T& pref(const Preference<T>& preference) {
        const PreferenceManager& prefs = PreferenceManager::instance();
        return prefs.get(preference);
    }

    std::map<IO::Path, QString> parseINI(QTextStream* iniStream);
    std::map<IO::Path, QString> getINISettingsV1(const QString& path);
    std::map<IO::Path, QString> readV1Settings();
    std::map<IO::Path, QString> migrateV1ToV2(const std::map<IO::Path, QString>& v1Prefs);

    // V2 settings
    QString v2SettingsPath();
    std::map<IO::Path, QString> readV2SettingsFromPath(const QString& path);
    std::map<IO::Path, QString> readV2Settings();

    bool writeV2SettingsToPath(const QString& path, const std::map<IO::Path, QString>& v2Prefs);

    std::map<IO::Path, QString> parseV2SettingsFromJSON(const QByteArray& jsonData);
    QByteArray writeV2SettingsToJSON(const std::map<IO::Path, QString>& v2Prefs);
}

#endif /* defined(TrenchBroom_PreferenceManager) */
