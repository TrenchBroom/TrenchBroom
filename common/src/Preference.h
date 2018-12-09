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

#include <memory>

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/tokenzr.h>
#include <wx/thread.h>

namespace TrenchBroom {
    template <typename T>
    class PreferenceSerializer {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, T& result) const { return false; }
        bool write(wxConfigBase* config, const IO::Path& path, const T& value) const { return false; }
    };
    
    template <>
    class PreferenceSerializer<bool> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, bool& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                long longValue = 0;
                if (string.ToLong(&longValue)) {
                    result = longValue != 0L;
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const bool& value) const {
            wxString str;
            str << (value ? 1 : 0);
            return config->Write(path.asString('/'), str);
        }
    };
    
    template <>
    class PreferenceSerializer<int> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, int& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                long longValue = 0;
                if (string.ToLong(&longValue) && longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max()) {
                    result = static_cast<int>(longValue);
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const int& value) const {
            wxString str;
            str << value;
            return config->Write(path.asString('/'), str);
        }
    };
    
    template <>
    class PreferenceSerializer<float> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, float& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                double doubleValue = 0.0;
                if (string.ToDouble(&doubleValue) && doubleValue >= std::numeric_limits<float>::min() && doubleValue <= std::numeric_limits<float>::max()) {
                    result = static_cast<float>(doubleValue);
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const float& value) const {
            wxString str;
            str << value;
            return config->Write(path.asString('/'), str);
        }
    };
    
    template <>
    class PreferenceSerializer<double> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, double& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                double doubleValue = 0.0;
                if (string.ToDouble(&doubleValue)) {
                    result = doubleValue;
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const double& value) const {
            wxString str;
            str << value;
            return config->Write(path.asString('/'), str);
        }
    };
    
    template <>
    class PreferenceSerializer<String> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, String& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                result = string.ToStdString();
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const String& value) const {
            return config->Write(path.asString('/'), wxString(value));
        }
    };
    
    template <>
    class PreferenceSerializer<Color> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, Color& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                result = Color::parse(string.ToStdString());
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const Color& value) const {
            return config->Write(path.asString('/'), wxString(StringUtils::toString(value)));
        }
    };
    
    template<>
    class PreferenceSerializer<View::KeyboardShortcut> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, View::KeyboardShortcut& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                result = View::KeyboardShortcut(string);
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const View::KeyboardShortcut& value) const {
            return config->Write(path.asString('/'), wxString(value.asString()));
        }
    };
    
    template<>
    class PreferenceSerializer<IO::Path> {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, IO::Path& result) const {
            wxString string;
            if (config->Read(path.asString('/'), &string)) {
                result = IO::Path(string.ToStdString());
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const IO::Path& value) const {
            return config->Write(path.asString('/'), wxString(value.asString()));
        }
    };

    template<typename S>
    class PreferenceSerializer<std::vector<S> > {
    private:
        PreferenceSerializer<S> m_serializer;
    public:
        bool read(wxConfigBase* config, const IO::Path& path, std::vector<S>& result) const {
            const wxString wxPath(path.asString('/'));
            if (!config->Exists(wxPath))
                return false;
            
            const wxString oldPath = config->GetPath();
            config->SetPath(wxPath);

            bool success = true;
            std::vector<S> temp;

            wxString name;
            long index;
            if (config->GetFirstEntry(name, index)) {
                do {
                    S value;
                    success = m_serializer.read(config, IO::Path(name.ToStdString()), value);
                    if (success)
                        temp.push_back(value);
                } while (success && config->GetNextEntry(name, index));
            }
            
            config->SetPath(oldPath);
            
            using std::swap;
            if (success)
                swap(result, temp);
            return success;
        }

        bool write(wxConfigBase* config, const IO::Path& path, const std::vector<S>& values) const {
            const wxString oldPath = config->GetPath();
            config->DeleteGroup(path.asString('/'));
            config->SetPath(path.asString('/'));

            for (size_t i = 0; i < values.size(); ++i) {
                wxString name;
                name << i;
                m_serializer.write(config, IO::Path(name.ToStdString()), values[i]);
            }
            
            config->SetPath(oldPath);
            return true;
        }
    };
    
    template<typename S>
    class PreferenceSerializer<std::map<String, S> > {
    private:
        PreferenceSerializer<S> m_serializer;
    public:
        bool read(wxConfigBase* config, const IO::Path& path, std::map<String, S>& result) const {
            const wxString oldPath = config->GetPath();
            config->SetPath(path.asString('/'));
            
            bool success = true;
            std::map<String, S> temp;
            
            wxString name;
            long index;
            if (config->GetFirstEntry(name, index)) {
                do {
                    const String nameStr = name.ToStdString();
                    S value;
                    success = m_serializer.read(config, IO::Path(nameStr), value);
                    if (success)
                        temp[nameStr] = value;
                } while (success && config->GetNextEntry(name, index));
            }
            
            config->SetPath(oldPath);
            
            using std::swap;
            if (success)
                swap(result, temp);
            return success;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const std::map<String, S>& values) const {
            const wxString oldPath = config->GetPath();
            config->DeleteGroup(path.asString('/'));
            config->SetPath(path.asString('/'));
            
            for (const auto& entry : values) {
                const String& name = entry.first;
                const S& value = entry.second;
                m_serializer.write(config, IO::Path(name), value);
            }
            
            config->SetPath(oldPath);
            return true;
        }
    };

    class ValueHolderBase {
    public:
        typedef std::unique_ptr<ValueHolderBase> UPtr;
    };
    
    template <typename T>
    class ValueHolder : public ValueHolderBase {
    private:
        T m_value;
    public:
        ValueHolder(T value) :
        m_value(value) {}
        
        const T& value() const {
            return m_value;
        }
    };
    
    class PreferenceBase {
    public:
        typedef std::set<const PreferenceBase*> Set;
        PreferenceBase() {}

        PreferenceBase(const PreferenceBase& other) {}
        virtual ~PreferenceBase() {}
        
        PreferenceBase& operator=(const PreferenceBase& other) { return *this; }
        
        virtual void load(wxConfigBase* config) const = 0;
        virtual void save(wxConfigBase* config) = 0;
        virtual void resetToPrevious() = 0;
        virtual void setValue(const ValueHolderBase* valueHolder) = 0;

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

        void setValue(const ValueHolderBase* valueHolder) override {
            const ValueHolder<T>* actualValueHolder = static_cast<const ValueHolder<T>*>(valueHolder);
            setValue(actualValueHolder->value());
        }
        
        bool initialized() const {
            return m_initialized;
        }
        
        void load(wxConfigBase* config) const override {
            ensure(wxThread::IsMain(), "wxConfig can only be used on the main thread");

            using std::swap;
            T temp;
            if (m_serializer.read(config, m_path, temp)) {
                std::swap(m_value, temp);
                m_previousValue = m_value;
            }
            m_initialized = true;
        }
        
        void save(wxConfigBase* config) override {
            ensure(wxThread::IsMain(), "wxConfig can only be used on the main thread");

            if (m_modified) {
                assertResult(m_serializer.write(config, m_path, m_value));
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
        
        Preference(const Preference& other) :
        PreferenceBase(other),
        m_path(other.m_path),
        m_defaultValue(other.m_defaultValue),
        m_value(other.m_value),
        m_previousValue(other.m_previousValue),
        m_initialized(other.m_initialized),
        m_modified(other.m_modified) {}
        
        Preference& operator=(Preference other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(Preference& lhs, Preference& rhs) {
            using std::swap;
            swap(lhs.m_path, rhs.m_path);
            swap(lhs.m_defaultValue, rhs.m_defaultValue);
            swap(lhs.m_value, rhs.m_value);
            swap(lhs.m_previousValue, rhs.m_previousValue);
            swap(lhs.m_initialized, rhs.m_initialized);
            swap(lhs.m_modified, rhs.m_modified);
        }
        
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
