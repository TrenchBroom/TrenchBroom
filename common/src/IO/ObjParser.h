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

#ifndef TRENCHBROOM_OBJPARSER_H
#define TRENCHBROOM_OBJPARSER_H

#include "IO/EntityModelParser.h"
#include "IO/Path.h"

#include <vecmath/forward.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class Texture;
    }

    namespace IO {
        class FileSystem;

        class ObjParser : public EntityModelParser {
        private:
            std::string m_name;
            std::string m_text;
        public:
            /**
             * Creates a new parser for Wavefront OBJ models.
             *
             * @param name the name of the model
             * @param begin the start of the text
             * @param end the end of the text
             */
            ObjParser(const std::string& name, const char* begin, const char* end);

            /**
             * Transforms the various sets of coordinates.
             * Returns true to reverse vertex order (needed for switching between left/right-handed transforms)
             * @param positions Vertex positions to transform in-place.
             * @param texcoords Texture coordinates to transform in-place.
             */
            virtual bool transformObjCoordinateSet(std::vector<vm::vec3f>& positions, std::vector<vm::vec2f>& texcoords) = 0;

            /**
             * Loads a material. On failure, return the empty unique_ptr (as the original exceptions are usually caught anyway to test each format).
             * @param name The name of the material.
             */
            virtual std::unique_ptr<Assets::Texture> loadMaterial(const std::string& name) = 0;

            /**
             * Loads the "fallback material". This is used if no material is specified or if loadMaterial fails.
             * This function is not supposed to fail in any way. Should it still fail regardless, it should throw a ParserException.
             */
            virtual std::unique_ptr<Assets::Texture> loadFallbackMaterial() = 0;

        private:
            std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
        };

        /**
         * The specific instantiation of the ObjParser as it applies to Neverball.
         */
        class NvObjParser : public ObjParser {
        private:
            Path m_path;
            const FileSystem& m_fs;
        public:
            /**
             *
             * @param path the path of the model (important for texture lookup)
             * @param begin the start of the text
             * @param end the end of the text
             * @param fs the filesystem used to lookup textures
             */
            NvObjParser(const Path& path, const char* begin, const char* end, const FileSystem& fs);

            bool transformObjCoordinateSet(std::vector<vm::vec3f>& positions, std::vector<vm::vec2f>& texcoords) override;
            std::unique_ptr<Assets::Texture> loadMaterial(const std::string& name) override;
            std::unique_ptr<Assets::Texture> loadFallbackMaterial() override;
        };
    }
}


#endif //TRENCHBROOM_ASEPARSER_H
