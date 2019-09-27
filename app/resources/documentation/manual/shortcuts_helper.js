function key_str(key) {
    if (keys[key]) {
        return "<span class=\"shortcut\">" + keys[key] + "</span>";
    } else {
        return undefined;
    }
}

function shortcut_str(shortcut) {
    let result = "";
    if (shortcut) {
        if (shortcut.key == 0) {
            result = undefined;
        } else {
            for (i = 0; i < shortcut.modifiers.length; ++i) {
                result += key_str(shortcut.modifiers[i]);
            }
            result += key_str(shortcut.key);
        }
    } else {
        result += "&laquo;unknown shortcut&raquo;";
    }

    return result;
}

function menu_path_str(path) {
    return path.join(" &raquo; ");
}

function menu_item_str(key) {
    let result = "<b>";
    const item = menu[key];
    if (item) {
        result += menu_path_str(item.path);
        const shortcut = shortcut_str(item.shortcut);
        if (shortcut) {
            result += " (" + shortcut + ")";
        }
    } else {
        result += "unknown menu item \"" + key + "\"";
    }
    result += "</b>";
    return result;
}

function action_str(key) {
    let result = "<b>";
    const item = actions[key];
    if (item) {
        result += shortcut_str(item);
    } else {
        result += "unknown action \"" + key + "\"";
    }
    result += "</b>";
    return result;
}

function print_key(key) {
    document.write(key_str(key));
}

function print_menu_item(key) {
    document.write(menu_item_str(key));
}

function print_action(key) {
    document.write(action_str(key));
}
