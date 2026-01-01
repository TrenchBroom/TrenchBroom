# Build dependencies:
#   - https://github.com/ericwa/ericw-tools
#   - https://github.com/L-P/goldutil

# A single cube, three faces with an embedded texture, three faces with a
# texture found in an external WAD.
# Due to the way paths work differently in TrenchBroom and ericw, WADs would
# need to be moved to the .map directory to be displayed properly in TrenchBroom.
# Encased in a SKIP box for VIS purposes.
all: hl.bsp invalid_version.bsp

embedded/embedded.wad: skip.png embedded.png
	mkdir -p "$(dir $@)"
	goldutil wad create -out "$@" $^

external/external.wad: external.png
	mkdir -p "$(dir $@)"
	goldutil wad create -out "$@" $^

hl.bsp: hl.map embedded/embedded.wad external/external.wad
	qbsp -nolog -hlbsp -noextendedsurfflags \
		-wadpath embedded -xwadpath external "$<" "$@"
	vis -nolog "$@"
	light -nolog "$@"
	rm "$(subst .bsp,.prt,$@)"

invalid_version.bsp: hl.bsp
	cp "$<" "$@"
	# First byte is the BSP version, use 0x3F ('?') as an arbitrary canary value.
	printf '?' | dd of="$@" bs=1 count=1 seek=0 conv=notrunc
