/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Exceptions.h"
#include "Macros.h"
#include "StringUtils.h"
#include "IO/Path.h"
#include "View/KeyboardShortcut.h"

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/tokenzr.h>

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
    class Converter<double> {
    public:
        wxString toWxString(const double& value) const {
            wxString string;
            string << value;
            return string;
        }
        
        double fromWxString(const wxString& string) const {
            double doubleValue;
            if (string.ToDouble(&doubleValue))
                return doubleValue;
            return 0.0;
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
            return Color::parse(string.ToStdString());
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
    
    template<>
    class Converter<IO::Path> {
    public:
        wxString toWxString(const IO::Path& value) const {
            return wxString(value.asString());
        }
        
        IO::Path fromWxString(const wxString& string) const {
            return IO::Path(string.ToStdString());
        }
    };

    template<>
    class Converter<StringMap > {
    public:
        wxString toWxString(const StringMap& values) const {
            wxString result("{ ");
            StringMap::const_iterator it, end;
            size_t index = 0;
            for (it = values.begin(), end = values.end(); it != end; ++it) {
                const String& key = it->first;
                const String& value = it->second;
                result << StringUtils::escape(key, "\"") << " = " << StringUtils::escape(value, "\"");
                if (index < values.size() - 1)
                    result << "; ";
                ++index;
            }
            result << " }";
            
            return result;
        }
        
        StringMap fromWxString(const wxString& string) const {
            StringMap result;
            
            wxStringTokenizer tokenizer(string);
            if (!tokenizer.HasMoreTokens())
                return result;

            wxString token;
            StringStream buffer;
            
            expect(tokenizer, "{");
            token = nextToken(tokenizer);
            while (token != "}") {
                const String key = StringUtils::unescape(readString(tokenizer, token), "\"");
                expect(tokenizer, "=");
                const String value = StringUtils::unescape(readString(tokenizer, nextToken(tokenizer)), "\"");
                token = expect(tokenizer, "}", ";");
                
                result[key] = value;
            }
            return result;
        }
    private:
        wxString expect(wxStringTokenizer& tokenizer, const String& expected1, const String& expected2 = "", const String& expected3 = "") const {
            const wxString token = nextToken(tokenizer);
            expect(token, expected1, expected2, expected3);
            return token;
        }
        
        wxString nextToken(wxStringTokenizer& tokenizer) const {
            if (!tokenizer.HasMoreTokens())
                throw ParserException("Unexpected end of string");
            return tokenizer.GetNextToken();
        }
        
        void expect(const wxString& token, const String& expected1, const String& expected2 = "", const String& expected3 = "") const {
            if (token != expected1 &&
                !expected2.empty() && token != expected2 &&
                !expected3.empty() && token != expected3) {
                ParserException e;
                e << "Expected '" << expected1 << "'";
                if (!expected2.empty())
                    e << ", '" << expected2 << "'";
                if (!expected3.empty())
                    e << ", '" << expected3 << "'";
                e << ", but got '" << token.ToStdString() << "'";
                throw e;
            }
        }
        
        String readString(wxStringTokenizer& tokenizer, wxString token) const {
            static StringStream buffer;
            
            if (token[0] != '"') {
                char c;
                ParserException e;
                e << "Expected '\"', but got '";
                if (token[0].GetAsChar(&c))
                    e << c;
                else
                    e << "<non-ASCII-char>";
                e << "'";
                throw e;
            }
            
            buffer.str("");
            while (token[token.size() - 1] != '\"' &&
                   (token.size() == 1 || token[token.size() - 2] != '\\')) {
                buffer << token.ToStdString();
                token = nextToken(tokenizer);
            }
            buffer << token.ToStdString();
            return buffer.str();
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

        bool operator== (const PreferenceBase& other) const {
            return this == &other;
        }

        virtual const IO::Path& path() const = 0;
    };
    
    
    template <typename T>
    class Preference : public PreferenceBase {
    protected:
        friend class PreferenceManager;
        template<typename> friend class SetTemporaryPreference;
        
        Converter<T> m_converter;
        IO::Path m_path;
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
            if (config->Read(m_path.asString('/'), &string))
                m_value = m_converter.fromWxString(string);
            m_initialized = true;
        }
        
        void save(wxConfigBase* config) {
            if (m_modified) {
                wxString string = m_converter.toWxString(m_value);
                bool success = config->Write(m_path.asString('/'), string);
                assert(success);
                _UNUSED(success);
                
                m_modified = false;
            }
        }
    public:
        Preference(const IO::Path& path, const T& defaultValue) :
        m_path(path),
        m_value(defaultValue),
        m_initialized(false),
        m_modified(false) {
            m_modified = m_initialized;
        }
        
        const IO::Path& path() const {
            return m_path;
        }
        
        const T& value() const {
            return m_value;
        }
    };
}

#endif /* defined(__TrenchBroom__Preference__) */
