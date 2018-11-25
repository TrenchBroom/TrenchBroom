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

#include <gtest/gtest.h>

#include "StringUtils.h"
#include "IO/Quake3ShaderParser.h"

namespace TrenchBroom {
    namespace IO {
        TEST(Quake3ShaderParserTest, parseEmptyShader) {
            Quake3ShaderParser parser("");
            ASSERT_EQ(String(), parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseShaderWithEmptyBlock) {
            Quake3ShaderParser parser(R"(
textures/liquids/lavahell2 //path and name of new texture
{}
)");
            ASSERT_EQ(String(), parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseSimpleShaderWithoutEditorImage) {
            Quake3ShaderParser parser(R"(
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
            ASSERT_EQ(String(""), parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseSimpleShaderWithEditorImage) {
            Quake3ShaderParser parser(R"(
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
            ASSERT_EQ(String("textures/eerie/lavahell.tga"), parser.parse());
        }


        TEST(Quake3ShaderParserTest, parseComplexShaderWithEditorImage) {
            Quake3ShaderParser parser(R"(
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

}})");
            ASSERT_EQ(String("textures/gothic_light/ironcrosslt2.tga"), parser.parse());
        }
    }
}
