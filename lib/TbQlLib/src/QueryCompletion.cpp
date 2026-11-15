/*
 Copyright (C) 2026 Kristian Duske

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

#include "ql/QueryCompletion.h"

#include "base/ParserException.h"
#include "base/Result.h"
#include "el/Expression.h"
#include "el/ExpressionNode.h"
#include "el/ParseExpression.h"
#include "el/ParseMode.h"
#include "el/Parser.h"
#include "ql/Query.h"
#include "ql/QueryDomainInference.h"

#include "kd/contracts.h"
#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/string_compare.h"
#include "kd/string_utils.h"
#include "kd/unpack.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace tb::ql
{
namespace
{

// the range of the name that contains or touches `position`, or an empty range at
// `position` if there is no such name
std::tuple<size_t, size_t> nameRangeAt(const std::string_view text, const size_t position)
{
  const auto nameCharsBefore = std::ranges::distance(
    text.substr(0, position) | std::views::reverse
    | std::views::take_while(el::isNameChar));
  const auto nameCharsAfter =
    std::ranges::distance(text.substr(position) | std::views::take_while(el::isNameChar));

  return {position - size_t(nameCharsBefore), position + size_t(nameCharsAfter)};
}

struct CompletionToken
{
  el::ElToken::Type type;
  std::string data;
};

/**
 * Returns the tokens of `text` if a name appended to it would be a token of its own.
 *
 * Returns nothing otherwise, such as if `text` ends within a string or a comment, or if
 * `text` can't be tokenized.
 */
std::optional<std::vector<CompletionToken>> tokenizeBeforeName(
  const std::string_view text)
{
  const auto textWithName = fmt::format("{}x", text);
  auto tokenizer = el::Tokenizer{textWithName, 1, 1};
  auto tokens = std::vector<CompletionToken>{};

  try
  {
    for (auto token = tokenizer.nextToken(); !token.hasType(el::ElToken::Eof);
         token = tokenizer.nextToken())
    {
      if (token.position() == text.size())
      {
        return tokens;
      }
      tokens.push_back({token.type(), token.data()});
    }
  }
  catch (const ParserException&)
  {
  }

  return std::nullopt;
}

/**
 * Returns the delimiters that close the parentheses, brackets, braces and double braces
 * left open by the given tokens, innermost first and separated by spaces, or nothing if
 * a closing delimiter doesn't match.
 */
std::optional<std::string> closingDelimiters(const std::vector<CompletionToken>& tokens)
{
  auto closers = std::vector<std::string>{};
  for (const auto& token : tokens)
  {
    switch (token.type)
    {
    case el::ElToken::OParen:
      closers.emplace_back(")");
      break;
    case el::ElToken::OBracket:
      closers.emplace_back("]");
      break;
    case el::ElToken::OBrace:
      closers.emplace_back("}");
      break;
    case el::ElToken::DoubleOBrace:
      closers.emplace_back("}}");
      break;
    case el::ElToken::CParen:
    case el::ElToken::CBracket:
    case el::ElToken::CBrace:
    case el::ElToken::DoubleCBrace:
      if (closers.empty() || closers.back() != token.data)
      {
        return std::nullopt;
      }
      closers.pop_back();
      break;
    default:
      break;
    }
  }

  return fmt::format("{}", fmt::join(closers | std::views::reverse, " "));
}

/**
 * Parses `text` followed by `word` and `closers`, which close the delimiters left open in
 * `text`. If `word` completes a term, this succeeds exactly if `text` followed by `word`
 * is the beginning of a valid expression, since a delimiter is the only construct that is
 * still open after a complete term.
 */
Result<el::ExpressionNode> parseWithWord(
  const std::string_view text,
  const std::string_view word,
  const std::string_view closers)
{
  return el::parseExpression(
    el::ParseMode::Strict, fmt::format("{}{} {}", text, word, closers));
}

// see parseQuery's doc comment for where type names don't need quotes
bool isTypeNamePosition(const std::string_view text, const std::string_view closers)
{
  // a type name is quoted if the query is the same as with a string in its place
  const auto withTypeName =
    parseWithWord(text, "brush", closers) | kdl::transform(quoteTypeNames);
  const auto withString =
    parseWithWord(text, R"("brush")", closers) | kdl::transform(quoteTypeNames);
  return withTypeName.is_success() && withString.is_success()
         && withTypeName.value() == withString.value();
}

// the names of the built-in functions that are usually called with the given notation
std::vector<std::string> builtinFunctionNames(const el::FunctionNotation notation)
{
  return el::builtinFunctions()
         | std::views::filter(kdl::unpack([&](const auto&, const auto& function) {
             return function.notation == notation;
           }))
         | std::views::keys | kdl::ranges::to<std::vector>();
}

// see completeQuery's doc comment for which words are offered where; `text` is the text
// before the name at the cursor
std::vector<std::string> completionWords(const std::string_view text)
{
  const auto tokens = tokenizeBeforeName(text);
  if (!tokens)
  {
    // a word within a string or a comment is part of it rather than a name, and text
    // that can't be tokenized isn't the beginning of a valid query
    return {};
  }

  const auto closers = closingDelimiters(*tokens);
  if (!closers)
  {
    // a closing delimiter doesn't match an open one, so the text isn't the beginning of a
    // valid query
    return {};
  }

  if (isTypeNamePosition(text, *closers))
  {
    // other words would be valid here, but the value of `type` is always an object type
    // name, so nothing else is useful to compare it with
    return fullDomain() | std::views::transform([](const auto type) {
             return kdl::str_to_string(type);
           })
           | kdl::ranges::to<std::vector>();
  }

  auto words = std::vector<std::string>{};

  // unlike a name, which can also follow a dot or be the key of a map, a call can only
  // begin a term
  if (parseWithWord(text, "x()", *closers).is_success())
  {
    words = kdl::views::concat(
              queryFields() | std::views::keys,
              el::keywords(),
              builtinFunctionNames(el::FunctionNotation::Prefix))
            | kdl::ranges::to<std::vector>();
  }

  // `is` stands for any infix function
  if (parseWithWord(text, "is x", *closers).is_success())
  {
    const auto infixFunctionNames = builtinFunctionNames(el::FunctionNotation::Infix);
    words.insert(words.end(), infixFunctionNames.begin(), infixFunctionNames.end());
  }

  return words;
}

} // namespace

kdl_reflect_impl(QueryCompletion);

QueryCompletion completeQuery(
  const std::string_view inputText, const size_t cursorPosition)
{
  contract_pre(cursorPosition <= inputText.size());

  const auto [start, end] = nameRangeAt(inputText, cursorPosition);

  // a name can't begin with a digit
  if (start < end && !el::isNameStartChar(inputText[start]))
  {
    return {start, end, {}};
  }

  const auto prefix = inputText.substr(start, cursorPosition - start);
  const auto isPrefix = [&](const auto& word) {
    return kdl::ci::str_is_prefix(word, prefix);
  };

  auto words = completionWords(inputText.substr(0, start)) | std::views::filter(isPrefix)
               | kdl::ranges::to<std::vector>();
  std::ranges::sort(words, kdl::ci::string_less{});

  return {start, end, std::move(words)};
}

} // namespace tb::ql
