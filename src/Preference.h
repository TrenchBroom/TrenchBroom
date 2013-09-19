/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Preference__
#define __TrenchBroom__Preference__

#include "Color.h"
#include "StringUtils.h"
#include "View/KeyboardShortcut.h"

#include <wx/config.h>
#include <wx/confbase.h>

namespace TrenchBroom {
    template <typename T>
    class Converter {
    public:
        wxString toWxString(const T& value) const {
            return "";
        }
        
        T fromWxString(const wxString& string) const {
            return T();
        }
    };
    
    template <>
    class Converter<bool> {
    public:
        wxString toWxString(const bool& value) const {
            wxString string;
            string << (value ? 1 : 0);
            return string;
        }
        
        bool fromWxString(const wxString& string) const {
            long longValue;
            if (string.ToLong(&longValue))
                return longValue != 0L;
            return false;
        }
    };
    
    template <>
    class Converter<int> {
    public:
        wxString toWxString(const int& value) const {
            wxString string;
            string << value;
            return string;
        }
        
        int fromWxString(const wxString& string) const {
            long longValue;
            if (string.ToLong(&longValue) && longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max())
                return static_cast<int>(longValue);
            return 0;
        }
    };
    
    template <>
    class Converter<float> {
    public:
        wxString toWxString(const float& value) const {
            wxString string;
            string << value;
            return string;
        }
        
        float fromWxString(const wxString& string) const {
            double doubleValue;
            if (string.ToDouble(&doubleValue) && doubleValue >= std::numeric_limits<float>::min() && doubleValue <= std::numeric_limits<float>::max())
                return static_cast<float>(doubleValue);
            return 0.0f;
        }
    };
    
    template <>
    class Converter<String> {
    public:
        wxString toWxString(const String& value) const {
            return wxString(value);
        }
        
        String fromWxString(const wxString& string) const {
            return string.ToStdString();
        }
    };
    
    template <>
    class Converter<Color> {
    public:
        wxString toWxString(const Color& value) const {
            return wxString(value.asString());
        }
        
        Color fromWxString(const wxString& string) const {
            return Color(string.ToStdString());
        }
    };
    
    template<>
    class Converter<View::KeyboardShortcut> {
    public:
        wxString toWxString(const View::KeyboardShortcut& value) const {
            return wxString(value.asString());
        }
        
        View::KeyboardShortcut fromWxString(const wxString& string) const {
            return View::KeyboardShortcut(string.ToStdString());
        }
    };
    
    class ValueHolderBase {};
    
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
        
        virtual ~PreferenceBase() {}
        
        virtual void load(wxConfigBase* config) = 0;
        virtual void save(wxConfigBase* config) = 0;
        virtual void setValue(const ValueHolderBase* valueHolder) = 0;
        
        const bool operator== (const PreferenceBase& other) const {
            return this == &other;
        }
    };
    
    
    template <typename T>
    class Preference : public PreferenceBase {
    protected:
        friend class PreferenceManager;
        template<typename> friend class SetTemporaryPreference;
        
        Converter<T> m_converter;
        String m_name;
        T m_value;
        bool m_initialized;
        bool m_modified;
        
        void setValue(const T& value) {
            m_modified = true;
            m_value = value;
        }
        
        void setValue(const ValueHolderBase* valueHolder) {
            const ValueHolder<T>* actualValueHolder = static_cast<const ValueHolder<T>*>(valueHolder);
            setValue(actualValueHolder->value());
        }
        
        bool initialized() const {
            return m_initialized;
        }
        
        void load(wxConfigBase* config) {
            wxString string;
            if (config->Read(m_name, &string))
                m_value = m_converter.fromWxString(string);
            m_initialized = true;
        }
        
        void save(wxConfigBase* config) {
            if (m_modified) {
                wxString string = m_converter.toWxString(m_value);
                bool success = config->Write(m_name, string);
                assert(success);
                m_modified = false;
            }
        }
    public:
        Preference(const String& name, const T& defaultValue) :
        m_name(name),
        m_value(defaultValue),
        m_initialized(false),
        m_modified(false) {
            m_modified = m_initialized;
        }
        
        const String& name() const {
            return m_name;
        }
        
        const T& value() const {
            return m_value;
        }
    };
}

#endif /* defined(__TrenchBroom__Preference__) */
