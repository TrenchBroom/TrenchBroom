//Shaderfile for md3 models

models/mapobjects/cosmoflash/teleporter3{
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
//		blendfunc gl_dst_color gl_one_minus_dst_alpha
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}


models/mapobjects/cosmoflash/torch1{

    {
        map models/mapobjects/cosmoflash/torch1.tga
		rgbGen identity
	}
	
	{		map $lightmap 		blendfunc filter		rgbGen identity	}
}



models/mapobjects/weaponpod/weaponpod{    surfaceparm metalsteps

    {
        map textures/base_floor/clang_floor_s2.jpg
		rgbGen identity
	}
	
	{		map $lightmap 		blendfunc filter		rgbGen identity	}
}


models/mapobjects/cosmoflash/hand2{

    {
        map models/mapobjects/cosmoflash/hand2_n1.tga
		rgbGen identity
	}
	
	{		map $lightmap 		blendfunc filter		rgbGen identity	}
}

models/mapobjects/cosmoflash/cholder1a{

    {
        map models/mapobjects/cosmoflash/cholder1a.tga
		rgbGen identity
	}
	
	{		map $lightmap 		blendfunc filter		rgbGen identity	}
}

models/mapobjects/cosmoflash/menhir1
{
    q3map_nonplanar
    q3map_shadeangle 72
	{
		map textures/cosmo_liquids/plasma_red.jpg
//		blendFunc GL_ONE GL_ZERO
		rgbGen wave sin 0.5 0.5 0 .1
//        rgbGen identity
	}
    {
        map models/mapobjects/cosmoflash/menhir1.tga
		blendFunc GL_ONE GL_SRC_ALPHA
//        depthWrite
		rgbGen identity
	}
	{		map $lightmap 		blendfunc filter		rgbGen identity	}

}

models/mapobjects/cosmoflash/menhir2
{
    q3map_nonplanar
    q3map_shadeangle 66
	{
		map textures/cosmo_liquids/plasma_red.jpg
//		blendFunc GL_ONE GL_ZERO
		rgbGen wave sin 0.5 0.5 0.5 .2
//        rgbGen identity
	}
    {
        map models/mapobjects/cosmoflash/menhir2.tga
		blendFunc GL_ONE GL_SRC_ALPHA
		rgbGen identity
	}
	{		map $lightmap 		blendfunc filter		rgbGen identity	}
}

models/mapobjects/cosmoflash/fountain1{
    q3map_nonplanar
    q3map_shadeangle 89
    {
        map models/mapobjects/cosmoflash/fountain1.jpg
        blendFunc GL_ONE GL_ZERO
		rgbGen identity
	}
	
	{		map $lightmap 		blendfunc filter		rgbGen identity	}
}

models/mapobjects/gargoyle/stoned{
    q3map_nonplanar
    q3map_shadeangle 179
	{		map $lightmap 		rgbGen identity	}
    {
        map models/mapobjects/gargoyle/stoned.jpg
		blendfunc filter
		rgbGen identity
	}
}

models/mapobjects/cosmoflash/tele4_portal{
    {
        map models/mapobjects/cosmoflash/tele4_portal.tga
        blendFunc GL_ONE GL_ZERO
//		blendfunc blend
//		blendFunc GL_ONE GL_SRC_ALPHA
		rgbGen identity
	}
	{		map $lightmap
		blendfunc filter		rgbGen identity	}
}


models/mapobjects/cosmoflash/tele4_portal2{
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

models/mapobjects/cosmoflash/tele4_portal3{
    cull none
    {
        map models/mapobjects/cosmoflash/tele4_portal.tga
		blendfunc blend
//		blendFunc GL_ONE GL_SRC_ALPHA
		rgbGen identity
	}
}

models/mapobjects/cosmoflash/tele4_frame{
	surfaceparm nomarks
    q3map_lightimage models/mapobjects/cosmoflash/tele4_frame_glow.jpg
	q3map_surfacelight 750
	{		map $lightmap		rgbGen identity	}
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
}

