// handles the string in a #key() macro
function key_str(key) {
    if (keys[key]) {
        return "<span class=\"shortcut\">" + keys[key] + "</span>";
    } else {
        console.error("unknown key ", key);
        return undefined;
    }
}

// Pandoc smart typography converts three periods to …, but this breaks
// our menu item lookups.
function fix_ellipsis(path) {
    return path.replace("…", "...");
}

function shortcut_str(shortcut) {
    let result = "";
    if (shortcut) {
        if (shortcut.key == "") {
            result = undefined;
        } else {
            for (i = 0; i < shortcut.modifiers.length; ++i) {
                result += key_str(shortcut.modifiers[i]);
            }
            result += key_str(shortcut.key);
        }
    } else {
        console.error("unknown shortcut ", shortcut);
        result += "&laquo;unknown shortcut&raquo;";
    }

    return result;
}

function menu_path_str(path) {
    return path.join(" &raquo; ");
}

// handles the string in a #menu() macro
function menu_item_str(key) {
    key = fix_ellipsis(key);

    let result = "<b>";
    const item = menu[key];
    if (item) {
        result += menu_path_str(item.path);
        const shortcut = shortcut_str(item.shortcut);
        if (shortcut) {
            result += " (" + shortcut + ")";
        }
    } else {
        console.error("unknown menu item ", key);
        result += "unknown menu item \"" + key + "\"";
    }
    result += "</b>";
    return result;
}

// handles the string in an #action() macro
function action_str(key) {
    key = fix_ellipsis(key);

    let result = "<b>";
    const item = actions[key];
    if (item) {
        result += shortcut_str(item);
    } else {
        console.error("unknown action ", key);
        result += "unknown action \"" + key + "\"";
    }
    result += "</b>";
    return result;
}

// #key() macros expand into calls to this
function print_key(key) {
    document.write(key_str(key));
}

// #menu() macros expand into calls to this
function print_menu_item(key) {
    document.write(menu_item_str(key));
}

// #action() macros expand into calls to this
function print_action(key) {
    document.write(action_str(key));
}
