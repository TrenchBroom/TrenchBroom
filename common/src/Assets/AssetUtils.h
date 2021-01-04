/*
 Copyright (C) 2010-2020 Kristian Duske

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

#pragma once

#include "Logger.h"
#include "Assets/ModelDefinition.h"
#include "EL/ELExceptions.h"

#include <string_view>

namespace TrenchBroom {
    namespace Assets {
        /**
         * Evaluates the given lambda and returns the resulting model specification. If an EL exception is thrown by
         * the given lambda, it is caught and an error message is logged using the given logger.
         *
         * @tparam GetModelSpec the type of the given lambda
         * @param logger the logger to log errors to
         * @param classname the classname to use when logging errors
         * @param getModelSpec the lambda to evaluate, must return a ModelSpecification
         * @return the model specification returned by the lambda, or an empty model specification if the lambda throws
         * an EL exception
         */
        template <typename GetModelSpec>
        ModelSpecification safeGetModelSpecification(Logger& logger, std::string_view classname, GetModelSpec getModelSpec) {
            try {
                return getModelSpec();
            } catch (const EL::Exception& e) {
                logger.error() << "Could not get entity model for entity '" << classname << "': " << e.what();
                return ModelSpecification();
            }
        }
    }
}

