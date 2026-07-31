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

#pragma once

namespace tb::gl
{
class ResourceManager;
struct ProcessContext;
class MockGl;

void processResourcesSync(
  ResourceManager& resourceManager, const ProcessContext& processContext);

/**
 * Wires up the calls a Vbo needs in order to be constructed, bound, written to,
 * unbound and freed, using sequential non-zero buffer ids. Real OpenGL never hands
 * out id 0, and Vbo relies on that: bind, unbind and free all assert the id is
 * non-zero. Individual slots can be overridden afterwards to verify specific calls.
 */
void installVboSupport(MockGl& gl);

/**
 * Wires up the calls Texture::upload needs, using sequential non-zero texture ids.
 * Individual slots can be overridden afterwards to verify specific calls.
 */
void installTextureUploadSupport(MockGl& gl);

/**
 * Wires up the calls needed to successfully create a shader (id 2) and program
 * (id 1) and link them, so that gl::loadShader and ShaderManager::loadProgram
 * succeed. Individual slots can be overridden afterwards to verify specific calls.
 */
void installShaderCompileSupport(MockGl& gl);

} // namespace tb::gl
