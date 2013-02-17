// Mac modifier key symbols:
// Command Key:  &#8984;
// Control Key:  &#x2303;
// Option Key:   &#x2325;
// Shift Key:    &#8679;
// Cursor Up:    &#x2191;
// Cursor Down:  &#x2193;
// Cursor Left:  &#x2190;
// Cursor Right: &#x2192;
// Page Up:      &#x21DE;
// Page Down:    &#x21DF;

var menu_commands = new Object();
menu_commands["Mac"] = new Object();
menu_commands["Windows"] = new Object();
menu_commands["Linux"] = new Object();

menu_commands["Mac"]["preferences"] 					= "TrenchBroom &raquo; Preferences - &#8984;,";
menu_commands["Mac"]["file_new"]						= "File &raquo; New - &#8984;N"
menu_commands["Mac"]["file_open"]						= "File &raquo; Open... - &#8984;O"
menu_commands["Mac"]["file_save"]						= "File &raquo; Save - &#8984;S"
menu_commands["Mac"]["file_save_as"]					= "File &raquo; Save as... - &#8679;&#8984;S"
menu_commands["Mac"]["edit_map_properties"]				= "Edit &raquo; Map Properties..."
menu_commands["Mac"]["view_center_on_selection"]		= "View &raquo; Camera &raquo; Center on Selection - &#x2325;C"
menu_commands["Mac"]["edit_duplicate_selection"]		= "<b>Edit &raquo; Actions &raquo; Duplicate - &#8679;D</b>"
menu_commands["Mac"]["edit_copy"]						= "<b>Edit &raquo; Copy - &#8679;C</b>"
menu_commands["Mac"]["edit_cut"]						= "<b>Edit &raquo; Cut - &#8679;X</b>"
menu_commands["Mac"]["edit_paste"]						= "<b>Edit &raquo; Paste - &#8679;V</b>"

menu_commands["Windows"]["preferences"] 				= "View &raquo; Preferences";
menu_commands["Windows"]["file_new"]					= "File &raquo; New - Ctrl+N"
menu_commands["Windows"]["file_open"]					= "File &raquo; Open... - Ctrl+O"
menu_commands["Windows"]["file_save"]					= "File &raquo; Save - Ctrl+S"
menu_commands["Windows"]["file_save_as"]				= "File &raquo; Save as... - Ctrl+Shift+S"
menu_commands["Windows"]["edit_map_properties"]			= "Edit &raquo; Map Properties..."
menu_commands["Windows"]["view_center_on_selection"]	= "View &raquo; Camera &raquo; Center on Selection - Alt+C"
menu_commands["Windows"]["edit_duplicate_selection"]	= "<b>Edit &raquo; Actions &raquo; Duplicate - Ctrl+D</b>"
menu_commands["Windows"]["edit_copy"]					= "<b>Edit &raquo; Copy - Ctrl+C</b>"
menu_commands["Windows"]["edit_cut"]					= "<b>Edit &raquo; Cut - Ctrl+X</b>"
menu_commands["Windows"]["edit_paste"]					= "<b>Edit &raquo; Paste - Ctrl+V</b>"


//option key (&#x2325;)

var keys = new Object();

keys["Mac"] = new Object();
keys["Mac"]["CmdCtrl"] 	= "&#8984;";
keys["Mac"]["Alt"] 		= "&#x2325;";
keys["Mac"]["Shift"]	= "&#8679;";
keys["Mac"]["Up"]		= "&#x2191;";
keys["Mac"]["Down"]		= "&#x2193;";
keys["Mac"]["Left"]		= "&#x2190;";
keys["Mac"]["Right"]	= "&#x2192;";
keys["Mac"]["PgUp"]		= "&#x21DE;";
keys["Mac"]["PgDown"]	= "&#x21DF;";
keys["Mac"]["Combiner"] = "";

keys["Windows"] = new Object();
keys["Windows"]["CmdCtrl"] 	= "Ctrl";
keys["Windows"]["Alt"] 		= "Alt";
keys["Windows"]["Shift"]	= "Shift";
keys["Windows"]["Up"]		= "&#x2191;";
keys["Windows"]["Down"]		= "&#x2193;";
keys["Windows"]["Left"]		= "&#x2190;";
keys["Windows"]["Right"]	= "&#x2192;";
keys["Windows"]["PgUp"]		= "PageUp";
keys["Windows"]["PgDown"]	= "PageDown";
keys["Windows"]["Combiner"] = "+";

keys["Linux"] = new Object();

var platform = "Windows";
if (navigator.platform.indexOf("Win")!=-1) platform="Windows";
if (navigator.platform.indexOf("Mac")!=-1) platform="Mac";
if (navigator.platform.indexOf("X11")!=-1) platform="Linux";
if (navigator.platform.indexOf("Linux")!=-1) OSName="Linux";
//var platform = "Mac"; //debug

function print_menu_command(name) {
	document.write(menu_commands[platform][name]);
}

function resolve_key(name) {
	if (name in keys[platform])
		return keys[platform][name];
	return name;
}

function print_key(name, mod1, mod2, mod3) {
	var keyStr = "";
	if (mod1 != undefined)
		keyStr += resolve_key(mod1) + resolve_key("Combiner");
	if (mod2 != undefined)
		keyStr += resolve_key(mod2) + resolve_key("Combiner");
	if (mod3 != undefined)
		keyStr += resolve_key(mod3) + resolve_key("Combiner");
	keyStr += resolve_key(name);

	document.write("<b>" + keyStr + "</b>");
}

function print_macCtrlClick() {
	if (platform == "Mac")
		document.write(" (or <b>ctrl-click</b>)");
}
