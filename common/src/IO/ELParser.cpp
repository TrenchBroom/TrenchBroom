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

#include <sstream>
#include <string>

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
                        return Token(ELToken::BitwiseXor, c, c+1, offset(c), startLine, startColumn);
                    case '!':
                        advance();
                        if (curChar() == '=') {
                            advance();
                            return Token(ELToken::Inequal, c, c+2, offset(c), startLine, startColumn);
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
            const auto result = EL::Expression(parseExpression());
            if (m_mode == Mode::Strict) {
                expect(ELToken::Eof, m_tokenizer.peekToken()); // avoid trailing garbage
            }
            return result;
        }

        EL::ExpressionBase* ELParser::parseExpression() {
            if (m_tokenizer.peekToken().hasType(ELToken::OParen)) {
                return parseGroupedTerm();
            } else {
                return parseTerm();
            }
        }

        EL::ExpressionBase* ELParser::parseGroupedTerm() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::OParen, token);
            EL::ExpressionBase* expression = parseTerm();
            expect(ELToken::CParen, m_tokenizer.nextToken());

            EL::ExpressionBase* lhs = EL::GroupingOperator::create(expression, token.line(), token.column());
            if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
                return parseCompoundTerm(lhs);
            return lhs;
        }

        EL::ExpressionBase* ELParser::parseTerm() {
            expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, m_tokenizer.peekToken());

            EL::ExpressionBase* lhs = parseSimpleTermOrSwitch();
            if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
                return parseCompoundTerm(lhs);
            return lhs;
        }

        EL::ExpressionBase* ELParser::parseSimpleTermOrSwitch() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, token);

            if (token.hasType(ELToken::SimpleTerm))
                return parseSimpleTerm();
            return parseSwitch();
        }

        EL::ExpressionBase* ELParser::parseSimpleTerm() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm, token);

            EL::ExpressionBase* term = nullptr;
            if (token.hasType(ELToken::UnaryOperator))
                term = parseUnaryOperator();
            else if (token.hasType(ELToken::OParen))
                term = parseGroupedTerm();
            else if (token.hasType(ELToken::Name))
                term = parseVariable();
            else
                term = parseLiteral();

            while (m_tokenizer.peekToken().hasType(ELToken::OBracket))
                term = parseSubscript(term);

            return term;
        }

        EL::ExpressionBase* ELParser::parseSubscript(EL::ExpressionBase* lhs) {
            Token token = m_tokenizer.nextToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            expect(ELToken::OBracket, token);
            EL::ExpressionBase::List elements;
            if (!m_tokenizer.peekToken().hasType(ELToken::CBracket)) {
                do {
                    elements.emplace_back(parseExpressionOrAnyRange());
                } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else {
                m_tokenizer.nextToken();
            }

            if (elements.size() == 1)
                return EL::SubscriptOperator::create(lhs, elements.front().release(), startLine, startColumn);
            return EL::SubscriptOperator::create(lhs, EL::ArrayExpression::create(std::move(elements), startLine, startColumn), startLine, startColumn);
        }

        EL::ExpressionBase* ELParser::parseVariable() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::Name, token);
            return EL::VariableExpression::create(token.data(), token.line(), token.column());
        }

        EL::ExpressionBase* ELParser::parseLiteral() {
            Token token = m_tokenizer.peekToken();
            expect(ELToken::Literal | ELToken::OBracket | ELToken::OBrace, token);

            if (token.hasType(ELToken::String)) {
                m_tokenizer.nextToken();
                // Escaping happens in EL::Value::appendToStream
                const std::string value = kdl::str_unescape(token.data(), "\\\"");
                return EL::LiteralExpression::create(EL::Value(value), token.line(), token.column());
            }
            if (token.hasType(ELToken::Number)) {
                m_tokenizer.nextToken();
                return EL::LiteralExpression::create(EL::Value(token.toFloat<EL::NumberType>()), token.line(), token.column());
            }
            if (token.hasType(ELToken::Boolean)) {
                m_tokenizer.nextToken();
                return EL::LiteralExpression::create(EL::Value(token.data() == "true"), token.line(), token.column());
            }
            if (token.hasType(ELToken::Null)) {
                m_tokenizer.nextToken();
                return EL::LiteralExpression::create(EL::Value::Null, token.line(), token.column());
            }

            if (token.hasType(ELToken::OBracket))
                return parseArray();
            return parseMap();
        }

        EL::ExpressionBase* ELParser::parseArray() {
            Token token = m_tokenizer.nextToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            expect(ELToken::OBracket, token);
            EL::ExpressionBase::List elements;
            if (!m_tokenizer.peekToken().hasType(ELToken::CBracket)) {
                do {
                    elements.emplace_back(parseExpressionOrRange());
                } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else {
                m_tokenizer.nextToken();
            }

            return EL::ArrayExpression::create(std::move(elements), startLine, startColumn);
        }

        EL::ExpressionBase* ELParser::parseExpressionOrRange() {
            EL::ExpressionBase* expression = parseExpression();
            if (m_tokenizer.peekToken().hasType(ELToken::Range)) {
                Token token = m_tokenizer.nextToken();
                expression = EL::RangeOperator::create(expression, parseExpression(), token.line(), token.column());
            }

            return expression;
        }

        EL::ExpressionBase* ELParser::parseExpressionOrAnyRange() {
            EL::ExpressionBase* expression = nullptr;
            if (m_tokenizer.peekToken().hasType(ELToken::Range)) {
                Token token = m_tokenizer.nextToken();
                expression = EL::RangeOperator::createAutoRangeWithRightOperand(parseExpression(), token.line(), token.column());
            } else {
                expression = parseExpression();
                if (m_tokenizer.peekToken().hasType(ELToken::Range)) {
                    Token token = m_tokenizer.nextToken();
                    if (m_tokenizer.peekToken().hasType(ELToken::SimpleTerm))
                        expression = EL::RangeOperator::create(expression, parseExpression(), token.line(), token.column());
                    else
                        expression = EL::RangeOperator::createAutoRangeWithLeftOperand(expression, token.line(), token.column());
                }
            }

            return expression;
        }

        EL::ExpressionBase* ELParser::parseMap() {
            EL::ExpressionBase::Map elements;

            Token token = m_tokenizer.nextToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            expect(ELToken::OBrace, token);
            if (!m_tokenizer.peekToken().hasType(ELToken::CBrace)) {
                do {
                    token = m_tokenizer.nextToken();
                    expect(ELToken::String | ELToken::Name, token);
                    const std::string key = token.data();

                    expect(ELToken::Colon, m_tokenizer.nextToken());
                    elements[key] = std::unique_ptr<EL::ExpressionBase>(parseExpression());
                } while (expect(ELToken::Comma | ELToken::CBrace, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else {
                m_tokenizer.nextToken();
            }

            return EL::MapExpression::create(std::move(elements), startLine, startColumn);
        }

        EL::ExpressionBase* ELParser::parseUnaryOperator() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::UnaryOperator, token);

            if (token.hasType(ELToken::Addition))
                return EL::UnaryPlusOperator::create(parseSimpleTermOrSwitch(), token.line(), token.column());
            else if (token.hasType(ELToken::Subtraction))
                return EL::UnaryMinusOperator::create(parseSimpleTermOrSwitch(), token.line(), token.column());
            else if (token.hasType(ELToken::LogicalNegation))
                return EL::LogicalNegationOperator::create(parseSimpleTermOrSwitch(), token.line(), token.column());
            else if (token.hasType(ELToken::BitwiseNegation))
                return EL::BitwiseNegationOperator::create(parseSimpleTermOrSwitch(), token.line(), token.column());
            else
                throw ParserException(token.line(), token.column(), "Unhandled unary operator: " + tokenName(token.type()));
        }

        EL::ExpressionBase* ELParser::parseSwitch() {
            Token token = m_tokenizer.nextToken();
            expect(ELToken::DoubleOBrace, token);

            const size_t startLine = token.line();
            const size_t startColumn = token.column();
            EL::ExpressionBase::List subExpressions;

            token = m_tokenizer.peekToken();
            expect(ELToken::SimpleTerm | ELToken::DoubleCBrace, token);

            if (token.hasType(ELToken::SimpleTerm)) {
                do {
                    subExpressions.emplace_back(parseExpression());
                } while (expect(ELToken::Comma | ELToken::DoubleCBrace, m_tokenizer.nextToken()).hasType(ELToken::Comma));
            } else if (token.hasType(ELToken::DoubleCBrace)) {
                m_tokenizer.nextToken();
            }

            return EL::SwitchOperator::create(std::move(subExpressions), startLine, startColumn);
        }

        EL::ExpressionBase* ELParser::parseCompoundTerm(EL::ExpressionBase* lhs) {
            while (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm)) {
                Token token = m_tokenizer.nextToken();
                expect(ELToken::CompoundTerm, token);

                if (token.hasType(ELToken::Addition))
                    lhs = EL::AdditionOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Subtraction))
                    lhs = EL::SubtractionOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Multiplication))
                    lhs = EL::MultiplicationOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Division))
                    lhs = EL::DivisionOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Modulus))
                    lhs = EL::ModulusOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::LogicalAnd))
                    lhs = EL::LogicalAndOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::LogicalOr))
                    lhs = EL::LogicalOrOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Less))
                    lhs = EL::ComparisonOperator::createLess(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::LessOrEqual))
                    lhs = EL::ComparisonOperator::createLessOrEqual(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Equal))
                    lhs = EL::ComparisonOperator::createEqual(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Inequal))
                    lhs = EL::ComparisonOperator::createInequal(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::GreaterOrEqual))
                    lhs = EL::ComparisonOperator::createGreaterOrEqual(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Greater))
                    lhs = EL::ComparisonOperator::createGreater(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::BitwiseAnd))
                    lhs = EL::BitwiseAndOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::BitwiseXor))
                    lhs = EL::BitwiseXorOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::BitwiseOr))
                    lhs = EL::BitwiseOrOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::BitwiseShiftLeft))
                    lhs = EL::BitwiseShiftLeftOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::BitwiseShiftRight))
                    lhs = EL::BitwiseShiftRightOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else if (token.hasType(ELToken::Case))
                    lhs = EL::CaseOperator::create(lhs, parseSimpleTermOrSwitch(), token.line(), token.column());
                else
                    throw ParserException(token.line(), token.column(), "Unhandled binary operator: " + tokenName(token.type()));
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
            result[ELToken::Inequal]            = "'!='";
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
