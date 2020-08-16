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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Assets/Quake3Shader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderParser.h"
#include "IO/Reader.h"
#include "IO/TestParserStatus.h"

#include <string>

namespace TrenchBroom {
    namespace IO {
        void assertShaders(const std::vector<Assets::Quake3Shader>& expected, const std::vector<Assets::Quake3Shader>& actual);

        TEST_CASE("Quake3ShaderParserTest.parseEmptyShader", "[Quake3ShaderParserTest]") {
            const std::string data("");
            Quake3ShaderParser parser(data);
            const auto expected = std::vector<Assets::Quake3Shader> {};

            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseSingleShaderWithEmptyBlock", "[Quake3ShaderParserTest]") {
            const std::string data(R"(
textures/liquids/lavahell2 //path and name of new texture
{}
)");
            const auto expected = std::vector<Assets::Quake3Shader> {
                {
                    IO::Path("textures/liquids/lavahell2"), // shaderPath
                    IO::Path(), // editorImage
                    IO::Path(), // lightImage
                    Assets::Quake3Shader::Culling::Front, // culling
                    {}, // surfaceParms
                    {} // stages
                }
            };
            Quake3ShaderParser parser(data);
            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseSingleSimpleShaderWithoutEditorImage", "[Quake3ShaderParserTest]") {
            const std::string data(R"(
textures/liquids/lavahell2 //path and name of new texture
{

    //based on this
    qer_nocarve
    //cannot be cut by CSG subtract
    surfaceparm noimpact
    //projectiles do not hit it
    surfaceparm lava
    //has the game properties of lava
    surfaceparm nolightmap
    //environment lighting does not affect
    q3map_surfacelight 3000
    //light is emitted
    tessSize 256
    //relatively large triangles
    cull disable
    //no sides are removed
    deformVertexes wave 100 sin 5 5 .5 0.02
    fogparms 0.8519142 0.309723 0.0 128 128
    {
    map textures/eerie/lavahell.tga
    //base texture artwork
    tcMod turb .25 0.2 1 0.02
    //texture is subjected to turbulence
    tcMod scroll 0.1 0.1
    //the turbulence is scrolled
    }

})");
            const auto expected = std::vector<Assets::Quake3Shader> {
                {
                    IO::Path("textures/liquids/lavahell2"), // shaderPath
                    IO::Path(), // editorImage
                    IO::Path(), // lightImage
                    Assets::Quake3Shader::Culling::None, // culling
                    {"noimpact", "lava", "nolightmap" }, // surfaceParms
                    {
                        {
                            Path("textures/eerie/lavahell.tga"), // map
                            { "", "" } // blendFunc
                        }
                    } // stages
                }
            };
            Quake3ShaderParser parser(data);
            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseSingleSimpleShaderWithEditorImage", "[Quake3ShaderParserTest]") {
            const std::string data(R"(
textures/liquids/lavahell2 //path and name of new texture
{

    qer_editorimage textures/eerie/lavahell.tga
    //based on this
    qer_nocarve
    //cannot be cut by CSG subtract
    surfaceparm noimpact
    //projectiles do not hit it
    surfaceparm lava
    //has the game properties of lava
    surfaceparm nolightmap
    //environment lighting does not affect
    q3map_surfacelight 3000
    //light is emitted
    tessSize 256
    //relatively large triangles
    cull disable
    //no sides are removed
    deformVertexes wave 100 sin 5 5 .5 0.02
    fogparms 0.8519142 0.309723 0.0 128 128
    {
    map textures/eerie/lavahell.tga
    //base texture artwork
    tcMod turb .25 0.2 1 0.02
    //texture is subjected to turbulence
    tcMod scroll 0.1 0.1
    //the turbulence is scrolled
    }

})");
            const auto expected = std::vector<Assets::Quake3Shader>{
                {
                    IO::Path("textures/liquids/lavahell2"), // shaderPath
                    IO::Path("textures/eerie/lavahell.tga"), // editorImage
                    IO::Path(), // lightImage
                    Assets::Quake3Shader::Culling::None, // culling
                    {"noimpact", "lava", "nolightmap" }, // surfaceParms
                    {
                        {
                            Path("textures/eerie/lavahell.tga"), // map
                            { "", "" } // blendFunc
                        }
                    } // stages
                }
            };
            Quake3ShaderParser parser(data);
            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseSingleComplexShaderWithEditorImage", "[Quake3ShaderParserTest]") {
            const std::string data(R"(
textures/eerie/ironcrosslt2_10000
{

    q3map_lightimage textures/gothic_light/ironcrosslt2.blend.tga
    // this TGA is the source for the color of the blended light

    qer_editorimage textures/gothic_light/ironcrosslt2.tga
    //base TGA (used because the shader is used with several
    // different light values

    q3map_surfacelight 10000
    //emitted light value of 10,000

    {
    map $lightmap
    //source texture is affected by the lightmap
    rgbGen identity
    // this command handles the overbright bits created by "sunlight"
    // in the game
    }
    {
    map textures/gothic_light/ironcrosslt2.tga
    blendFunc filter
    rgbGen identity
    }
    {
    map textures/gothic_light/ironcrosslt2.blend.tga
    blendFunc add
    }

})");
            const auto expected = std::vector<Assets::Quake3Shader>{
                {
                    IO::Path("textures/eerie/ironcrosslt2_10000"), // shaderPath
                    IO::Path("textures/gothic_light/ironcrosslt2.tga"), // editorImage
                    IO::Path("textures/gothic_light/ironcrosslt2.blend.tga"), // lightImage
                    Assets::Quake3Shader::Culling::Front, // culling
                    {}, // surfaceParms
                    {
                        {
                            Path("$lightmap"), // map
                            { "","" } // blendFunc
                        },
                        {
                            Path("textures/gothic_light/ironcrosslt2.tga"), // map
                            { "GL_DST_COLOR", "GL_ZERO" } // blendFunc
                        },
                        {
                            Path("textures/gothic_light/ironcrosslt2.blend.tga"), // map
                            { "GL_ONE", "GL_ONE" } // blendFunc
                        }
                    } // stages
                }
            };
            Quake3ShaderParser parser(data);
            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseTwoShaders", "[Quake3ShaderParserTest]") {
            const std::string data(R"(
textures/eerie/ironcrosslt2_10000
{

    q3map_lightimage textures/gothic_light/ironcrosslt2.blend.tga
    // this TGA is the source for the color of the blended light

    qer_editorimage textures/gothic_light/ironcrosslt2.tga
    //base TGA (used because the shader is used with several
    // different light values

    q3map_surfacelight 10000
    //emitted light value of 10,000

    {
    map $lightmap
    //source texture is affected by the lightmap
    rgbGen identity
    // this command handles the overbright bits created by "sunlight"
    // in the game
    }
    {
    map textures/gothic_light/ironcrosslt2.tga
    blendFunc filter
    rgbGen identity
    }
    {
    map textures/gothic_light/ironcrosslt2.blend.tga
    blendFunc add
    }

}

textures/liquids/lavahell2 //path and name of new texture
{

    qer_editorimage textures/eerie/lavahell.tga
    //based on this
    qer_nocarve
    qer_trans 0.4
    //cannot be cut by CSG subtract
    surfaceparm noimpact
    //projectiles do not hit it
    surfaceparm lava
    //has the game properties of lava
    surfaceparm nolightmap
    //environment lighting does not affect
    q3map_surfacelight 3000
    //light is emitted
    tessSize 256
    //relatively large triangles
    cull disable
    //no sides are removed
    deformVertexes wave 100 sin 5 5 .5 0.02
    fogparms 0.8519142 0.309723 0.0 128 128
    {
    map textures/eerie/lavahell.tga
    //base texture artwork
    tcMod turb .25 0.2 1 0.02
    //texture is subjected to turbulence
    tcMod scroll 0.1 0.1
    //the turbulence is scrolled
    }

}

)");
            const auto expected = std::vector<Assets::Quake3Shader>{
                {
                    IO::Path("textures/eerie/ironcrosslt2_10000"), // shaderPath
                    IO::Path("textures/gothic_light/ironcrosslt2.tga"), // editorImage
                    IO::Path("textures/gothic_light/ironcrosslt2.blend.tga"), // lightImage
                    Assets::Quake3Shader::Culling::Front, // culling
                    {}, // surfaceParms
                    {
                        {
                            Path("$lightmap"), // map
                            { "","" } // blendFunc
                        },
                        {
                            Path("textures/gothic_light/ironcrosslt2.tga"), // map
                            { "GL_DST_COLOR", "GL_ZERO" } // blendFunc
                        },
                        {
                            Path("textures/gothic_light/ironcrosslt2.blend.tga"), // map
                            { "GL_ONE", "GL_ONE" } // blendFunc
                        }
                    } // stages
                },
                {
                    IO::Path("textures/liquids/lavahell2"), // shaderPath
                    IO::Path("textures/eerie/lavahell.tga"), // editorImage
                    IO::Path(), // lightImage
                    Assets::Quake3Shader::Culling::None, // culling
                    {"noimpact", "lava", "nolightmap" }, // surfaceParms
                    {
                        {
                            Path("textures/eerie/lavahell.tga"), // map
                            { "", "" } // blendFunc
                        }
                    } // stages
                }
            };
            Quake3ShaderParser parser(data);
            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseShadersWithCommentTerminatingBlockEntry", "[Quake3ShaderParserTest]") {
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
            ASSERT_NO_THROW(parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseShadersWithMultilineComment", "[Quake3ShaderParserTest]") {
            const std::string data(R"(
/*
This is a
multiline comment.
*/

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
            ASSERT_NO_THROW(parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseBlendFuncParameters", "[Quake3ShaderParserTest]") {
            // see https://github.com/id-Software/Quake-III-Arena/blob/master/code/renderer/tr_shader.c#L176
            const std::string data(R"(
            waterBubble
            {
                {
                    map sprites/bubble.tga
                    blendFunc add
                }
                {
                    map sprites/bubble.tga
                    blendFunc filter
                }
                {
                    map sprites/bubble.tga
                    blendFunc blend
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_ONE GL_ONE
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_ZERO GL_ZERO
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_DST_COLOR GL_SRC_ALPHA
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_SRC_ALPHA GL_DST_ALPHA
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE_MINUS_DST_ALPHA
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_DST_ALPHA GL_SRC_COLOR
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_ONE_MINUS_DST_ALPHA GL_ONE_MINUS_SRC_COLOR
                }
                {
                    map sprites/bubble.tga
                    blendFunc GL_SRC_ALPHA_SATURATE GL_ONE_MINUS_SRC_COLOR
                }
            }

            )");

            Quake3ShaderParser parser(data);
            TestParserStatus status;
            const auto shaders = parser.parse(status);
            ASSERT_EQ(1u, shaders.size());

            const auto& shader = shaders.front();
            const auto& stages = shader.stages;
            ASSERT_EQ(12u, stages.size());

            // add
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::One, stages[0].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::One, stages[0].blendFunc.destFactor);

            // filter
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::DestColor, stages[1].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::Zero, stages[1].blendFunc.destFactor);

            // blend
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::SrcAlpha, stages[2].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha, stages[2].blendFunc.destFactor);

            // GL_ONE GL_ONE
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::One, stages[3].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::One, stages[3].blendFunc.destFactor);

            // GL_ZERO GL_ZERO
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::Zero, stages[4].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::Zero, stages[4].blendFunc.destFactor);

            // GL_DST_COLOR GL_SRC_ALPHA
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::DestColor, stages[5].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::SrcAlpha, stages[5].blendFunc.destFactor);

            // GL_ONE_MINUS_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusDestColor, stages[6].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha, stages[6].blendFunc.destFactor);

            // GL_SRC_ALPHA GL_DST_ALPHA
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::SrcAlpha, stages[7].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::DestAlpha, stages[7].blendFunc.destFactor);

            // GL_ONE_MINUS_SRC_ALPHA GL_ONE_MINUS_DST_ALPHA
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha, stages[8].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusDestAlpha, stages[8].blendFunc.destFactor);

            // GL_DST_ALPHA GL_SRC_COLOR
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::DestAlpha, stages[9].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::SrcColor, stages[9].blendFunc.destFactor);

            // GL_ONE_MINUS_DST_ALPHA GL_ONE_MINUS_SRC_COLOR
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusDestAlpha, stages[10].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusSrcColor, stages[10].blendFunc.destFactor);

            // GL_SRC_ALPHA_SATURATE GL_ONE_MINUS_SRC_COLOR
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::SrcAlphaSaturate, stages[11].blendFunc.srcFactor);
            ASSERT_EQ(Assets::Quake3ShaderStage::BlendFunc::OneMinusSrcColor, stages[11].blendFunc.destFactor);
        }

        TEST_CASE("Quake3ShaderParserTest.parseShadersWithInvalidWhitespace", "[Quake3ShaderParserTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2537
            // The file contains a carriage return without a consecutive line feed, which tripped the parser.

            const auto workDir = Disk::getCurrentWorkingDir();
            auto fs = DiskFileSystem(workDir + Path("fixture/test/IO/Shader/parser"));
            auto testFile = fs.openFile(Path("am_cf_models.shader"));
            auto reader = testFile->reader().buffer();

            Quake3ShaderParser parser(std::begin(reader), std::end(reader));
            TestParserStatus status;
            ASSERT_NO_THROW(parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseShaderAbsolutePath", "[Quake3ShaderParserTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2633
            // apparently, the Q3 engine can handle this

            const std::string data(R"(
/textures/eerie/ironcrosslt2_10000
{
    qer_editorimage textures/gothic_light/ironcrosslt2.tga
    //base TGA (used because the shader is used with several
    // different light values

})");
            const auto expected = std::vector<Assets::Quake3Shader>{
                {
                    IO::Path("textures/eerie/ironcrosslt2_10000"), // shaderPath
                    IO::Path("textures/gothic_light/ironcrosslt2.tga"), // editorImage
                    IO::Path(), // lightImage
                    Assets::Quake3Shader::Culling::Front, // culling
                    {}, // surfaceParms
                    {} // stages
                }
            };
            Quake3ShaderParser parser(data);
            TestParserStatus status;
            assertShaders(expected, parser.parse(status));
        }

        TEST_CASE("Quake3ShaderParserTest.parseShaderWithMissingCBrace", "[Quake3ShaderParserTest]") {
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
            ASSERT_NO_THROW(parser.parse(status));
        }

        void assertShaders(const std::vector<Assets::Quake3Shader>& expected, const std::vector<Assets::Quake3Shader>& actual) {
            ASSERT_EQ(expected.size(), actual.size());
            for (const auto& expectedShader : expected) {
                auto it = std::find(std::begin(actual), std::end(actual), expectedShader);
                ASSERT_TRUE(it != std::end(actual));

                const auto& actualShader = *it;
                ASSERT_TRUE(isEqual(expectedShader, actualShader));
            }
        }
    }
}
