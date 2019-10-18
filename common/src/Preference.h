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

#ifndef TrenchBroom_Preference
#define TrenchBroom_Preference

#include "Color.h"
#include "Exceptions.h"
#include "Macros.h"
#include "StringType.h"
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "View/KeyboardShortcut.h"
#include "View/wxUtils.h"

#include <memory>

#include <QApplication>
#include <QSettings>
#include <QThread>
#include <QDebug>
#include <QVariant>
#include <QKeySequence>

#include <optional-lite/optional.hpp>

class QString;
class QTextStream;
class QKeySequence;

namespace TrenchBroom {
    class PrefSerializer {
    public:
        virtual ~PrefSerializer() = default;

        virtual bool readFromString(const QString& in, bool* out) const = 0;
        virtual bool readFromString(const QString& in, Color* out) const = 0;
        virtual bool readFromString(const QString& in, float* out) const = 0;
        virtual bool readFromString(const QString& in, int* out) const = 0;
        virtual bool readFromString(const QString& in, IO::Path* out) const = 0;
        virtual bool readFromString(const QString& in, QKeySequence* out) const = 0;
        virtual bool readFromString(const QString& in, QString* out) const = 0;

        virtual void writeToString(QTextStream& stream, bool in) const = 0;
        virtual void writeToString(QTextStream& stream, const Color& in) const = 0;
        virtual void writeToString(QTextStream& stream, float in) const = 0;
        virtual void writeToString(QTextStream& stream, int in) const = 0;
        virtual void writeToString(QTextStream& stream, const IO::Path& in) const = 0;
        virtual void writeToString(QTextStream& stream, const QKeySequence& in) const = 0;
        virtual void writeToString(QTextStream& stream, const QString& in) const = 0;
    };

    template <class T>
    nonstd::optional<QString> migratePreference(const PrefSerializer& from, const PrefSerializer& to, const QString& input) {
        T result;
        if (!from.readFromString(input, &result)) {
            return {};
        }

        QString string;
        QTextStream stream(&string);
        to.writeToString(stream, result);
        return {string};
    }

    class PreferenceBase {
    public:
        PreferenceBase() = default;
        virtual ~PreferenceBase() = default;

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
        virtual nonstd::optional<QString> migratePreferenceForThisType(const PrefSerializer& from,
            const PrefSerializer& to, const QString& input) const = 0;
        virtual bool loadFromString(const PrefSerializer& format, const QString& value) = 0;
        virtual QString writeToString(const PrefSerializer& format) const = 0;
    };

    class DynamicPreferencePatternBase {
    public:
        virtual ~DynamicPreferencePatternBase() = default;
        virtual const IO::Path& pathPattern() const = 0;
        virtual nonstd::optional<QString> migratePreferenceForThisType(const PrefSerializer& from, const PrefSerializer& to, const QString& input) const = 0;
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

        nonstd::optional<QString> migratePreferenceForThisType(const PrefSerializer& from, const PrefSerializer& to, const QString& input) const override {
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

        nonstd::optional<QString> migratePreferenceForThisType(const PrefSerializer& from, const PrefSerializer& to, const QString& input) const override {
            return migratePreference<T>(from, to, input);
        }

        bool loadFromString(const PrefSerializer& format, const QString& value) override {
            T res;
            bool ok = format.readFromString(value, &res);
            if (ok) {
                m_value = res;
            }
            return ok;
        }

        QString writeToString(const PrefSerializer& format) const override {
            QString result;
            QTextStream stream(&result);

            format.writeToString(stream, value());

            return result;
        }
    };
}

#endif /* defined(TrenchBroom_Preference) */
