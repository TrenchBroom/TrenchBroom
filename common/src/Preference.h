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

#pragma once

#include "Macros.h"
#include "IO/Path.h"
#include "View/KeyboardShortcut.h"

#include <optional>

#include <QString>
#include <QTextStream>
#include <QJsonValue>

class QKeySequence;

namespace TrenchBroom {
    class Color;

    class PrefSerializer {
    public:
        virtual ~PrefSerializer();

        virtual bool readFromJSON(const QJsonValue& in, bool* out) const = 0;
        virtual bool readFromJSON(const QJsonValue& in, Color* out) const = 0;
        virtual bool readFromJSON(const QJsonValue& in, float* out) const = 0;
        virtual bool readFromJSON(const QJsonValue& in, int* out) const = 0;
        virtual bool readFromJSON(const QJsonValue& in, IO::Path* out) const = 0;
        virtual bool readFromJSON(const QJsonValue& in, QKeySequence* out) const = 0;
        virtual bool readFromJSON(const QJsonValue& in, QString* out) const = 0;

        virtual QJsonValue writeToJSON(bool in) const = 0;
        virtual QJsonValue writeToJSON(const Color& in) const = 0;
        virtual QJsonValue writeToJSON(float in) const = 0;
        virtual QJsonValue writeToJSON(int in) const = 0;
        virtual QJsonValue writeToJSON(const IO::Path& in) const = 0;
        virtual QJsonValue writeToJSON(const QKeySequence& in) const = 0;
        virtual QJsonValue writeToJSON(const QString& in) const = 0;
    };

    template <class T>
    std::optional<QJsonValue> migratePreference(const PrefSerializer& from, const PrefSerializer& to, const QJsonValue& input) {
        T result;
        if (!from.readFromJSON(input, &result)) {
            return {};
        }

        return {to.writeToJSON(result)};
    }

    class PreferenceBase {
    public:
        PreferenceBase() = default;
        virtual ~PreferenceBase();

        PreferenceBase(const PreferenceBase& other) = default;
        PreferenceBase(PreferenceBase&& other) noexcept = default;
        PreferenceBase& operator=(const PreferenceBase& other) = default;
        PreferenceBase& operator=(PreferenceBase&& other) = default;

        bool operator==(const PreferenceBase& other) const {
            return this == &other;
        }

        virtual const IO::Path& path() const = 0;
    public: // private to PreferenceManager
        virtual void resetToDefault() = 0;
        virtual bool valid() const = 0;
        virtual void setValid(bool _valid) = 0;
        virtual std::optional<QJsonValue> migratePreferenceForThisType(const PrefSerializer& from,
            const PrefSerializer& to, const QJsonValue& input) const = 0;
        virtual bool loadFromJSON(const PrefSerializer& format, const QJsonValue& value) = 0;
        virtual QJsonValue writeToJSON(const PrefSerializer& format) const = 0;
        virtual bool isDefault() const = 0;
    };

    class DynamicPreferencePatternBase {
    public:
        virtual ~DynamicPreferencePatternBase();
        virtual const IO::Path& pathPattern() const = 0;
        virtual std::optional<QJsonValue> migratePreferenceForThisType(const PrefSerializer& from, const PrefSerializer& to, const QJsonValue& input) const = 0;
    };

    template <typename T>
    class DynamicPreferencePattern : public DynamicPreferencePatternBase {
    private:
        IO::Path m_pathPattern;
    public:
        explicit DynamicPreferencePattern(const IO::Path& pathPattern) :
        m_pathPattern(pathPattern) {}

        const IO::Path& pathPattern() const override {
            return m_pathPattern;
        }

        std::optional<QJsonValue> migratePreferenceForThisType(const PrefSerializer& from, const PrefSerializer& to, const QJsonValue& input) const override {
            return migratePreference<T>(from, to, input);
        }
    };

    /**
     * Stores the current value and default value of a preference, in deserialized form.
     * No public API for reading/writing the value, use PreferenceManager instead.
     */
    template <typename T>
    class Preference : public PreferenceBase {
    private:
        IO::Path m_path;
        T m_defaultValue;
        T m_value;
        bool m_valid;
    public:
        Preference(const IO::Path& path, const T& defaultValue) :
        m_path(path),
        m_defaultValue(defaultValue),
        m_value(m_defaultValue),
        m_valid(false) {}

        Preference(const Preference& other) = default;
        Preference(Preference&& other) = default; // cannot be noexcept because it will call QKeySequence's copy constructor

        Preference& operator=(const Preference& other) = default;
        Preference& operator=(Preference&& other) = default;

        const IO::Path& path() const override {
            return m_path;
        }

        const T& defaultValue() const {
            return m_defaultValue;
        }

    public: // PreferenceManager private
        void setValue(const T& value) {
            m_value = value;
        }

        void resetToDefault() override {
            m_value = m_defaultValue;
        }

        bool valid() const override {
            return m_valid;
        }

        void setValid(const bool _valid) override {
            m_valid = _valid;
        }

        const T& value() const {
            assert(m_valid);
            return m_value;
        }

        std::optional<QJsonValue> migratePreferenceForThisType(const PrefSerializer& from, const PrefSerializer& to, const QJsonValue& input) const override {
            return migratePreference<T>(from, to, input);
        }

        bool loadFromJSON(const PrefSerializer& format, const QJsonValue& value) override {
            T res;
            bool ok = format.readFromJSON(value, &res);
            if (ok) {
                m_value = res;
            }
            return ok;
        }

        QJsonValue writeToJSON(const PrefSerializer& format) const override {
            return format.writeToJSON(value());
        }

        bool isDefault() const override {
            return m_defaultValue == m_value;
        }
    };
}

#endif /* defined(TrenchBroom_Preference) */
