var platform = "win";
if (navigator.platform.indexOf("Win")!=-1) platform="windows";
if (navigator.platform.indexOf("Mac")!=-1) platform="mac";
if (navigator.platform.indexOf("X11")!=-1) platform="linux";
if (navigator.platform.indexOf("Linux")!=-1) OSName="linux";
//platform = "mac"; //debug

function key_str(key) {
	return "<span class=\"shortcut\">" + keys[platform][key] + "</span>";
}

function shortcut_str(shortcut) {
	var result = "";
	if (shortcut) {
		for (i = 0; i < shortcut.modifiers.length; ++i)
			result += key_str(shortcut.modifiers[i]);
		result += key_str(shortcut.key);
	} else {
		result += "&laquo;unknown shortcut&raquo;";
	}
	return result;
}

function menu_path_str(path) {
	var result = "";
	for (i = 0; i < path.length; ++i) {
		result += path[i];
		if (i < path.length - 1)
			result += " &raquo; ";
	}
	return result;
}

function menu_item_str(key) {
	var result = "<b>";
	var item = menu[key];
	if (item) {
		result += menu_path_str(item.path);
		result += " (" + shortcut_str(item.shortcut) + ")";
	} else {
		result += "unknown menu item \"" + key + "\"";
	}
	result += "</b>";
	return result;
}

function action_str(key) {
	var result = "<b>";
	var item = actions[key];
	if (item) {
		result += shortcut_str(item);
	} else {
		result += "unknown action \"" + key + "\"";
	}
	result += "</b>";
	return result;
}

function print_menu_item(key) {
	document.write(menu_item_str(key));
}

function print_action(key) {
	document.write(action_str(key));
}