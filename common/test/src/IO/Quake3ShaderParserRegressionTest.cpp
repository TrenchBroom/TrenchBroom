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

#include "Assets/Quake3Shader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderParser.h"
#include "IO/Reader.h"
#include "IO/TestParserStatus.h"

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE(
  "Quake3ShaderParserTest.parseShadersWithCommentTerminatingBlockEntry",
  "[Quake3ShaderParserTest]")
{
  const std::string data(R"(
waterBubble
{
    sort	underwater
    cull none
    entityMergable		// this comment terminates a block entry
    {
        map sprites/bubble.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen		vertex
        alphaGen	vertex
    }
}

)");
  Quake3ShaderParser parser(data);
  TestParserStatus status;
  CHECK_NOTHROW(parser.parse(status));
}

TEST_CASE(
  "Quake3ShaderParserTest.parseShadersWithInvalidWhitespace", "[Quake3ShaderParserTest]")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2537
  // The file contains a carriage return without a consecutive line feed, which tripped
  // the parser.

  const auto workDir = Disk::getCurrentWorkingDir();
  auto fs = DiskFileSystem(workDir + Path("fixture/test/IO/Shader/parser"));
  auto testFile = fs.openFile(Path("am_cf_models.shader"));
  auto reader = testFile->reader().buffer();

  Quake3ShaderParser parser(reader.stringView());
  TestParserStatus status;
  CHECK_NOTHROW(parser.parse(status));
}

TEST_CASE("Quake3ShaderParserTest.parseShaderAbsolutePath")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2633
  // apparently, the Q3 engine can handle this

  const std::string data(R"(
/textures/eerie/ironcrosslt2_10000
{
    qer_editorimage textures/gothic_light/ironcrosslt2.tga
    //base TGA (used because the shader is used with several
    // different light values

})");

  Quake3ShaderParser parser(data);
  TestParserStatus status;

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      IO::Path("textures/eerie/ironcrosslt2_10000"),      // shaderPath
      IO::Path("textures/gothic_light/ironcrosslt2.tga"), // editorImage
      IO::Path(),                                         // lightImage
      Assets::Quake3Shader::Culling::Front,               // culling
      {},                                                 // surfaceParms
      {}                                                  // stages
    }}));
}

TEST_CASE(
  "Quake3ShaderParserTest.parseShaderWithMissingCBrace", "[Quake3ShaderParserTest]")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2663
  // Quake 3 allows this, too.

  const std::string data(R"(
textures/evil3_floors/t-flr_oddtile_drty
{
        {
		map $lightmap
		              rgbGen identity 	}
}
textures/evil3_floors/cemtiledrk_mhbrk
{

        {
		map textures/evil3_floors/cemtiledrk_mhbrk_glow.tga
	}
}
)");

  Quake3ShaderParser parser(data);
  TestParserStatus status;
  CHECK_NOTHROW(parser.parse(status));
}
} // namespace IO
} // namespace TrenchBroom
