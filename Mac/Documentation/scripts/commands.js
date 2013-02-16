// Mac modifier key symbols:
// Command Key: &#8984;
// Control Key: &#x2303;
// Option Key:
// Shift Key:   &#8679;

var menu_commands = new Object();
menu_commands["Mac"] = new Object();
menu_commands["Windows"] = new Object();
menu_commands["Linux"] = new Object();

menu_commands["Mac"]["preferences"] 			= "TrenchBroom &raquo; Preferences - &#8984;,";
menu_commands["Mac"]["file_new"]				= "File &raquo; New - &#8984;N"
menu_commands["Mac"]["file_open"]				= "File &raquo; Open... - &#8984;O"
menu_commands["Mac"]["file_save"]				= "File &raquo; Save - &#8984;S"
menu_commands["Mac"]["file_save_as"]			= "File &raquo; Save as... - &#8679;&#8984;S"
menu_commands["Mac"]["edit_map_properties"]		= "Edit &raquo; Map Properties..."

menu_commands["Windows"]["preferences"] 			= "View &raquo; Preferences";
menu_commands["Windows"]["file_new"]				= "File &raquo; New - Ctrl+N"
menu_commands["Windows"]["file_open"]				= "File &raquo; Open... - Ctrl+O"
menu_commands["Windows"]["file_save"]				= "File &raquo; Save - Ctrl+S"
menu_commands["Windows"]["file_save_as"]			= "File &raquo; Save as... - Ctrl+Shift+S"
menu_commands["Windows"]["edit_map_properties"]		= "Edit &raquo; Map Properties..."

var platform = "Mac";

function print_menu_command(name) {
	document.write(menu_commands[platform][name]);
}