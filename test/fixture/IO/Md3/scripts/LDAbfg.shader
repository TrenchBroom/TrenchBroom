models/weapons2/bfg/LDAbfg
{
	{
		map models/weapons2/bfg/fx.jpg
		rgbGen lightingDiffuse
		tcMod scroll 0.006 0.009
		tcMod scale 3 3
		tcGen environment 
	}
	{
		map models/weapons2/bfg/LDAbfg.tga
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map $lightmap 
		blendfunc gl_dst_color gl_one_minus_dst_alpha
		rgbGen lightingDiffuse
		tcGen lightmap 
	}
}

models/weapons2/bfg/ldabfg_wfx
{
	{
		map models/weapons2/bfg/ldabfg_z.jpg
		rgbGen identity
		tcMod scroll 1 2
	}
	{
		map models/weapons2/bfg/ldabfg_wfx.tga
		blendfunc gl_one gl_src_alpha
		rgbGen lightingDiffuse
	}
}

models/weapons2/bfg/LDAbfg_z
{
	{
		map models/weapons2/bfg/ldabfg_z.jpg
		rgbGen Vertex
		tcMod scroll 1 2
		tcMod turb 0 0.5 0 0.2
	}
	{
		map models/weapons2/bfg/ldabfg_z.jpg
		blendfunc add
		rgbGen wave sin 0.1 0.4 7 4 
		tcMod scale 1 0.1
		tcMod scroll 2 2
		tcMod rotate 33
	}
	{
		map models/weapons2/bfg/fx.jpg
		blendfunc add
		rgbGen Vertex
		tcMod scale 1 1
		tcMod scroll 2 2
		tcGen environment 
	}
}

