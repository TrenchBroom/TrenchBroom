/*
 Copyright (C) 2010 Kristian Duske

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

#include "Interpolator.h"

#include "el/Expression.h"
#include "el/Value.h"

#include <sstream>
#include <string>

namespace tb::el
{

Interpolator::Interpolator(const std::string_view str)
  : ELParser{ELParser::Mode::Lenient, str}
{
}

std::string Interpolator::interpolate(const EvaluationContext& context)
{
  auto result = std::stringstream{};
  while (!m_tokenizer.eof())
  {
    m_tokenizer.appendUntil("${", result);
    if (!m_tokenizer.eof())
    {
      const auto expression = parse();
      result
        << expression.evaluate(context).convertTo(el::ValueType::String).stringValue();
      expect(IO::ELToken::CBrace, m_tokenizer.nextToken());
    }
  }

  return result.str();
}

std::string interpolate(const std::string_view str, const EvaluationContext& context)
{
  auto interpolator = Interpolator{str};
  return interpolator.interpolate(context);
}

} // namespace tb::el
