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

#include <map>
#include <memory>
#include <set>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

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

        Notifier1<const IO::Path&> preferenceDidChangeNotifier;

        bool saveInstantly() const;
        PreferenceBase::Set saveChanges();
        PreferenceBase::Set discardChanges();

        template <typename T>
        Preference<T>& dynamicPreference(const IO::Path& path, T&& defaultValue) {
            const auto [found, pos] = MapUtils::findInsertPos(m_dynamicPreferences, path);
            if (found) {
                // this is potentially unsafe if T doesn't match the existing preference's type argument
                return static_cast<Preference<T>&>(*(pos->second));
            } else {
                const auto it = m_dynamicPreferences.emplace_hint(
                    std::prev(pos),
                    std::make_pair(path, std::make_unique<Preference<T>>(path, defaultValue)));
                return static_cast<Preference<T>&>(*it->second);
            }
        }

        template <typename T>
        const T& get(const Preference<T>& preference) const {
            ensure(wxThread::IsMain(), "PreferenceManager can only be used on the main thread");

            if (!preference.initialized()) {
                preference.load(wxConfig::Get());
            }

            return preference.value();
        }

        template <typename T>
        const T& getDefault(const Preference<T>& preference) const {
            return preference.defaultValue();
        }

        template <typename T>
        bool set(Preference<T>& preference, const T& value) {
            ensure(wxThread::IsMain(), "PreferenceManager can only be used on the main thread");

            const T previousValue = preference.value();
            if (previousValue == value)
                return false;

            preference.setValue(value);
            if (saveInstantly()) {
                preference.save(wxConfig::Get());
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
        PreferenceManager(const PreferenceManager&);
        PreferenceManager& operator=(const PreferenceManager&);
    };

    template <typename T>
    const T& pref(const Preference<T>& preference) {
        const PreferenceManager& prefs = PreferenceManager::instance();
        return prefs.get(preference);
    }

    template <typename T>
    class SetTemporaryPreference {
    private:
        Preference<T>& m_pref;
        T m_oldValue;
    public:
        SetTemporaryPreference(Preference<T>& pref, const T& newValue) :
        m_pref(pref),
        m_oldValue(pref.value()) {
            m_pref.setValue(newValue);
        }

        ~SetTemporaryPreference() {
            m_pref.setValue(m_oldValue);
        }
    };
}

#endif /* defined(TrenchBroom_PreferenceManager) */
