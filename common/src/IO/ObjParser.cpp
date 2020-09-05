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
#include "IO/ResourceUtils.h"
#include "Model/BrushFaceAttributes.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

#include <vecmath/forward.h>
#include <kdl/string_utils.h>

#include <functional>
#include <string>

namespace TrenchBroom {
    namespace IO {
        struct ObjVertexRef {
            /**
             * Parses a vertex reference.
             *
             * @param text the text of this reference (such as "1/2/3")
             */
            explicit ObjVertexRef(const std::string& text) : m_texcoord(0) {
                const auto components = kdl::str_split(text, "/");
                std::vector<size_t> components_num;
                for (const std::string& com : components) {
                    const auto csize = kdl::str_to_size(com);
                    if (csize) {
                        components_num.push_back(*csize);
                    } else {
                        throw ParserException("OBJ file has invalid number in vertex reference");
                    }
                }
                if (components_num.size() == 0) {
                    throw ParserException("OBJ file has an empty vertex reference");
                } else if (components_num.size() == 1) {
                    m_position = components_num[0];
                } else {
                    m_position = components_num[0];
                    m_texcoord = components_num[1];
                }
            }

            // Position index (1-based. Should always be present.)
            size_t m_position;
            // Texture coordinate index (Also 1-based. If not present, 0.)
            size_t m_texcoord;
        };

        struct ObjFace {
            // The material of this face (as a skin index)
            size_t m_material;
            // The vertices of this face.
            std::vector<ObjVertexRef> m_vertices;
        };

        ObjParser::ObjParser(const std::string& name, const char* begin, const char* end) :
        m_name(name), m_text(begin, end) {}

        std::unique_ptr<Assets::EntityModel> ObjParser::doInitializeModel(Logger& logger) {
            // Model construction prestart (skins are added to this mid-parse)
            auto model = std::make_unique<Assets::EntityModel>(m_name, Assets::PitchType::Normal);
            model->addFrames(1);
            auto& surface = model->addSurface(m_name);

            std::vector<Assets::Texture> textures;

            // Load the default material (skin 0) ; must be present as a default for materialless faces
            // This default skin is used for all unloadable textures and all unspecified textures.
            // As such this implicitly covers situations where the default skin is intended to be used, but is manually specified incorrectly.
            if (auto fallbackMaterial = loadFallbackMaterial(logger)) {
                textures.push_back(std::move(*fallbackMaterial));
            }
            
            // Define the various OBJ parsing state.
            std::vector<vm::vec3f> positions;
            std::vector<vm::vec2f> texcoords;
            std::vector<ObjFace> faces;
            
            // Begin parsing.
            size_t current_material = 0;
            size_t last_material = 0;
            for (const std::string& line : kdl::str_split(m_text, "\n")) {
                // logger.debug() << "obj line: " << line;
                const std::string trimmed = kdl::str_trim(line);
                const auto tokens = kdl::str_split(trimmed, " \t");
                if (tokens.size() != 0) {
                    if (tokens[0] == "v") {
                        if (tokens.size() < 4) {
                            throw ParserException("OBJ file has a vertex with too few dimensions");
                        }
                        // This can and should be replaced with a less Neverball-specific transform
                        positions.push_back(vm::vec3f(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])));
                    } else if (tokens[0] == "vt") {
                        if (tokens.size() < 3) {
                            throw ParserException("OBJ file has a texcoord with too few dimensions");
                        }
                        texcoords.push_back(vm::vec2f(std::stof(tokens[1]), std::stof(tokens[2])));
                    } else if (tokens[0] == "usemtl") {
                        if (tokens.size() < 2) {
                            // Assume they meant "use default material" (just in case; this doesn't really make sense, but...)
                            current_material = 0;
                        } else {
                            auto tex = loadMaterial(tokens[1], logger);
                            if (tex) {
                                textures.push_back(std::move(*tex));
                                ++last_material;
                                current_material = last_material;
                            } else {
                                current_material = 0;
                                logger.warn() << "unable to find OBJ model material " << tokens[1];
                            }
                        }
                    } else if (tokens[0] == "f") {
                        ObjFace face;
                        face.m_material = current_material;
                        for (size_t i = 1; i < tokens.size(); ++i) {
                            face.m_vertices.push_back(ObjVertexRef(tokens[i]));
                        }
                        faces.push_back(face);
                    }
                }
            }
            surface.setSkins(std::move(textures));
            
            // Done parsing; transform (and get the 'reverse' flag for future use)
            const bool reverse = transformObjCoordinateSet(positions, texcoords);
            // Everything's in TrenchBroom Relative Coordinates! Build bounds.
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
            for (const ObjFace& face : faces) {
                size.inc(surface.skin(face.m_material), Renderer::PrimType::Polygon, face.m_vertices.size());
                totalVertexCount += face.m_vertices.size();
            }
            // Model construction, part 2. Building
            auto& frame = model->loadFrame(0, m_name, bounds.bounds());
            Renderer::TexturedIndexRangeMapBuilder<Assets::EntityModelVertex::Type> builder(totalVertexCount, size);
            // Actual build {
            for (const ObjFace &face : faces) {
                std::vector<Assets::EntityModelVertex> vertices;
                for (const ObjVertexRef& ref : face.m_vertices) {
                    size_t point = ref.m_position;
                    if (point == 0) {
                        throw ParserException("OBJ file has vertex with no position (was this generated/parsed correctly?)");
                    }
                    // As previously stated, OBJ file indexes are 1-based. This converts it to a 0-based index.
                    --point;
                    if (point >= positions.size()) {
                        throw ParserException("OBJ file has vertex referring to a position that hasn't been defined");
                    }
                    vm::vec2f texcoord = vm::vec2f(0, 0);
                    if (ref.m_texcoord != 0) {
                        const size_t c = ref.m_texcoord - 1;
                        if (c >= texcoords.size()) {
                            throw ParserException("OBJ file has vertex referring to a texcoord that hasn't been defined");
                        }
                        texcoord = texcoords[c];
                    }
                    if (reverse) {
                        vertices.insert(vertices.begin(), Assets::EntityModelVertex(positions[point], texcoord));
                    } else {
                        vertices.push_back(Assets::EntityModelVertex(positions[point], texcoord));
                    }
                }
                builder.addPolygon(surface.skin(face.m_material), vertices);
            }
            // }
            surface.addTexturedMesh(frame, builder.vertices(), builder.indices());
            return model;
        }

        // -- Neverball --

        NvObjParser::NvObjParser(const Path& path, const char* begin, const char* end, const FileSystem& fs) :
        ObjParser(path.lastComponent().asString(), begin, end),
        m_path(path),
        m_fs(fs) {}

        bool NvObjParser::transformObjCoordinateSet(std::vector<vm::vec3f>& positions, std::vector<vm::vec2f>& texcoords) {
            for (vm::vec3f& pos : positions) {
                // The transform we want to perform is OBJ-To-MAP.
                // The transform used in make_body is MAP-To-OBJ, as Neverball uses the OBJ coordinate space natively.
                // The output is (X, Z, -Y); thus the inverse transform is (X, -Z, Y)
                pos[0] *= 64.0f;
                float y = pos[1];
                pos[1] = pos[2] * -64.0f;
                pos[2] = y * 64.0f;
            }
            for (vm::vec2f& uv : texcoords) {
                // This should be checked using the __TB_info_player_start model; Blender-defaults-output files are consistent with Neverball.
                uv[1] = 1.0f - uv[1];
            }
            return true;
        }

        std::optional<Assets::Texture> NvObjParser::loadMaterial(const std::string& text, Logger& logger) {
            // NOTE: A reasonable solution here would be to use the same material handling as the brushes unless otherwise required.
            // Then Neverball just gets an additional texture search directory.
            // But there's raw pointers all over the Texture system, so without further details on how memory is managed there, that's a bad idea.

            std::vector<Path> texturePaths = {
                Path("textures") + Path(text).addExtension("png"),
                Path("textures") + Path(text).addExtension("jpg"),
                Path(text).addExtension("png"),
                Path(text).addExtension("jpg"),
            };

            IO::FreeImageTextureReader imageReader(IO::TextureReader::StaticNameStrategy(""), m_fs, logger);
            for (const auto& texturePath : texturePaths) {
                try {
                    auto file = m_fs.openFile(texturePath);
                    return imageReader.readTexture(file);
                } catch (const Exception& /*ex1*/) {
                    // ignore and try the next texture path
                }
            }

            return std::nullopt;
        }

        std::optional<Assets::Texture> NvObjParser::loadFallbackMaterial(Logger& logger) {
            // Try to remove the '.obj' extension and grab that as a texture.
            // This isn't really how it works, but the Neverball-side truth involves MAP files acting as a replacement for something like JSON.
            // This is a less Neverball-specific set of logic which should be useful for any game.
            const auto basic_skin_name = m_path.lastComponent().deleteExtension().asString();
            if (auto material = loadMaterial(basic_skin_name, logger)) {
                return material;
            } else {
                return loadMaterial(Model::BrushFaceAttributes::NoTextureName, logger);
            }
        }
    }
}
