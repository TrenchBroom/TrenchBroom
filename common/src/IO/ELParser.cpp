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

#include "ELParser.h"

#include "EL/Expression.h"
#include "EL/Value.h"

#include <kdl/string_format.h>

#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace TrenchBroom {
    namespace IO {
        const std::string& ELTokenizer::NumberDelim() const {
            static const std::string Delim = Whitespace() + "(){}[],:+-*/%";
            return Delim;
        }

        const std::string& ELTokenizer::IntegerDelim() const {
            static const std::string Delim = NumberDelim() + ".";
            return Delim;
        }

        ELTokenizer::ELTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "\"", '\\') {}

        ELTokenizer::ELTokenizer(const std::string& str) :
        Tokenizer(str, "\"", '\\') {}

        void ELTokenizer::appendUntil(const std::string& pattern, std::stringstream& str) {
            const char* begin = curPos();
            const char* end = discardUntilPattern(pattern);
            str << std::string(begin, end);
            if (!eof())
                discard("${");
        }

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
                        if (curChar() == '{') {
                            advance();
                            return Token(ELToken::DoubleOBrace, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        if (curChar() == '}') {
                            advance();
                            return Token(ELToken::DoubleCBrace, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        advance();
                        return Token(ELToken::OParen, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(ELToken::CParen, c, c+1, offset(c), startLine, startColumn);
                    case '+':
                        advance();
                        return Token(ELToken::Addition, c, c+1, offset(c), startLine, startColumn);
                    case '-':
                        advance();
                        if (curChar() == '>') {
                            advance();
                            return Token(ELToken::Case, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::Subtraction, c, c+1, offset(c), startLine, startColumn);
                    case '*':
                        advance();
                        return Token(ELToken::Multiplication, c, c+1, offset(c), startLine, startColumn);
                    case '/':
                        advance();
                        if (curChar() == '/') {
                            discardUntil("\n\r");
                            break;
                        }
                        return Token(ELToken::Division, c, c+1, offset(c), startLine, startColumn);
                    case '%':
                        advance();
                        return Token(ELToken::Modulus, c, c+1, offset(c), startLine, startColumn);
                    case '~':
                        advance();
                        return Token(ELToken::BitwiseNegation, c, c+1, offset(c), startLine, startColumn);
                    case '&':
                        advance();
                        if (curChar() == '&') {
                            advance();
                            return Token(ELToken::LogicalAnd, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::BitwiseAnd, c, c+1, offset(c), startLine, startColumn);
                    case '|':
                        advance();
                        if (curChar() == '|') {
                            advance();
                            return Token(ELToken::LogicalOr, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::BitwiseOr, c, c+1, offset(c), startLine, startColumn);
                    case '^':
                        advance();
                        return Token(ELToken::BitwiseXOr, c, c+1, offset(c), startLine, startColumn);
                    case '!':
                        advance();
                        if (curChar() == '=') {
                            advance();
                            return Token(ELToken::NotEqual, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::LogicalNegation, c, c+1, offset(c), startLine, startColumn);
                    case '<':
                        advance();
                        if (curChar() == '=') {
                            advance();
                            return Token(ELToken::LessOrEqual, c, c+2, offset(c), startLine, startColumn);
                        } else if (curChar() == '<') {
                            advance();
                            return Token(ELToken::BitwiseShiftLeft, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::Less, c, c+1, offset(c), startLine, startColumn);
                    case '>':
                        advance();
                        if (curChar() == '=') {
                            advance();
                            return Token(ELToken::GreaterOrEqual, c, c+2, offset(c), startLine, startColumn);
                        } else if (curChar() == '>') {
                            advance();
                            return Token(ELToken::BitwiseShiftRight, c, c+2, offset(c), startLine, startColumn);
                        }
                        return Token(ELToken::Greater, c, c+1, offset(c), startLine, startColumn);
                    case ':':
                        advance();
                        return Token(ELToken::Colon, c, c+1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(ELToken::Comma, c, c+1, offset(c), startLine, startColumn);
                    case '\'':
                    case '"': {
                        const char delim = curChar();
                        advance();
                        c = curPos();
                        const char* e = readQuotedString(delim);
                        return Token(ELToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace());
                        break;
                    default: {
                        switch (curChar()) {
                            case '.':
                                if (lookAhead() == '.') {
                                    advance(2);
                                    return Token(ELToken::Range, c, c+2, offset(c), startLine, startColumn);
                                }
                                break;
                            case '=':
                                if (curChar() == '=') {
                                    advance(2);
                                    return Token(ELToken::Equal, c, c+2, offset(c), startLine, startColumn);
                                }
                                break;
                            default:
                                break;
                        }

                        const char* e;
                        if ((e = readDecimal(NumberDelim())) != nullptr) {
                            if (!eof() && curChar() == '.' && lookAhead() != '.')
                                throw ParserException(startLine, startColumn, "Unexpected character: " + std::string(c, 1));
                            return Token(ELToken::Number, c, e, offset(c), startLine, startColumn);
                        }

                        if ((e = readInteger(IntegerDelim())) != nullptr)
                            return Token(ELToken::Number, c, e, offset(c), startLine, startColumn);

                        if ((e = discard("true")) != nullptr)
                            return Token(ELToken::Boolean, c, e, offset(c), startLine, startColumn);
                        if ((e = discard("false")) != nullptr)
                            return Token(ELToken::Boolean, c, e, offset(c), startLine, startColumn);

                        if ((e = discard("null")) != nullptr)
                            return Token(ELToken::Null, c, e, offset(c), startLine, startColumn);

                        if (isLetter(*c) || *c == '_') {
                            do {
                                advance();
                                e = curPos();
                            } while (!eof() && (isLetter(*e) || isDigit(*e) || *e == '_'));

                            return Token(ELToken::Name, c, e, offset(c), startLine, startColumn);
                        }

                        throw ParserException(startLine, startColumn, "Unexpected character: " + std::string(c, 1));
                    }
                }
            }
            return Token(ELToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        ELParser::ELParser(const ELParser::Mode mode, const char* begin, const char* end) :
        m_mode(mode),
        m_tokenizer(begin, end) {}

        ELParser::ELParser(const ELParser::Mode mode, const std::string& str) :
        m_mode(mode),
        m_tokenizer(str) {}

        EL::Expression ELParser::parseStrict(const std::string& str) {
            return ELParser(Mode::Strict, str).parse();
        }

        EL::Expression ELParser::parseLenient(const std::string& str) {
            return ELParser(Mode::Lenient, str).parse();
        }

        EL::Expression ELParser::parse() {
            auto result = parseExpression();
            if (m_mode == Mode::Strict) {
                expect(ELToken::Eof, m_tokenizer.peekToken()); // avoid trailing garbage
            }
            return result;
        }

        EL::Expression ELParser::parseExpression() {
            if (m_tokenizer.peekToken().hasType(ELToken::OParen)) {
                return parseGroupedTerm();
            } else {
                return parseTerm();
            }
        }

        EL::Expression ELParser::parseGroupedTerm() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::OParen, token);
            EL::Expression expression = parseTerm();
            expect(ELToken::CParen, m_tokenizer.nextToken());

            EL::Expression lhs = EL::Expression(EL::UnaryExpression(EL::UnaryOperator::Group, std::move(expression)), token.line(), token.column());
            if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
                return parseCompoundTerm(lhs);
            return lhs;
        }

        EL::Expression ELParser::parseTerm() {
            expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, m_tokenizer.peekToken());

            EL::Expression lhs = parseSimpleTermOrSwitch();
            if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
                return parseCompoundTerm(lhs);
            return lhs;
        }

        EL::Expression ELParser::parseSimpleTermOrSwitch() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, token);

            if (token.hasType(ELToken::SimpleTerm))
                return parseSimpleTermOrSubscript();
            return parseSwitch();
        }

        EL::Expression ELParser::parseSimpleTermOrSubscript() {
            EL::Expression term = parseSimpleTerm();

            while (m_tokenizer.peekToken().hasType(ELToken::OBracket)) {
                term = parseSubscript(std::move(term));
            }

            return term;
        }

        EL::Expression ELParser::parseSimpleTerm() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm, token);

            if (token.hasType(ELToken::UnaryOperator)) {
                return parseUnaryOperator();
            } else if (token.hasType(ELToken::OParen)) {
                return parseGroupedTerm();
            } else if (token.hasType(ELToken::Name)) {
                return parseVariable();
            } else {
                return parseLiteral();
            }
        }
        
        EL::Expression ELParser::parseSubscript(EL::Expression lhs) {
            Token token = m_tokenizer.nextToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            expect(ELToken::OBracket, token);
            std::vector<EL::Expression> elements;
            if (!m_tokenizer.peekToken().hasType(ELToken::CBracket)) {
                do {
                    elements.push_back(parseExpressionOrAnyRange());
                } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else {
                m_tokenizer.nextToken();
            }

            auto rhs = elements.size() == 1u ? std::move(elements.front()) : EL::Expression(EL::ArrayExpression(std::move(elements)), startLine, startColumn);
            return EL::Expression(EL::SubscriptExpression(std::move(lhs), std::move(rhs)), startLine, startColumn);
        }

        EL::Expression ELParser::parseVariable() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::Name, token);
            return EL::Expression(EL::VariableExpression(token.data()), token.line(), token.column());
        }

        EL::Expression ELParser::parseLiteral() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::Literal | ELToken::OBracket | ELToken::OBrace, token);

            if (token.hasType(ELToken::String)) {
                m_tokenizer.nextToken();
                // Escaping happens in EL::Value::appendToStream
                std::string value = kdl::str_unescape(token.data(), "\\\"");
                return EL::Expression(EL::LiteralExpression(EL::Value(std::move(value))), token.line(), token.column());
            }
            if (token.hasType(ELToken::Number)) {
                m_tokenizer.nextToken();
                return EL::Expression(EL::LiteralExpression(EL::Value(token.toFloat<EL::NumberType>())), token.line(), token.column());
            }
            if (token.hasType(ELToken::Boolean)) {
                m_tokenizer.nextToken();
                return EL::Expression(EL::LiteralExpression(EL::Value(token.data() == "true")), token.line(), token.column());
            }
            if (token.hasType(ELToken::Null)) {
                m_tokenizer.nextToken();
                return EL::Expression(EL::LiteralExpression(EL::Value::Null), token.line(), token.column());
            }

            if (token.hasType(ELToken::OBracket)) {
                return parseArray();
            } else {
                return parseMap();
            }
        }

        EL::Expression ELParser::parseArray() {
            Token token = m_tokenizer.nextToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            expect(ELToken::OBracket, token);
            std::vector<EL::Expression> elements;
            if (!m_tokenizer.peekToken().hasType(ELToken::CBracket)) {
                do {
                    elements.push_back(parseExpressionOrRange());
                } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else {
                m_tokenizer.nextToken();
            }

            return EL::Expression(EL::ArrayExpression(std::move(elements)), startLine, startColumn);
        }

        EL::Expression ELParser::parseExpressionOrRange() {
            EL::Expression expression = parseExpression();
            if (m_tokenizer.peekToken().hasType(ELToken::Range)) {
                Token token = m_tokenizer.nextToken();
                expression = EL::Expression(EL::BinaryExpression(EL::BinaryOperator::Range, std::move(expression), parseExpression()), token.line(), token.column());
            }

            return expression;
        }

        EL::Expression ELParser::parseExpressionOrAnyRange() {
            std::optional<EL::Expression> expression;
            if (m_tokenizer.peekToken().hasType(ELToken::Range)) {
                Token token = m_tokenizer.nextToken();
                expression = EL::BinaryExpression::createAutoRangeWithRightOperand(parseExpression(), token.line(), token.column());
            } else {
                expression = parseExpression();
                if (m_tokenizer.peekToken().hasType(ELToken::Range)) {
                    Token token = m_tokenizer.nextToken();
                    if (m_tokenizer.peekToken().hasType(ELToken::SimpleTerm)) {
                        expression = EL::Expression(EL::BinaryExpression(EL::BinaryOperator::Range, std::move(*expression), parseExpression()), token.line(), token.column());
                    } else {
                        expression = EL::BinaryExpression::createAutoRangeWithLeftOperand(std::move(*expression), token.line(), token.column());
                    }
                }
            }

            return *expression;
        }

        EL::Expression ELParser::parseMap() {
            std::map<std::string, EL::Expression> elements;

            Token token = m_tokenizer.nextToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            expect(ELToken::OBrace, token);
            if (!m_tokenizer.peekToken().hasType(ELToken::CBrace)) {
                do {
                    token = m_tokenizer.nextToken();
                    expect(ELToken::String | ELToken::Name, token);
                    std::string key = token.data();

                    expect(ELToken::Colon, m_tokenizer.nextToken());
                    elements.insert({ std::move(key), parseExpression() });
                } while (expect(ELToken::Comma | ELToken::CBrace, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else {
                m_tokenizer.nextToken();
            }

            return EL::Expression(EL::MapExpression(std::move(elements)), startLine, startColumn);
        }

        EL::Expression ELParser::parseUnaryOperator() {
            static const auto TokenMap = std::unordered_map<ELToken::Type, EL::UnaryOperator>{
                { ELToken::Addition, EL::UnaryOperator::Plus },
                { ELToken::Subtraction, EL::UnaryOperator::Minus },
                { ELToken::LogicalNegation, EL::UnaryOperator::LogicalNegation },
                { ELToken::BitwiseNegation, EL::UnaryOperator::BitwiseNegation },
            };

            Token token = m_tokenizer.nextToken();
            expect(ELToken::UnaryOperator, token);

            const auto it = TokenMap.find(token.type());
            if (it == std::end(TokenMap)) {
                throw ParserException(token.line(), token.column(), "Unhandled unary operator: " + tokenName(token.type()));
            } else {
                const auto op = it->second;
                return EL::Expression(EL::UnaryExpression(op, parseSimpleTermOrSwitch()), token.line(), token.column());
            }
        }

        EL::Expression ELParser::parseSwitch() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::DoubleOBrace, token);

            const size_t startLine = token.line();
            const size_t startColumn = token.column();
            std::vector<EL::Expression> subExpressions;

            token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm | ELToken::DoubleCBrace, token);

            if (token.hasType(ELToken::SimpleTerm)) {
                do {
                    subExpressions.push_back(parseExpression());
                } while (expect(ELToken::Comma | ELToken::DoubleCBrace, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else if (token.hasType(ELToken::DoubleCBrace)) {
                m_tokenizer.nextToken();
            }

            return EL::Expression(EL::SwitchExpression(std::move(subExpressions)), startLine, startColumn);
        }

        EL::Expression ELParser::parseCompoundTerm(EL::Expression lhs) {
            static const auto TokenMap = std::unordered_map<ELToken::Type, EL::BinaryOperator>{
                { ELToken::Addition, EL::BinaryOperator::Addition },
                { ELToken::Subtraction, EL::BinaryOperator::Subtraction },
                { ELToken::Multiplication, EL::BinaryOperator::Multiplication },
                { ELToken::Division, EL::BinaryOperator::Division },
                { ELToken::Modulus, EL::BinaryOperator::Modulus },
                { ELToken::LogicalAnd, EL::BinaryOperator::LogicalAnd },
                { ELToken::LogicalOr, EL::BinaryOperator::LogicalOr },
                { ELToken::BitwiseAnd, EL::BinaryOperator::BitwiseAnd },
                { ELToken::BitwiseXOr, EL::BinaryOperator::BitwiseXOr },
                { ELToken::BitwiseOr, EL::BinaryOperator::BitwiseOr },
                { ELToken::BitwiseShiftLeft, EL::BinaryOperator::BitwiseShiftLeft },
                { ELToken::BitwiseShiftRight, EL::BinaryOperator::BitwiseShiftRight },
                { ELToken::Less, EL::BinaryOperator::Less },
                { ELToken::LessOrEqual, EL::BinaryOperator::LessOrEqual },
                { ELToken::Greater, EL::BinaryOperator::Greater },
                { ELToken::GreaterOrEqual, EL::BinaryOperator::GreaterOrEqual },
                { ELToken::Equal, EL::BinaryOperator::Equal },
                { ELToken::NotEqual, EL::BinaryOperator::NotEqual },
                { ELToken::Range, EL::BinaryOperator::Range },
                { ELToken::Case, EL::BinaryOperator::Case },
            };
        
            while (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm)) {
                Token token = m_tokenizer.nextToken();
                expect(ELToken::CompoundTerm, token);

                const auto it = TokenMap.find(token.type());
                if (it == std::end(TokenMap)) {
                    throw ParserException(token.line(), token.column(), "Unhandled binary operator: " + tokenName(token.type()));
                } else {
                    const auto op = it->second;
                    lhs = EL::Expression(EL::BinaryExpression(op, std::move(lhs), parseSimpleTermOrSwitch()), token.line(), token.column());
                }
            }

            return lhs;
        }

        ELParser::TokenNameMap ELParser::tokenNames() const {
            TokenNameMap result;
            result[ELToken::Name]               = "variable";
            result[ELToken::String]             = "string";
            result[ELToken::Number]             = "number";
            result[ELToken::Boolean]            = "boolean";
            result[ELToken::OBracket]           = "'['";
            result[ELToken::CBracket]           = "']'";
            result[ELToken::OBrace]             = "'{'";
            result[ELToken::CBrace]             = "'}'";
            result[ELToken::OParen]             = "'('";
            result[ELToken::CParen]             = "')'";
            result[ELToken::Addition]           = "'+'";
            result[ELToken::Subtraction]        = "'-'";
            result[ELToken::Multiplication]     = "'*'";
            result[ELToken::Division]           = "'/'";
            result[ELToken::Modulus]            = "'%'";
            result[ELToken::Colon]              = "':'";
            result[ELToken::Comma]              = "','";
            result[ELToken::Range]              = "'..'";
            result[ELToken::LogicalNegation]    = "'!'";
            result[ELToken::LogicalAnd]         = "'&&'";
            result[ELToken::LogicalOr]          = "'||'";
            result[ELToken::Less]               = "'<'";
            result[ELToken::LessOrEqual]        = "'<='";
            result[ELToken::Equal]              = "'=='";
            result[ELToken::NotEqual]           = "'!='";
            result[ELToken::GreaterOrEqual]     = "'>='";
            result[ELToken::Greater]            = "'>'";
            result[ELToken::Case]               = "'->'";
            result[ELToken::BitwiseNegation]    = "'~'";
            result[ELToken::BitwiseAnd]         = "'&'";
            result[ELToken::BitwiseOr]          = "'|'";
            result[ELToken::BitwiseShiftLeft]   = "'<<'";
            result[ELToken::BitwiseShiftRight]  = "'>>'";
            result[ELToken::DoubleOBrace]       = "'{{'";
            result[ELToken::DoubleCBrace]       = "'}}'";
            result[ELToken::Null]               = "'null'";
            result[ELToken::Eof]                = "end of file";
            return result;
        }
    }
}
