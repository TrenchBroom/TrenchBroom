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

#ifndef TrenchBroom_Parser
#define TrenchBroom_Parser

#include "Exceptions.h"
#include "StringUtils.h"
#include "IO/ParserStatus.h"
#include "IO/Token.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        template <typename TokenType>
        class Parser {
        protected:
            typedef std::map<TokenType, String> TokenNameMap;
        private:
            typedef TokenTemplate<TokenType> Token;
            mutable TokenNameMap m_tokenNames;
        public:
            virtual ~Parser() {}
        protected:
            void expect(const TokenType typeMask, const Token& token) const {
                if ((token.type() & typeMask) == 0) {
                    const String data(token.begin(), token.end());
                    throw ParserException(token.line(), token.column()) << " Expected " << tokenName(typeMask) << ", but got '" << data << "'";
                }
            }
            
            void expect(ParserStatus& status, const TokenType typeMask, const Token& token) const {
                if ((token.type() & typeMask) == 0) {
                    const String data(token.begin(), token.end());
                    StringStream msg;
                    msg << " Expected " << tokenName(typeMask) << ", but got '" << data << "'";
                    const String msgStr = msg.str();
                    status.error(token.line(), token.column(), msgStr);
                    throw ParserException(token.line(), token.column(), msgStr);
                }
            }

            String tokenName(const TokenType typeMask) const {
                if (m_tokenNames.empty())
                    m_tokenNames = tokenNames();
                
                StringList names;
                typename TokenNameMap::const_iterator it, end;
                for (it = m_tokenNames.begin(), end = m_tokenNames.end(); it != end; ++it) {
                    const TokenType type = it->first;
                    const String& name = it->second;
                    if ((typeMask & type) != 0)
                        names.push_back(name);
                }
                
                if (names.empty())
                    return "unknown token type";
                if (names.size() == 1)
                    return names[0];
                return StringUtils::join(names, ", ", ", or ", " or ");
            }
        private:
            virtual TokenNameMap tokenNames() const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Parser) */
