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

#include "ObjParser.h"

#include "Logger.h"
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderTextureReader.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

#include <vecmath/forward.h>
#include <kdl/string_utils.h>

#include <functional>
#include <string>

namespace TrenchBroom {
    namespace IO {

        ObjVertexRef::ObjVertexRef(const std::string& text) : m_texcoord(0) {
            auto components = kdl::str_split(text, "/");
            std::vector<size_t> components_num;
            for (const std::string & com : components)
                components_num.push_back((size_t) std::stoul(com));
            if (components_num.size() == 0) {
                throw ParserException("OBJ file has an empty vertex reference");
            } else {
                m_position = components_num[0];
            }
            if (components_num.size() >= 2)
                m_texcoord = components_num[1];
        }

        ObjParser::ObjParser(const std::string& name, const char* begin, const char* end, const FileSystem& fs) :
        m_name(name), m_text(begin, (size_t) (end - begin)), m_fs(fs) {}

        std::unique_ptr<Assets::EntityModel> ObjParser::doInitializeModel(Logger& logger) {
            // Model construction prestart (skins are added to this mid-parse)
            auto model = std::make_unique<Assets::EntityModel>(m_name);
            model->addFrames(1);
            auto& surface = model->addSurface(m_name);

            IO::FreeImageTextureReader imageReader(IO::TextureReader::StaticNameStrategy(""));
            surface.addSkin(imageReader.readTexture(m_fs.openFile(IO::Path("textures/__TB_empty.png"))));

            // 'v': Defines a 3D position.
            std::vector<vm::vec3f> positions;
            std::vector<vm::vec2f> texcoords;
            std::vector<ObjFace> faces;
            // Begin parsing.
            auto lines = kdl::str_split(m_text, "\n");
            size_t current_material = 0;
            size_t last_material = 0;
            for (const std::string& line : lines) {
                // logger.debug() << "obj line: " << line;
                std::string trimmed = kdl::str_trim(line);
                auto tokens = kdl::str_split(trimmed, " \t");
                if (tokens.size() != 0) {
                    if (tokens[0] == "v") {
                        if (tokens.size() < 4)
                            throw ParserException("OBJ file has a vertex with too few dimensions");
                        // This can and should be replaced with a less Neverball-specific transform
                        positions.push_back(vm::vec3f(std::stof(tokens[1]) * 64, std::stof(tokens[3]) * 64, std::stof(tokens[2]) * 64));
                    } else if (tokens[0] == "vt") {
                        if (tokens.size() < 3)
                            throw ParserException("OBJ file has a texcoord with too few dimensions");
                        texcoords.push_back(vm::vec2f(std::stof(tokens[1]), std::stof(tokens[2])));
                    } else if (tokens[0] == "usemtl") {
                        if (tokens.size() < 2)
                            throw ParserException("OBJ file has usemtl with no material name");
                        // NOTE: This can and should be replaced with something less Neverball-specific.
                        // A reasonable solution here would be to use the same material handling as the brushes unless otherwise required.
                        // But there's raw pointers all over the Texture system, so I can't really trust it...
                        std::unique_ptr<Assets::Texture> tex;
                        try {
                            tex = std::unique_ptr<Assets::Texture> (imageReader.readTexture(m_fs.openFile(IO::Path("textures/" + tokens[1] + ".png"))));
                        } catch (const Exception& /*ex1*/) {
                            try {
                                tex = std::unique_ptr<Assets::Texture> (imageReader.readTexture(m_fs.openFile(IO::Path("textures/" + tokens[1] + ".jpg"))));
                            } catch (const Exception& /*ex2*/) {
                                current_material = 0;
                                logger.warn() << "unable to find Neverball model material " << tokens[1];
                            }
                        }
                        if (tex) {
                            surface.addSkin(tex.release());
                            last_material++;
                            current_material = last_material;
                        }
                    } else if (tokens[0] == "f") {
                        ObjFace face;
                        face.m_material = current_material;
                        for (size_t i = 1; i < tokens.size(); i++)
                            face.m_vertices.push_back(ObjVertexRef(tokens[i]));
                        faces.push_back(face);
                    }
                }
            }
            // Done parsing; setup bounds.
            auto bounds = vm::bbox3f::builder();
            if (positions.size() == 0) {
                // passing empty bounds as bbox crashes the program, don't let it happen
                throw ParserException("OBJ file has no vertices (so no valid bounding box)");
            } else {
                bounds.add(std::begin(positions), std::end(positions));
            }
            // Model construction prestart
            // Begin model construction, part 1. Collation
            size_t totalVertexCount = 0;
            Renderer::TexturedIndexRangeMap::Size size;
            for (const ObjFace & face : faces) {
                size.inc(surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
                totalVertexCount += face.m_vertices.size();
            }
            // Model construction, part 2. Building
            auto& frame = model->loadFrame(0, m_name, bounds.bounds());
            Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type> builder(totalVertexCount, size);
            // Actual build {
            for (const ObjFace & face : faces) {
                std::vector<Assets::EntityModelVertex> vertices;
                for (const ObjVertexRef & ref : face.m_vertices) {
                    size_t point = ref.m_position;
                    if (point == 0)
                        throw ParserException("OBJ file has vertex with no position (was this generated/parsed correctly?)");
                    point--;
                    if (point >= positions.size())
                        throw ParserException("OBJ file has vertex referring to a position that hasn't been defined");
                    vm::vec2f texcoord = vm::vec2f(0, 0);
                    if (ref.m_texcoord != 0) {
                        size_t c = ref.m_texcoord - 1;
                        if (c >= texcoords.size())
                            throw ParserException("OBJ file has vertex referring to a texcoord that hasn't been defined");
                        texcoord = texcoords[c];
                    }
                    vertices.push_back(Assets::EntityModelVertex(positions[point], texcoord));
                }
                builder.addPolygon(surface.skin(face.m_material), vertices);
            }
            // }
            surface.addTexturedMesh(frame, builder.vertices(), builder.indices());
            return model;
        }
    }
}
