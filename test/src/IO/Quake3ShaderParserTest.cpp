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
#include "Assets/Quake3Shader.h"
#include "IO/Quake3ShaderParser.h"

namespace TrenchBroom {
    namespace IO {
        Assets::Quake3Shader makeShader(const IO::Path& shhaderPath, const IO::Path& qerImagePath = IO::Path(), const StringSet& surfaceParms = StringSet());
        void assertShaders(const std::vector<Assets::Quake3Shader>& expected, const std::vector<Assets::Quake3Shader>& actual);

        TEST(Quake3ShaderParserTest, parseEmptyShader) {
            const String data("");
            Quake3ShaderParser parser(data);
            const auto expected = std::vector<Assets::Quake3Shader> {};
            assertShaders(expected, parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseSingleShaderWithEmptyBlock) {
            const String data(R"(
textures/liquids/lavahell2 //path and name of new texture
{}
)");
            const auto expected = std::vector<Assets::Quake3Shader>{
                makeShader(IO::Path("textures/liquids/lavahell2"))
            };
            Quake3ShaderParser parser(data);
            assertShaders(expected, parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseSingleSimpleShaderWithoutEditorImage) {
            const String data(R"(
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
            const auto expected = std::vector<Assets::Quake3Shader>{
                makeShader(IO::Path("textures/liquids/lavahell2"), IO::Path(), StringSet { "noimpact", "lava", "nolightmap" })
            };
            Quake3ShaderParser parser(data);
            assertShaders(expected, parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseSingleSimpleShaderWithEditorImage) {
            const String data(R"(
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
                makeShader(IO::Path("textures/liquids/lavahell2"), IO::Path("textures/eerie/lavahell.tga"), StringSet { "noimpact", "lava", "nolightmap" })
            };
            Quake3ShaderParser parser(data);
            assertShaders(expected, parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseSingleComplexShaderWithEditorImage) {
            const String data(R"(
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
                    makeShader(IO::Path("textures/eerie/ironcrosslt2_10000"), IO::Path("textures/gothic_light/ironcrosslt2.tga"))
            };
            Quake3ShaderParser parser(data);
            assertShaders(expected, parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseTwoShaders) {
            const String data(R"(
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
                    makeShader(IO::Path("textures/eerie/ironcrosslt2_10000"), IO::Path("textures/gothic_light/ironcrosslt2.tga")),
                    makeShader(IO::Path("textures/liquids/lavahell2"), IO::Path("textures/eerie/lavahell.tga"), StringSet { "noimpact", "lava", "nolightmap" })

            };
            Quake3ShaderParser parser(data);
            assertShaders(expected, parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseShadersWithCommentTerminatingBlockEntry) {
            const String data(R"(
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
            ASSERT_NO_THROW(parser.parse());
        }

        TEST(Quake3ShaderParserTest, parseShadersWithInvalidWhitespace) {
            // see https://github.com/kduske/TrenchBroom/issues/2537

            const String data(R"(
//Shaderfile for md3 models

models/mapobjects/cosmoflash/teleporter3
{
    {
        map textures/cosmo_sfx/pulse.jpg
        tcMod scroll 0 1
        tcMod stretch sin 1 0.8 1 0.4
    }
    {
        map textures/cosmo_sfx/stoerung.jpg
        blendFunc GL_ONE GL_ONE
        rgbgen wave sin .25 0.1 0 0.1
        tcMod scroll 0 10
    }   
    {
        map textures/cosmo_sfx/stoerung.jpg
        blendFunc GL_ONE GL_ONE
        rgbgen wave sin 0.25 0.1 0 0.1
        tcMod scale  -1 1
        tcMod scroll 0 -5
    }
    {
        map models/mapobjects/cosmoflash/teleporter3.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaFunc GT0
        depthWrite
        rgbGen identity
    }
    {
        map $lightmap 
//      blendfunc gl_dst_color gl_one_minus_dst_alpha
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}


models/mapobjects/cosmoflash/torch1
{

    {
        map models/mapobjects/cosmoflash/torch1.tga
        rgbGen identity
    }
    
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }
}



models/mapobjects/weaponpod/weaponpod
{
    surfaceparm metalsteps

    {
        map textures/base_floor/clang_floor_s2.jpg
        rgbGen identity
    }
    
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }
}


models/mapobjects/cosmoflash/hand2
{

    {
        map models/mapobjects/cosmoflash/hand2_n1.tga
        rgbGen identity
    }
    
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }
}

models/mapobjects/cosmoflash/cholder1a
{

    {
        map models/mapobjects/cosmoflash/cholder1a.tga
        rgbGen identity
    }
    
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }
}

models/mapobjects/cosmoflash/menhir1
{
    q3map_nonplanar
    q3map_shadeangle 72
    {
        map textures/cosmo_liquids/plasma_red.jpg
//      blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 0.5 0.5 0 .1
//        rgbGen identity
    }
    {
        map models/mapobjects/cosmoflash/menhir1.tga
        blendFunc GL_ONE GL_SRC_ALPHA
//        depthWrite
        rgbGen identity
    }
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }

}

models/mapobjects/cosmoflash/menhir2
{
    q3map_nonplanar
    q3map_shadeangle 66
    {
        map textures/cosmo_liquids/plasma_red.jpg
//      blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 0.5 0.5 0.5 .2
//        rgbGen identity
    }
    {
        map models/mapobjects/cosmoflash/menhir2.tga
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }
}

models/mapobjects/cosmoflash/fountain1
{
    q3map_nonplanar
    q3map_shadeangle 89
    {
        map models/mapobjects/cosmoflash/fountain1.jpg
        blendFunc GL_ONE GL_ZERO
        rgbGen identity
    }
    
    {
        map $lightmap 
        blendfunc filter
        rgbGen identity
    }
}

models/mapobjects/gargoyle/stoned
{
    q3map_nonplanar
    q3map_shadeangle 179
    {
        map $lightmap 
        rgbGen identity
    }
    {
        map models/mapobjects/gargoyle/stoned.jpg
        blendfunc filter
        rgbGen identity
    }
}

models/mapobjects/cosmoflash/tele4_portal
{
    {
        map models/mapobjects/cosmoflash/tele4_portal.tga
        blendFunc GL_ONE GL_ZERO
//      blendfunc blend
//      blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendfunc filter
        rgbGen identity
    }
}


models/mapobjects/cosmoflash/tele4_portal2
{
    cull none
    deformVertexes wave 100 sin 3 0 0 0 
    q3map_surfacelight 400
    q3map_flare flareShader-wide
    {
        map textures/pulchr/teleenv.tga
        blendfunc add
        tcMod rotate 30
        tcMod scroll 1 0.1
        tcGen environment 
    }
}

models/mapobjects/cosmoflash/tele4_portal3
{
    cull none
    {
        map models/mapobjects/cosmoflash/tele4_portal.tga
        blendfunc blend
//      blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen identity
    }
}

models/mapobjects/cosmoflash/tele4_frame
{
    surfaceparm nomarks
    q3map_lightimage models/mapobjects/cosmoflash/tele4_frame_glow.jpg
    q3map_surfacelight 750
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/mapobjects/cosmoflash/tele4_frame.tga
        blendfunc filter
        rgbGen identity
    }
    {
        map models/mapobjects/cosmoflash/tele4_frame_glow.jpg
        blendfunc add
        rgbGen wave sin 0.25 0.25 0 .25
    }
})");
            Quake3ShaderParser parser(data);
            ASSERT_NO_THROW(parser.parse());
        }

        Assets::Quake3Shader makeShader(const IO::Path& shaderPath, const IO::Path& qerImagePath, const StringSet& surfaceParms) {
            auto shader = Assets::Quake3Shader();
            shader.setTexturePath(shaderPath);

            if (!qerImagePath.isEmpty()) {
                shader.setQerImagePath(qerImagePath);
            }

            for (const auto& surfaceParm : surfaceParms) {
                shader.addSurfaceParm(surfaceParm);
            }

            return shader;
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
