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
// Backspace:    &#x232B;
// Return:       &#x21A9;
// Tab:          &#x21E5;

var menu_commands = new Object();
menu_commands["Mac"] = new Object();
menu_commands["Windows"] = new Object();
menu_commands["Linux"] = new Object();

menu_commands["Mac"]["preferences"] 					= "TrenchBroom &raquo; Preferences - &#8984;,";
menu_commands["Mac"]["file_new"]						= "File &raquo; New - &#8984;N";
menu_commands["Mac"]["file_open"]						= "File &raquo; Open... - &#8984;O";
menu_commands["Mac"]["file_save"]						= "File &raquo; Save - &#8984;S";
menu_commands["Mac"]["file_save_as"]					= "File &raquo; Save as... - &#8679;&#8984;S";

menu_commands["Mac"]["file_load_pointfile"]				= "File &raquo; Load Pointfile";
menu_commands["Mac"]["file_reload_pointfile"]			= "File &raquo; Reload Pointfile";
menu_commands["Mac"]["file_unload_pointfile"]			= "File &raquo; Unload Pointfile";

menu_commands["Mac"]["edit_copy"]						= "Edit &raquo; Copy - &#8984;C";
menu_commands["Mac"]["edit_cut"]						= "Edit &raquo; Cut - &#8984;X";
menu_commands["Mac"]["edit_paste"]						= "Edit &raquo; Paste - &#8984;V";
menu_commands["Mac"]["edit_paste_at_original_pos"]		= "Edit &raquo; Paste At Original Position - &#8984;&#8679;V";
menu_commands["Mac"]["edit_select_all"]					= "Edit &raquo; Select All - &#8984;A";
menu_commands["Mac"]["edit_select_siblings"]			= "Edit &raquo; Select Siblings - &#x2325;&#8984;A";
menu_commands["Mac"]["edit_select_touching"]			= "Edit &raquo; Select Touching - &#8984;T";
menu_commands["Mac"]["edit_select_by_file_position"]	= "Edit &raquo; Select by Line Numbers...";
menu_commands["Mac"]["edit_select_none"]				= "Edit &raquo; Select None - &#8679;&#8984;A";
menu_commands["Mac"]["edit_rotate_tool"]				= "Edit &raquo; Tools &raquo; Rotate Objects Tool - R";
menu_commands["Mac"]["edit_clip_tool"]					= "Edit &raquo; Tools &raquo; Clip Tool - C";
menu_commands["Mac"]["edit_clip_side"]					= "Edit &raquo; Tools &raquo; Toggle Clip Side - &#x21E5;";
menu_commands["Mac"]["edit_perform_clip"]				= "Edit &raquo; Tools &raquo; Perform Clip - &#x21A9;";
menu_commands["Mac"]["edit_flip_horizontally"]			= "Edit &raquo; Actions &raquo; Flip Horizontally - &#8984;F";
menu_commands["Mac"]["edit_flip_vertically"]			= "Edit &raquo; Actions &raquo; Flip Vertically - &#x2325;&#8984;F";
menu_commands["Mac"]["edit_duplicate_selection"]		= "Edit &raquo; Actions &raquo; Duplicate - &#8984;D";
menu_commands["Mac"]["edit_map_properties"]				= "Edit &raquo; Map Properties...";

menu_commands["Mac"]["edit_undo"]						= "Edit &raquo; Undo - &#8984;Z";
menu_commands["Mac"]["edit_redo"]						= "Edit &raquo; Redo - &#8984;Y";

menu_commands["Mac"]["edit_hide_selected"]				= "Edit &raquo; Hide Selected - &#8984;H";
menu_commands["Mac"]["edit_hide_unselected"]			= "Edit &raquo; Hide Unselected - &#8984;&#x2325;H";
menu_commands["Mac"]["edit_unhide_all"]					= "Edit &raquo; Unhide All - &#8984;&#8679;H";
menu_commands["Mac"]["edit_lock_selected"]				= "Edit &raquo; Lock Selected - &#8984;L";
menu_commands["Mac"]["edit_lock_unselected"]			= "Edit &raquo; Lock Unselected - &#8984;&#x2325;L";
menu_commands["Mac"]["edit_unlock_all"]					= "Edit &raquo; Unlock All - &#8984;&#8679;L";

menu_commands["Mac"]["edit_actions_correct_verts"]		= "Edit &raquo; Actions &raquo; Correct Vertices";
menu_commands["Mac"]["edit_actions_snap_verts"]			= "Edit &raquo; Actions &raquo; Snap Vertices";

menu_commands["Mac"]["view_center_on_selection"]		= "View &raquo; Camera &raquo; Center on Selection - &#x2325;C";

menu_commands["Mac"]["view_camera_move_to_next_pt"]	= "View &raquo; Camera &raquo; Move Camera to Next Point, &#x2325; +";
menu_commands["Mac"]["view_camera_move_to_prev_pt"]	= "View &raquo; Camera &raquo; Move Camera to Previous Point, &#x2325; -";


menu_commands["Windows"]["preferences"] 				= "View &raquo; Preferences";
menu_commands["Windows"]["file_new"]					= "File &raquo; New - Ctrl+N";
menu_commands["Windows"]["file_open"]					= "File &raquo; Open... - Ctrl+O";
menu_commands["Windows"]["file_save"]					= "File &raquo; Save - Ctrl+S";
menu_commands["Windows"]["file_save_as"]				= "File &raquo; Save as... - Ctrl+Shift+S";

menu_commands["Windows"]["file_load_pointfile"]			= "File &raquo; Load Pointfile";
menu_commands["Windows"]["file_reload_pointfile"]		= "File &raquo; Reload Pointfile";
menu_commands["Windows"]["file_unload_pointfile"]		= "File &raquo; Unload Pointfile";

menu_commands["Windows"]["edit_copy"]					= "Edit &raquo; Copy - Ctrl+C";
menu_commands["Windows"]["edit_cut"]					= "Edit &raquo; Cut - Ctrl+X";
menu_commands["Windows"]["edit_paste"]					= "Edit &raquo; Paste - Ctrl+V";
menu_commands["Windows"]["edit_paste_at_original_pos"]	= "Edit &raquo; Paste At Original Position - Ctrl+Shift+V";
menu_commands["Windows"]["edit_select_all"]				= "Edit &raquo; Select All - Ctrl+A";
menu_commands["Windows"]["edit_select_siblings"]		= "Edit &raquo; Select Siblings - Ctrl+Alt+A";
menu_commands["Windows"]["edit_select_touching"]		= "Edit &raquo; Select Touching - Ctrl+T";
menu_commands["Windows"]["edit_select_by_file_position"]= "Edit &raquo; Select by Line Numbers...";
menu_commands["Windows"]["edit_select_none"]			= "Edit &raquo; Select None - Ctrl+Shift+A";
menu_commands["Windows"]["edit_rotate_tool"]			= "Edit &raquo; Tools &raquo; Rotate Objects Tool - R";
menu_commands["Windows"]["edit_clip_tool"]				= "Edit &raquo; Tools &raquo; Clip Tool - C";
menu_commands["Windows"]["edit_clip_side"]				= "Edit &raquo; Tools &raquo; Toggle Clip Side - Tab";
menu_commands["Windows"]["edit_perform_clip"]			= "Edit &raquo; Tools &raquo; Perform Clip - Enter";
menu_commands["Windows"]["edit_flip_horizontally"]		= "Edit &raquo; Actions &raquo; Flip Horizontally - Ctrl+F";
menu_commands["Windows"]["edit_flip_vertically"]		= "Edit &raquo; Actions &raquo; Flip Vertically - Ctrl+Alt+F";
menu_commands["Windows"]["edit_duplicate_selection"]	= "Edit &raquo; Actions &raquo; Duplicate - Ctrl+D";
menu_commands["Windows"]["edit_map_properties"]			= "Edit &raquo; Map Properties...";

menu_commands["Windows"]["edit_undo"]					= "Edit &raquo; Undo - Ctrl+Z";
menu_commands["Windows"]["edit_redo"]					= "Edit &raquo; Redo - Ctrl+Y";

menu_commands["Windows"]["edit_hide_selected"]			= "Edit &raquo; Hide Selected - Ctrl+H";
menu_commands["Windows"]["edit_hide_unselected"]		= "Edit &raquo; Hide Unselected - Ctrl+Alt+H";
menu_commands["Windows"]["edit_unhide_all"]				= "Edit &raquo; Unhide All - Ctrl+Shift+H";
menu_commands["Windows"]["edit_lock_selected"]			= "Edit &raquo; Lock Selected - Ctrl+L";
menu_commands["Windows"]["edit_lock_unselected"]		= "Edit &raquo; Lock Unselected - Ctrl+Alt+L";
menu_commands["Windows"]["edit_unlock_all"]				= "Edit &raquo; Unlock All - Ctrl+Shift+L";

menu_commands["Windows"]["edit_actions_correct_verts"]	= "Edit &raquo; Actions &raquo; Correct Vertices";
menu_commands["Windows"]["edit_actions_snap_verts"]		= "Edit &raquo; Actions &raquo; Snap Vertices";

menu_commands["Windows"]["view_center_on_selection"]	= "View &raquo; Camera &raquo; Center on Selection - Alt+C";
menu_commands["Windows"]["view_camera_move_to_next_pt"]	= "View &raquo; Camera &raquo; Move Camera to Next Point - Alt+(Plus (+))";
menu_commands["Windows"]["view_camera_move_to_prev_pt"]	= "View &raquo; Camera &raquo; Move Camera to Previous Point - Alt+(Minus (-))";

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
keys["Mac"]["Delete"]	= "&#x232B;";
keys["Mac"]["Return"]	= "&#x21A9;";
keys["Mac"]["Tab"]		= "&#x21E5;";
keys["Mac"]["Combiner"] = "";
keys["Mac"]["ArrowKeys"]	= "(&#x2191;, &#x2193;, &#x2190;, &#x2192;)";

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
keys["Windows"]["Delete"]	= "Del";
keys["Windows"]["Return"]	= "Enter";
keys["Windows"]["Tab"]		= "Tab";
keys["Windows"]["Combiner"] = "+";
keys["Windows"]["ArrowKeys"]	= "(&#x2191;, &#x2193;, &#x2190;, &#x2192;)";

keys["Linux"] = new Object();

var platform = "Windows";
if (navigator.platform.indexOf("Win")!=-1) platform="Windows";
if (navigator.platform.indexOf("Mac")!=-1) platform="Mac";
if (navigator.platform.indexOf("X11")!=-1) platform="Linux";
if (navigator.platform.indexOf("Linux")!=-1) OSName="Linux";
//platform = "Mac"; //debug

function print_menu_command(name) {
	document.write("<b>" + menu_commands[platform][name] + "</b>");
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

function open_cheat_sheet() {
	if (platform == "Mac")
		top.frames["content"].location.href = "images/cheat_sheet_mac.pdf";
	else
		top.frames["content"].location.href = "images/cheat_sheet_win.pdf";
}
