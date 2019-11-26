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

#ifndef ELParser_h
#define ELParser_h

#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace EL {
        class Expression;
        class ExpressionBase;
    }

    namespace IO {
        namespace ELToken {
            using Type = uint64_t;
            static const Type Name              = Type(1) <<  1;
            static const Type String                = Type(1) <<  2;
            static const Type Number                = Type(1) <<  3;
            static const Type Boolean               = Type(1) <<  4;
            static const Type OBracket              = Type(1) <<  5;
            static const Type CBracket              = Type(1) <<  6;
            static const Type OBrace                = Type(1) <<  7;
            static const Type CBrace                = Type(1) <<  8;
            static const Type OParen                = Type(1) <<  9;
            static const Type CParen                = Type(1) << 10;
            static const Type Addition              = Type(1) << 11;
            static const Type Subtraction           = Type(1) << 12;
            static const Type Multiplication        = Type(1) << 13;
            static const Type Division              = Type(1) << 14;
            static const Type Colon                 = Type(1) << 16;
            static const Type Modulus               = Type(1) << 15;
            static const Type Comma                 = Type(1) << 17;
            static const Type Range                 = Type(1) << 18;
            static const Type LogicalNegation       = Type(1) << 19;
            static const Type LogicalAnd            = Type(1) << 20;
            static const Type LogicalOr             = Type(1) << 21;
            static const Type Less                  = Type(1) << 22;
            static const Type LessOrEqual           = Type(1) << 23;
            static const Type Equal                 = Type(1) << 24;
            static const Type Inequal               = Type(1) << 25;
            static const Type GreaterOrEqual        = Type(1) << 26;
            static const Type Greater               = Type(1) << 27;
            static const Type Case                  = Type(1) << 28;
            static const Type BitwiseNegation       = Type(1) << 29;
            static const Type BitwiseAnd            = Type(1) << 30;
            static const Type BitwiseXor            = Type(1) << 31;
            static const Type BitwiseOr             = Type(1) << 32;
            static const Type BitwiseShiftLeft      = Type(1) << 33;
            static const Type BitwiseShiftRight     = Type(1) << 34;
            static const Type DoubleOBrace          = Type(1) << 35;
            static const Type DoubleCBrace          = Type(1) << 36;
            static const Type Null                  = Type(1) << 37;
            static const Type Eof                   = Type(1) << 38;
            static const Type Literal               = String | Number | Boolean | Null;
            static const Type UnaryOperator         = Addition | Subtraction | LogicalNegation | BitwiseNegation;
            static const Type SimpleTerm            = Name | Literal | OParen | OBracket | OBrace | UnaryOperator;
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
            Token emitToken() override;
        };

        class ELParser : public Parser<ELToken::Type> {
        public:
            enum class Mode {
                Strict,
                Lenient
            };
        protected:
            ELParser::Mode m_mode;
            ELTokenizer m_tokenizer;
            using Token = ELTokenizer::Token;
        public:
            ELParser(ELParser::Mode mode, const char* begin, const char* end);
            ELParser(ELParser::Mode mode, const String& str);

            static EL::Expression parseStrict(const String& str);
            static EL::Expression parseLenient(const String& str);

            template <typename OtherToken>
            ELParser(Tokenizer<OtherToken>& nestedTokenizer) :
                    m_mode(Mode::Lenient),
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
            TokenNameMap tokenNames() const override;
        };
    }
}

#endif /* ELParser_h */
