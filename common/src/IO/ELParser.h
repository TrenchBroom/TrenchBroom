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

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace IO {
        namespace ELToken {
            typedef uint64_t Type;
            static const Type Variable              = 1ul <<  1;
            static const Type String                = 1ul <<  2;
            static const Type Number                = 1ul <<  3;
            static const Type Boolean               = 1ul <<  4;
            static const Type OBracket              = 1ul <<  5;
            static const Type CBracket              = 1ul <<  6;
            static const Type OBrace                = 1ul <<  7;
            static const Type CBrace                = 1ul <<  8;
            static const Type OParen                = 1ul <<  9;
            static const Type CParen                = 1ul << 10;
            static const Type Addition              = 1ul << 11;
            static const Type Subtraction           = 1ul << 12;
            static const Type Multiplication        = 1ul << 13;
            static const Type Division              = 1ul << 14;
            static const Type Colon                 = 1ul << 16;
            static const Type Modulus               = 1ul << 15;
            static const Type Comma                 = 1ul << 17;
            static const Type Range                 = 1ul << 18;
            static const Type LogicalNegation       = 1ul << 19;
            static const Type LogicalAnd            = 1ul << 20;
            static const Type LogicalOr             = 1ul << 21;
            static const Type Less                  = 1ul << 22;
            static const Type LessOrEqual           = 1ul << 23;
            static const Type Equal                 = 1ul << 24;
            static const Type Inequal               = 1ul << 25;
            static const Type GreaterOrEqual        = 1ul << 26;
            static const Type Greater               = 1ul << 27;
            static const Type Case                  = 1ul << 28;
            static const Type BitwiseNegation       = 1ul << 29;
            static const Type BitwiseAnd            = 1ul << 30;
            static const Type BitwiseXor            = 1ul << 31;
            static const Type BitwiseOr             = 1ul << 32;
            static const Type BitwiseShiftLeft      = 1ul << 33;
            static const Type BitwiseShiftRight     = 1ul << 34;
            static const Type DoubleOBrace          = 1ul << 35;
            static const Type DoubleCBrace          = 1ul << 36;
            static const Type Eof                   = 1ul << 37;
            static const Type Literal               = String | Number | Boolean;
            static const Type UnaryOperator         = Addition | Subtraction | LogicalNegation | BitwiseNegation;
            static const Type SimpleTerm            = Variable | Literal | OParen | OBracket | OBrace | UnaryOperator;
            static const Type CompoundTerm          = Addition | Subtraction | Multiplication | Division | Modulus | LogicalAnd | LogicalOr | Less | LessOrEqual | Equal | Inequal | GreaterOrEqual | Greater | Case | BitwiseAnd | BitwiseXor | BitwiseOr | BitwiseShiftLeft | BitwiseShiftRight;
        }

        class ELTokenizer : public Tokenizer<ELToken::Type> {
        private:
            const String& NumberDelim() const;
            const String& IntegerDelim() const;
        public:
            ELTokenizer(const char* begin, const char* end);
            ELTokenizer(const String& str);

            template <typename OtherToken>
            ELTokenizer(Tokenizer<OtherToken>& nestedTokenizer) :
            Tokenizer(nestedTokenizer) {}
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

            static EL::Expression parse(const String& str);

            template <typename OtherToken>
            ELParser(Tokenizer<OtherToken>& nestedTokenizer) :
            m_tokenizer(nestedTokenizer) {}

            EL::Expression parse();
        private:
            EL::ExpressionBase* parseExpression();
            EL::ExpressionBase* parseGroupedTerm();
            EL::ExpressionBase* parseTerm();
            EL::ExpressionBase* parseSimpleTermOrSwitch();
            EL::ExpressionBase* parseSimpleTerm();
            EL::ExpressionBase* parseSubscript(EL::ExpressionBase* lhs);
            EL::ExpressionBase* parseVariable();
            EL::ExpressionBase* parseLiteral();
            EL::ExpressionBase* parseArray();
            EL::ExpressionBase* parseExpressionOrRange();
            EL::ExpressionBase* parseExpressionOrAnyRange();
            EL::ExpressionBase* parseMap();
            EL::ExpressionBase* parseUnaryOperator();
            EL::ExpressionBase* parseSwitch();
            EL::ExpressionBase* parseCompoundTerm(EL::ExpressionBase* lhs);
        private:
            TokenNameMap tokenNames() const;
        };
    }
}

#endif /* ELParser_h */
