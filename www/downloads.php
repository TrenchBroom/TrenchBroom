<?php
header('Location: https://github.com/kduske/TrenchBroom/releases');
die();

function has_extension($file, $exts) {
	foreach ($exts as $ext) {
		if (pathinfo($file, PATHINFO_EXTENSION) == $ext)
			return true;
	}
	return false;
}

function list_downloads($dir, $exts) {
	$files = array();
	if (is_dir($dir) && $dir_handle = opendir($dir)) {
		while (($file = readdir($dir_handle)) !== false) {
			$mtime = filemtime($dir . '/' . $file);
			if (!is_dir($file) && has_extension($file, $exts) && $mtime < (time() - 10))
				$files[$mtime] = $file;
		}
	}
	krsort($files);
	return $files;
}

function print_download_link($dir, $file) {
?>
<a href="<?php echo $dir . '/' . urlencode($file) ?>"><?php echo $file ?></a><br />
<?php
}

function print_download_list($dir, $files) {
	while (list($time, $file) = each($files))
		print_download_link($dir, $file);
}

function print_latest_download($dir, $files) {
	if (list($time, $file) = each($files)) {
?>
<div class="latest">
	<h3>Latest</h3>
	<?php print_download_link($dir, $file); ?>
</div>
<?php		
	}
}

function print_old_downloads($dir, $files) {
	if (current($files)) {
?>
<h3>All Releases</h3>
<?php
		print_download_list($dir, $files);
	}
}

function print_downloads($header, $dir, $exts) {
	$files = list_downloads($dir, $exts);
	if (count($files) > 0) {
?>
<div class="releases">
	<h2><?php echo($header); ?></h2>
<?php
		print_latest_download($dir, $files);
		print_old_downloads($dir, $files);
?>
</div>
<?php
	}
}

function print_all_downloads($basedir, $exts) {
	print_downloads("Stable", $basedir . "/stable", $exts);
	print_downloads("Beta", $basedir . "/beta", $exts);
}
?>
<?php include 'header.php' ?>
<div id="content">
<?php
if ($_REQUEST["platform"] == "win32") {
?>
	<div class="downloads">
		<h2>Windows</h2>
		<div class="requirements">
			<h3>Requirements</h3>
			<ul>
				<li><a href="http://www.microsoft.com/en-us/download/details.aspx?id=5555" target="_blank">Microsoft Visual C++ 2010 Redistributable Package</a></li>
				<li>Windows XP or newer</li>
				<li>OpenGL 2.1 and GLSL 1.2 capable driver</li>
			</ul>
		</div>
		<?php print_all_downloads("downloads/win32", array("zip", "7z")); ?>
	</div>
<?php	
} else if ($_REQUEST["platform"] == "macosx") {
?>
	<div class="downloads">
		<h2>Mac OS X</h2>
		<div class="requirements">
			<h3>Requirements</h3>
			<ul>
				<li>Mac OS X 10.6 or newer</li>
				<li>OpenGL 2.1 and GLSL 1.2 capable driver</li>
			</ul>
		</div>
		<?php print_all_downloads("downloads/macosx", array("dmg", "zip")); ?>
	</div>
<?php	
} else if ($_REQUEST["platform"] == "linux_deb") {
?>
	<div class="downloads">
		<h2>Linux DEB Packages</h2>
		<?php print_all_downloads("downloads/linux", array("deb")); ?>
	</div>
<?php	
} else if ($_REQUEST["platform"] == "linux_rpm") {
?>
	<div class="downloads">
		<h2>Linux RPM Packages</h2>
		<?php print_all_downloads("downloads/linux", array("rpm")); ?>
	</div>
<?php	
} else {
?>
	<div class="platforms">
		<h2>Choose Your Platform</h2>
		<ul>
			<li><a href="<?php echo($_SERVER['REQUEST_URI']); ?>?platform=win32">Windows</a></li>
			<li><a href="<?php echo($_SERVER['REQUEST_URI']); ?>?platform=macosx">Mac OS X</a></li>
			<li><a href="<?php echo($_SERVER['REQUEST_URI']); ?>?platform=linux_deb">Linux DEB Packages</a></li>
			<li><a href="<?php echo($_SERVER['REQUEST_URI']); ?>?platform=linux_rpm">Linux RPM Packages</a></li>
		</ul>
	</div>
<?php
}
?>
<?php include 'footer.php' ?>
