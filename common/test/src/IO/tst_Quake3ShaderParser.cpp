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

#include <filesystem>
#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{

TEST_CASE("Quake3ShaderParserTest.parseEmptyShader")
{
  const auto data = "";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK(parser.parse(status).empty());
}

TEST_CASE("Quake3ShaderParserTest.parseSingleShaderWithEmptyBlock")
{
  const auto data = R"(
textures/liquids/lavahell2 //path and name of new texture
{}
)";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      "textures/liquids/lavahell2",         // shaderPath
      "",                                   // editorImage
      "",                                   // lightImage
      Assets::Quake3Shader::Culling::Front, // culling
      {},                                   // surfaceParms
      {}                                    // stages
    }}));
}

TEST_CASE("Quake3ShaderParserTest.parseSingleSimpleShaderWithoutEditorImage")
{
  const auto data = R"(
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

})";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      "textures/liquids/lavahell2",        // shaderPath
      "",                                  // editorImage
      "",                                  // lightImage
      Assets::Quake3Shader::Culling::None, // culling
      {"noimpact", "lava", "nolightmap"},  // surfaceParms
      {{
        "textures/eerie/lavahell.tga", // map
        {"", ""}                       // blendFunc
      }}                               // stages
    }}));
}

TEST_CASE("Quake3ShaderParserTest.parseSingleSimpleShaderWithEditorImage")
{
  const auto data = R"(
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

})";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      "textures/liquids/lavahell2",        // shaderPath
      "textures/eerie/lavahell.tga",       // editorImage
      "",                                  // lightImage
      Assets::Quake3Shader::Culling::None, // culling
      {"noimpact", "lava", "nolightmap"},  // surfaceParms
      {{
        "textures/eerie/lavahell.tga", // map
        {"", ""}                       // blendFunc
      }}                               // stages
    }}));
}

TEST_CASE("Quake3ShaderParserTest.parseSingleComplexShaderWithEditorImage")
{
  const auto data = R"(
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

})";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      "textures/eerie/ironcrosslt2_10000",            // shaderPath
      "textures/gothic_light/ironcrosslt2.tga",       // editorImage
      "textures/gothic_light/ironcrosslt2.blend.tga", // lightImage
      Assets::Quake3Shader::Culling::Front,           // culling
      {},                                             // surfaceParms
      {{
         "$lightmap", // map
         {"", ""}     // blendFunc
       },
       {
         "textures/gothic_light/ironcrosslt2.tga", // map
         {"GL_DST_COLOR", "GL_ZERO"}               // blendFunc
       },
       {
         "textures/gothic_light/ironcrosslt2.blend.tga", // map
         {"GL_ONE", "GL_ONE"}                            // blendFunc
       }}                                                // stages
    }}));
}

TEST_CASE("Quake3ShaderParserTest.caseSensitivity")
{
  const auto data = R"(
textures/eerie/ironcrosslt2_10000
{

    Q3MAP_LIGHTIMAGE textures/gothic_light/ironcrosslt2.blend.tga
    // this TGA is the source for the color of the blended light

    QER_EDITORIMAGE textures/gothic_light/ironcrosslt2.tga
    //base TGA (used because the shader is used with several
    // different light values

    Q3MAP_SURFACELIGHT 10000
    //emitted light value of 10,000

    {
    MAP $lightmap
    //source texture is affected by the lightmap
    RGBGEN IDENTITY
    // this command HANDLES the overbright bits created by "sunlight"
    // in the game
    }
    {
    MAP textures/gothic_light/ironcrosslt2.tga
    BLENDFUNC FILTER
    RGBGEN IDENTITY
    }
    {
    MAP textures/gothic_light/ironcrosslt2.blend.tga
    BLENDFUNC ADD
    }

})";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      "textures/eerie/ironcrosslt2_10000",            // shaderPath
      "textures/gothic_light/ironcrosslt2.tga",       // editorImage
      "textures/gothic_light/ironcrosslt2.blend.tga", // lightImage
      Assets::Quake3Shader::Culling::Front,           // culling
      {},                                             // surfaceParms
      {{
         "$lightmap", // map
         {"", ""}     // blendFunc
       },
       {
         "textures/gothic_light/ironcrosslt2.tga", // map
         {"GL_DST_COLOR", "GL_ZERO"}               // blendFunc
       },
       {
         "textures/gothic_light/ironcrosslt2.blend.tga", // map
         {"GL_ONE", "GL_ONE"}                            // blendFunc
       }}                                                // stages
    }}));
}

TEST_CASE("Quake3ShaderParserTest.parseTwoShaders")
{
  const auto data = R"(
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

)";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{
      {
        "textures/eerie/ironcrosslt2_10000",            // shaderPath
        "textures/gothic_light/ironcrosslt2.tga",       // editorImage
        "textures/gothic_light/ironcrosslt2.blend.tga", // lightImage
        Assets::Quake3Shader::Culling::Front,           // culling
        {},                                             // surfaceParms
        {{
           "$lightmap", // map
           {"", ""}     // blendFunc
         },
         {
           "textures/gothic_light/ironcrosslt2.tga", // map
           {"GL_DST_COLOR", "GL_ZERO"}               // blendFunc
         },
         {
           "textures/gothic_light/ironcrosslt2.blend.tga", // map
           {"GL_ONE", "GL_ONE"}                            // blendFunc
         }}                                                // stages
      },
      {
        "textures/liquids/lavahell2",        // shaderPath
        "textures/eerie/lavahell.tga",       // editorImage
        "",                                  // lightImage
        Assets::Quake3Shader::Culling::None, // culling
        {"noimpact", "lava", "nolightmap"},  // surfaceParms
        {{
          "textures/eerie/lavahell.tga", // map
          {"", ""}                       // blendFunc
        }}                               // stages
      }}));
}

TEST_CASE("Quake3ShaderParserTest.parseShadersWithMultilineComment")
{
  const auto data = R"(
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

)";
  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};
  CHECK_NOTHROW(parser.parse(status));
}

TEST_CASE("Quake3ShaderParserTest.parseBlendFuncParameters")
{
  // see
  // https://github.com/id-Software/Quake-III-Arena/blob/master/code/renderer/tr_shader.c#L176
  const auto data = R"(
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

            )";

  using BF = Assets::Quake3ShaderStage::BlendFunc;

  auto parser = Quake3ShaderParser{data};
  auto status = TestParserStatus{};

  CHECK_THAT(
    parser.parse(status),
    Catch::UnorderedEquals(std::vector<Assets::Quake3Shader>{{
      "waterBubble",                        // shaderPath
      "",                                   // editorImage
      "",                                   // lightImage
      Assets::Quake3Shader::Culling::Front, // culling
      {},                                   // surfaceParms
      {
        {
          "sprites/bubble.tga", // map
          {BF::One, BF::One}    // blendFunc
        },
        {
          "sprites/bubble.tga",     // map
          {BF::DestColor, BF::Zero} // blendFunc
        },
        {
          "sprites/bubble.tga",                // map
          {BF::SrcAlpha, BF::OneMinusSrcAlpha} // blendFunc
        },
        {
          "sprites/bubble.tga", // map
          {BF::One, BF::One}    // blendFunc
        },
        {
          "sprites/bubble.tga", // map
          {BF::Zero, BF::Zero}  // blendFunc
        },
        {
          "sprites/bubble.tga",         // map
          {BF::DestColor, BF::SrcAlpha} // blendFunc
        },
        {
          "sprites/bubble.tga",                         // map
          {BF::OneMinusDestColor, BF::OneMinusSrcAlpha} // blendFunc
        },
        {
          "sprites/bubble.tga",         // map
          {BF::SrcAlpha, BF::DestAlpha} // blendFunc
        },
        {
          "sprites/bubble.tga",                         // map
          {BF::OneMinusSrcAlpha, BF::OneMinusDestAlpha} // blendFunc
        },
        {
          "sprites/bubble.tga",         // map
          {BF::DestAlpha, BF::SrcColor} // blendFunc
        },
        {
          "sprites/bubble.tga",                         // map
          {BF::OneMinusDestAlpha, BF::OneMinusSrcColor} // blendFunc
        },
        {
          "sprites/bubble.tga",                        // map
          {BF::SrcAlphaSaturate, BF::OneMinusSrcColor} // blendFunc
        },
      } // stages
    }}));
}

} // namespace TrenchBroom::IO
