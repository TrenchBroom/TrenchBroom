/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef ELParser_h
#define ELParser_h

#include "EL.h"
#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"

#include <list>

namespace TrenchBroom {
    namespace IO {
        namespace ELToken {
            typedef size_t Type;
            static const Type Variable    = 1 <<  1;
            static const Type String = 1 << 2;
            static const Type Number = 1 << 3;
            static const Type Boolean = 1 << 4;
            static const Type OBracket = 1 << 5;
            static const Type CBracket = 1 << 6;
            static const Type OBrace = 1 << 7;
            static const Type CBrace = 1 << 8;
            static const Type OParen = 1 << 9;
            static const Type CParen = 1 << 10;
            static const Type Plus = 1 << 11;
            static const Type Minus = 1 << 12;
            static const Type Times = 1 << 13;
            static const Type Over = 1 << 14;
            static const Type Modulus = 1 << 15;
            static const Type Colon = 1 << 16;
            static const Type Comma = 1 << 17;
            static const Type Range = 1 << 18;
            static const Type Not = 1 << 19;
            static const Type And = 1 << 20;
            static const Type Or = 1 << 21;
            static const Type Less = 1 << 22;
            static const Type LessOrEqual = 1 << 23;
            static const Type Equal = 1 << 24;
            static const Type Inequal = 1 << 25;
            static const Type GreaterOrEqual = 1 << 26;
            static const Type Greater = 1 << 27;
            static const Type Eof = 1 << 28;
            static const Type Literal = String | Number | Boolean;
            static const Type UnaryOperator = Plus | Minus | Not;
            static const Type SimpleTerm = Variable | Literal | OParen | OBracket | OBrace | UnaryOperator;
            static const Type CompoundTerm = Plus | Minus | Times | Over | Modulus | And | Or | Less | LessOrEqual | Equal | Inequal | GreaterOrEqual | Greater;
        }
        
        class ELTokenizer : public Tokenizer<ELToken::Type> {
        private:
            const String& NumberDelim() const;
            const String& IntegerDelim() const;
        public:
            ELTokenizer(const char* begin, const char* end);
            ELTokenizer(const String& str);
        public:
            void appendUntil(const String& pattern, StringStream& str);
        private:
            Token emitToken();
        };
        
        class ELParser : public Parser<ELToken::Type> {
        protected:
            ELTokenizer m_tokenizer;
            typedef ELTokenizer::Token Token;
        public:
            ELParser(const char* begin, const char* end);
            ELParser(const String& str);
            
            EL::Expression parse();
        private:
            EL::ExpressionBase* parseExpression();
            EL::ExpressionBase* parseGroupedTerm();
            EL::ExpressionBase* parseTerm();
            EL::ExpressionBase* parseSimpleTerm();
            EL::ExpressionBase* parseSubscript(EL::ExpressionBase* lhs);
            EL::ExpressionBase* parseVariable();
            EL::ExpressionBase* parseLiteral();
            EL::ExpressionBase* parseArray();
            EL::ExpressionBase* parseExpressionOrRange();
            EL::ExpressionBase* parseExpressionOrAnyRange();
            EL::ExpressionBase* parseMap();
            EL::ExpressionBase* parseUnaryOperator();
            EL::ExpressionBase* parseCompoundTerm(EL::ExpressionBase* lhs);
        private:
            TokenNameMap tokenNames() const;
        };
    }
}

#endif /* ELParser_h */
