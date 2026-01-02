# Game
## variable attributes
- game path
## constant attributes
- palette (optional)
- texture format
Quake: wad
Quake2: wal
Hexen2: wad


# Example

{
	name = "Quake",
  icon = "games/quake/Icon.png",
  fileformats = {"Quake1", "Valve"},
	filesystem = {
		searchpath = "id1",
		packageformat = "pak"
	},
	textures = { 
		type = "wad",
    property = "wad",
		palette = "games/quake/palette.lmp",
		builtin = "textures"
	},
  entities = {
		definitions = "games/quake/Quake.fgd",
    defaultcolor = "1.0 1.0 1.0 1.0"
		modelformats = {"mdl", "bsp"}
  }
}
