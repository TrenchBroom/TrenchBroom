var platform = "win";
if (navigator.platform.indexOf("Win")!=-1) platform="windows";
if (navigator.platform.indexOf("Mac")!=-1) platform="mac";
if (navigator.platform.indexOf("X11")!=-1) platform="linux";
if (navigator.platform.indexOf("Linux")!=-1) OSName="linux";
//platform = "mac"; //debug

function shortcut(name) {
	var shortcut = shortcuts[name];

	document.write("<b>");
	if (shortcut) {
		for (i = 0; i < shortcut.modifiers.length; ++i) {
			document.write(keys[platform][shortcut.modifiers[i]]);
			if (i < shortcut.modifiers.length - 1)
				document.write(keys[platform]["+"])
		}
		document.write(keys[platform][shortcut.key]);
	} else {
		document.write("&laquo;unknown shortcut&raquo;");
	}
	document.write("</b>");
}
