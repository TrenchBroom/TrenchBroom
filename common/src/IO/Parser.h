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

#ifndef TrenchBroom_Parser
#define TrenchBroom_Parser

#include "Exceptions.h"
#include "StringType.h"
#include "IO/ParserStatus.h"
#include "IO/Token.h"
#include "StringUtils.h"

#include <kdl/string_utils.h>

#include <map>

namespace TrenchBroom {
    namespace IO {
        template <typename TokenType>
        class Parser {
        protected:
            using TokenNameMap = std::map<TokenType, String>;
        private:
            using Token =  TokenTemplate<TokenType>;
            mutable TokenNameMap m_tokenNames;
        public:
            virtual ~Parser() = default;
        protected:
            bool check(const TokenType typeMask, const Token& token) const {
                return token.hasType(typeMask);
            }

            const Token& expect(const TokenType typeMask, const Token& token) const {
                if (!check(typeMask, token)) {
                    throw ParserException(token.line(), token.column(), expectString(tokenName(typeMask), token));
                }
                return token;
            }

            const Token& expect(ParserStatus& status, const TokenType typeMask, const Token& token) const {
                if (!check(typeMask, token)) {
                    expect(status, tokenName(typeMask), token);
                }
                return token;
            }

            void expect(ParserStatus& /* status */, const String& typeName, const Token& token) const {
                const String msg = expectString(typeName, token);
                throw ParserException(token.line(), token.column(), msg);
            }

            void expect(const String& expected, const Token& token) const {
                if (token.data() != expected) {
                    throw ParserException(token.line(), token.column(), "Expected string '" + expected + "', but got '" + token.data() + "'");
                }
            }

            void expect(const StringList& expected, const Token& token) const {
                for (const auto& str : expected) {
                    if (token.data() == str) {
                        return;
                    }
                }
                throw ParserException(token.line(), token.column(), "Expected string '" + kdl::str_join(expected, "', '", "', or '", "' or '") + "', but got '" + token.data() + "'");
            }
       private:
            String expectString(const String& expected, const Token& token) const {
                StringStream msg;
                msg << "Expected " << expected << ", but got " << tokenName(token.type());
                if (!token.data().empty())
                    msg << " (raw data: '" << token.data() << "')";
                return msg.str();
            }
        protected:
            String tokenName(const TokenType typeMask) const {
                if (m_tokenNames.empty())
                    m_tokenNames = tokenNames();

                StringList names;
                for (const auto& entry : m_tokenNames) {
                    const TokenType type = entry.first;
                    const String& name = entry.second;
                    if ((typeMask & type) != 0)
                        names.push_back(name);
                }

                if (names.empty())
                    return "unknown token type";
                if (names.size() == 1)
                    return names[0];
                return kdl::str_join(names, ", ", ", or ", " or ");
            }
        private:
            virtual TokenNameMap tokenNames() const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Parser) */
