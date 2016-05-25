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

#include "ELParser.h"

#include "CollectionUtils.h"
#include "EL.h"

namespace TrenchBroom {
    namespace IO {
        const String& ELTokenizer::NumberDelim() const {
            static const String Delim = Whitespace() + "(){}[],:+-*/%";
            return Delim;
        }

        ELTokenizer::ELTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        ELTokenizer::ELTokenizer(const String& str) :
        Tokenizer(str) {}
        
        ELTokenizer::Token ELTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '[':
                        advance();
                        return Token(ELToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        advance();
                        return Token(ELToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '{':
                        advance();
                        return Token(ELToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(ELToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        advance();
                        return Token(ELToken::OParen, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(ELToken::CParen, c, c+1, offset(c), startLine, startColumn);
                    case '+':
                        advance();
                        return Token(ELToken::Plus, c, c+1, offset(c), startLine, startColumn);
                    case '-':
                        advance();
                        return Token(ELToken::Minus, c, c+1, offset(c), startLine, startColumn);
                    case '*':
                        advance();
                        return Token(ELToken::Times, c, c+1, offset(c), startLine, startColumn);
                    case '/':
                        advance();
                        if (curChar() == '/')
                            discardUntil("\n\r");
                        return Token(ELToken::Over, c, c+1, offset(c), startLine, startColumn);
                    case '%':
                        advance();
                        return Token(ELToken::Modulus, c, c+1, offset(c), startLine, startColumn);
                    case ':':
                        advance();
                        return Token(ELToken::Colon, c, c+1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(ELToken::Comma, c, c+1, offset(c), startLine, startColumn);
                    case '"': {
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(ELToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace());
                        break;
                    default: {
                        const char* e;
                        if ((e = readInteger(NumberDelim())) != NULL)
                            return Token(ELToken::Number, c, e, offset(c), startLine, startColumn);
                        
                        if ((e = readDecimal(NumberDelim())) != NULL)
                            return Token(ELToken::Number, c, e, offset(c), startLine, startColumn);

                        if ((e = discard("true")) != NULL)
                            return Token(ELToken::Boolean, c, e, offset(c), startLine, startColumn);
                        if ((e = discard("false")) != NULL)
                            return Token(ELToken::Boolean, c, e, offset(c), startLine, startColumn);

                        if (isLetter(*c) || *c == '_') {
                            do {
                                advance();
                                e = curPos();
                            } while (!eof() && (isLetter(*e) || isDigit(*e) || *e == '_'));
                            
                            return Token(ELToken::Variable, c, e, offset(c), startLine, startColumn);
                        }
                        
                        throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                    }
                }
            }
            return Token(ELToken::Eof, NULL, NULL, length(), line(), column());
        }

        ELParser::ELParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}
        
        ELParser::ELParser(const String& str) :
        m_tokenizer(str) {}
        
        EL::Expression* ELParser::parse() {
            return parseExpression();
        }

        EL::Expression* ELParser::parseExpression() {
            if (m_tokenizer.peekToken().hasType(ELToken::OParen)) {
                return parseGroupedTerm();
            } else {
                return parseTerm();
            }
        }

        EL::Expression* ELParser::parseGroupedTerm() {
            expect(ELToken::OParen, m_tokenizer.nextToken());
            EL::Expression* expression = parseTerm();
            expect(ELToken::CParen, m_tokenizer.nextToken());
            
            return EL::GroupingOperator::create(expression);
        }

        EL::Expression* ELParser::parseTerm() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm, token);
            
            EL::Expression* lhs = parseSimpleTerm();
            token = m_tokenizer.peekToken();
            if (token.hasType(ELToken::CompoundTerm))
                return parseCompoundTerm(lhs);
            return lhs;
        }

        EL::Expression* ELParser::parseSimpleTerm() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm, token);
            
            EL::Expression* term = NULL;
            if (token.hasType(ELToken::Plus | ELToken::Minus))
                term = parseUnaryOperator();
            else if (token.hasType(ELToken::OParen))
                term = parseGroupedTerm();
            else if (token.hasType(ELToken::Variable))
                term = parseVariable();
            else
                term = parseLiteral();
            
            while (m_tokenizer.peekToken().hasType(ELToken::OBracket)) {
                m_tokenizer.nextToken();
                term = EL::SubscriptOperator::create(term, parseExpression());
                expect(ELToken::CBracket, m_tokenizer.nextToken());
            }
            
            return term;
        }
        
        EL::Expression* ELParser::parseVariable() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::Variable, token);
            return EL::VariableExpression::create(token.data());
        }

        EL::Expression* ELParser::parseLiteral() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::String | ELToken::Number | ELToken::Boolean | ELToken::OBracket | ELToken::OBrace, token);
            
            if (token.hasType(ELToken::String))
                return EL::LiteralExpression::create(EL::Value(token.data()));
            if (token.hasType(ELToken::Number))
                return EL::LiteralExpression::create(EL::Value(token.toFloat<EL::NumberType>()));
            if (token.hasType(ELToken::Boolean))
                return EL::LiteralExpression::create(EL::Value(token.data() == "true"));
            
            m_tokenizer.pushToken(token);
            if (token.hasType(ELToken::OBracket))
                return parseArray();
            return parseMap();
        }

        EL::Expression* ELParser::parseArray() {
            EL::Expression::List elements;
            
            expect(ELToken::OBracket, m_tokenizer.nextToken());
            while (!m_tokenizer.peekToken().hasType(ELToken::CBracket)) {
                elements.push_back(parseExpression());
                Token token = m_tokenizer.nextToken();
                expect(ELToken::Comma | ELToken::CBracket, token);
                if (token.hasType(ELToken::CBracket))
                    m_tokenizer.pushToken(token);
                
            }
            expect(ELToken::CBracket, m_tokenizer.nextToken());
            
            return EL::ArrayLiteralExpression::create(elements);
        }
        
        EL::Expression* ELParser::parseMap() {
            EL::Expression::Map elements;
            
            expect(ELToken::OBrace, m_tokenizer.nextToken());
            while (!m_tokenizer.peekToken().hasType(ELToken::CBrace)) {
                Token token = m_tokenizer.nextToken();
                expect(ELToken::String, token);
                const String key = token.data();
                
                expect(ELToken::Colon, m_tokenizer.nextToken());
                EL::Expression* value = parseExpression();

                MapUtils::insertOrReplaceAndDelete(elements, key, value);
                
                expect(ELToken::Comma | ELToken::CBrace, token = m_tokenizer.nextToken());
                if (token.hasType(ELToken::CBrace))
                    m_tokenizer.pushToken(token);
            }
            expect(ELToken::CBrace, m_tokenizer.nextToken());
            
            return EL::MapLiteralExpression::create(elements);
        }

        EL::Expression* ELParser::parseUnaryOperator() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::Plus | ELToken::Minus, token);
            
            if (token.hasType(ELToken::Plus))
                return EL::UnaryPlusOperator::create(parseSimpleTerm());
            return EL::UnaryMinusOperator::create(parseSimpleTerm());
        }

        EL::Expression* ELParser::parseCompoundTerm(EL::Expression* lhs) {
            EL::BinaryOperator* op = NULL;
            
            {
                Token token = m_tokenizer.nextToken();
                expect(ELToken::Plus | ELToken::Minus | ELToken::Times | ELToken::Over | ELToken::Modulus, token);
                
                
                if (token.hasType(ELToken::Plus))
                    op = EL::AdditionOperator::create(lhs, parseSimpleTerm());
                else if (token.hasType(ELToken::Minus))
                    op = EL::SubtractionOperator::create(lhs, parseSimpleTerm());
                else if (token.hasType(ELToken::Times))
                    op = EL::MultiplicationOperator::create(lhs, parseSimpleTerm());
                else if (token.hasType(ELToken::Over))
                    op = EL::DivisionOperator::create(lhs, parseSimpleTerm());
                else
                    op = EL::ModulusOperator::create(lhs, parseSimpleTerm());
            }
            
            while (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm)) {
                Token token = m_tokenizer.nextToken();
                expect(ELToken::Plus | ELToken::Minus | ELToken::Times | ELToken::Over | ELToken::Modulus, token);
                
                if (token.hasType(ELToken::Plus))
                    op = EL::AdditionOperator::create(op, parseSimpleTerm());
                else if (token.hasType(ELToken::Minus))
                    op = EL::SubtractionOperator::create(op, parseSimpleTerm());
                else if (token.hasType(ELToken::Times))
                    op = EL::MultiplicationOperator::create(op, parseSimpleTerm());
                else if (token.hasType(ELToken::Over))
                    op = EL::DivisionOperator::create(op, parseSimpleTerm());
                else
                    op = EL::ModulusOperator::create(op, parseSimpleTerm());
            }
            
            return op;
        }

        ELParser::TokenNameMap ELParser::tokenNames() const {
            TokenNameMap result;
            result[ELToken::Variable]   = "variable";
            result[ELToken::String]     = "string";
            result[ELToken::Number]     = "number";
            result[ELToken::Boolean]    = "boolean";
            result[ELToken::OBracket]   = "'['";
            result[ELToken::CBracket]   = "']'";
            result[ELToken::OBrace]     = "'{'";
            result[ELToken::CBrace]     = "'}'";
            result[ELToken::OParen]     = "'('";
            result[ELToken::CParen]     = "')'";
            result[ELToken::Plus]       = "'+'";
            result[ELToken::Minus]      = "'-'";
            result[ELToken::Times]      = "'*'";
            result[ELToken::Over]       = "'/'";
            result[ELToken::Modulus]    = "'%'";
            result[ELToken::Colon]      = "':'";
            result[ELToken::Comma]      = "','";
            result[ELToken::Eof]        = "end of file";
            return result;
        }
    }
}
