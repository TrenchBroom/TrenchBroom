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
#include "StringUtils.h"
#include "IO/Path.h"
#include "View/KeyboardShortcut.h"
#include "View/wxUtils.h"

#include <memory>

#include <QApplication>
#include <QSettings>
#include <QThread>
#include <QDebug>
#include <QVariant>

namespace TrenchBroom {
    template <typename T>
    class PreferenceSerializer {
    public:
        bool read(QSettings* config, const QString& path, T& result) const { return false; }
        void write(QSettings* config, const QString& path, const T& value) const {}
    };

    template <>
    class PreferenceSerializer<bool> {
    public:
        bool read(QSettings* config, const QString& path, bool& result) const {
            const QVariant value = config->value(path);
            if (!value.isValid()) {
                return false;
            }
            result = value.toBool();
            return true;
        }

        void write(QSettings* config, const QString& path, const bool& value) const {
            config->setValue(path, QVariant(value));
        }
    };

    template <>
    class PreferenceSerializer<int> {
    public:
        bool read(QSettings* config, const QString& path, int& result) const {
            const QVariant value = config->value(path);
            bool ok = false;
            result = value.toInt(&ok);
            return ok;
        }

        void write(QSettings* config, const QString& path, const int& value) const {
            config->setValue(path, QVariant(value));
        }
    };

    template <>
    class PreferenceSerializer<float> {
    public:
        bool read(QSettings* config, const QString& path, float& result) const {
            const QVariant value = config->value(path);
            bool ok = false;
            result = value.toFloat(&ok);
            return ok;
        }

        void write(QSettings* config, const QString& path, const float& value) const {
            config->setValue(path, QVariant(value));
        }
    };

    template <>
    class PreferenceSerializer<double> {
    public:
        bool read(QSettings* config, const QString& path, double& result) const {
            const QVariant value = config->value(path);
            bool ok = false;
            result = value.toDouble(&ok);
            return ok;
        }

        void write(QSettings* config, const QString& path, const double& value) const {
            config->setValue(path, QVariant(value));
        }
    };

    template <>
    class PreferenceSerializer<String> {
    public:
        bool read(QSettings* config, const QString& path, String& result) const {
            const QVariant value = config->value(path);
            if (!value.isValid()) {
                return false;
            }
            result = value.toString().toStdString();
            return true;
        }

        void write(QSettings* config, const QString& path, const String& value) const {
            config->setValue(path, QVariant(QString::fromStdString(value)));
        }
    };

    template <>
    class PreferenceSerializer<Color> {
    public:
        bool read(QSettings* config, const QString& path, Color& result) const {
            const QVariant value = config->value(path);
            if (!value.isValid()) {
                return false;
            }
            result = Color::parse(value.toString().toStdString());
            return true;
        }

        void write(QSettings* config, const QString& path, const Color& value) const {
            config->setValue(path, QVariant(QString::fromStdString(StringUtils::toString(value))));
        }
    };

    template<>
    class PreferenceSerializer<View::KeyboardShortcut> {
    public:
        bool read(QSettings* config, const QString& path, View::KeyboardShortcut& result) const {
            const QVariant value = config->value(path);
            if (!value.isValid()) {
                return false;
            }
            // FIXME: Parse the old wxWidgets format too
            const auto keySequence = QKeySequence::fromString(value.toString(), QKeySequence::PortableText);
            result = View::KeyboardShortcut(keySequence);
            return true;
        }

        void write(QSettings* config, const QString& path, const View::KeyboardShortcut& value) const {
            const auto keySequence = value.keySequence();
            config->setValue(path, QVariant(keySequence.toString(QKeySequence::PortableText)));
        }
    };

    template<>
    class PreferenceSerializer<IO::Path> {
    public:
        bool read(QSettings* config, const QString& path, IO::Path& result) const {
            const QVariant value = config->value(path);
            if (!value.isValid()) {
                return false;
            }
            result = IO::Path(value.toString().toStdString());
            return true;
        }

        void write(QSettings* config, const QString& path, const IO::Path& value) const {
            config->setValue(path, QVariant(QString::fromStdString(value.asString())));
        }
    };

    class PreferenceBase {
    public:
        using Set = std::set<const PreferenceBase*>;
        PreferenceBase() = default;
        virtual ~PreferenceBase() = default;

        PreferenceBase(const PreferenceBase& other) = default;
        PreferenceBase(PreferenceBase&& other) noexcept = default;
        PreferenceBase& operator=(const PreferenceBase& other) = default;
        PreferenceBase& operator=(PreferenceBase&& other) = default;

        virtual void load() const = 0;
        virtual void save() = 0;
        virtual void resetToPrevious() = 0;

        bool operator==(const PreferenceBase& other) const {
            return this == &other;
        }

        virtual const IO::Path& path() const = 0;
    };


    template <typename T>
    class Preference : public PreferenceBase {
    protected:
        friend class PreferenceManager;
        template<typename> friend class SetTemporaryPreference;

        PreferenceSerializer<T> m_serializer;
        IO::Path m_path;
        T m_defaultValue;
        mutable T m_value;
        mutable T m_previousValue;
        mutable bool m_initialized;
        bool m_modified;

        void setValue(const T& value) {
            if (!m_modified) {
                m_modified = true;
                m_previousValue = m_value;
            }
            m_value = value;
        }

        bool initialized() const {
            return m_initialized;
        }

        static QSettings getSettings() {
            return View::getSettings();
        }

        void load() const override {
            ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

            QSettings settings = getSettings();

            static bool tested = false;
            if (!tested) {
                tested = true;

                for (auto s : settings.allKeys()) {
                    std::cout << s.toStdString() << " is " << settings.value(s).toString().toStdString() << "\n";
                }

            }

            using std::swap;
            T temp;
            if (m_serializer.read(&settings, m_path.asQString('/'), temp)) {
                std::swap(m_value, temp);
                m_previousValue = m_value;
            }
            m_initialized = true;
        }

        void save() override {
            ensure(qApp->thread() == QThread::currentThread(), "PreferenceManager can only be used on the main thread");

            if (m_modified) {
                QSettings settings = getSettings();

                m_serializer.write(&settings, m_path.asQString('/'), m_value);
                m_modified = false;
                m_previousValue = m_value;
            }
        }

        void resetToPrevious() override {
            if (m_modified) {
                m_value = m_previousValue;
                m_modified = false;
            }
        }
    public:
        Preference(const IO::Path& path, const T& defaultValue) :
        m_path(path),
        m_defaultValue(defaultValue),
        m_value(m_defaultValue),
        m_previousValue(m_value),
        m_initialized(false),
        m_modified(false) {
            m_modified = m_initialized;
        }

        Preference(const Preference& other) = default;
        Preference(Preference&& other) noexcept = default;

        Preference& operator=(const Preference& other) = default;
        Preference& operator=(Preference&& other) = default;

        const IO::Path& path() const override {
            return m_path;
        }

        const T& defaultValue() const {
            return m_defaultValue;
        }

        const T& value() const {
            return m_value;
        }
    };
}

#endif /* defined(TrenchBroom_Preference) */
