<?php include 'header.php' ?>
<ul class="menu">
	<li><a href="index.php">About</a></li>
	<li>Download</li>
	<li><a href="http://github.com/kduske/TrenchBroom">Source</a></li>
	<li><a href="docs/">Documentation</a></li>
	<li><a href="http://celephais.net/board/view_thread.php?id=60908">Discuss</a></li>
	<li><a href="http://github.com/kduske/TrenchBroom/issues">Contribute</a></li>
	<li><a href="mailto:kristian.duske@gmail.com">Contact</a></li>
</ul>
<div id="content">
	<div class="downloads">
		<h2>Windows</h2>
<?php
$dir = "downloads/win32";
if (is_dir($dir) && $dir_handle = opendir($dir)) {
	$files = array();
	while (($file = readdir($dir_handle)) !== false)
		if (!is_dir($file))
			$files[] = $file;
	rsort($files);

	if (count($files) > 0) {
		$latest = $files[0];
?>
	<div class="latest">
		<h3>Latest Release</h3>
		<a href="<?php echo $dir . '/' . urlencode($latest) ?>"><?php echo $latest ?></a>
	</div>
	<div class="requirements">
		<h3>Requirements</h3>
		<ul>
			<li><a href="http://www.microsoft.com/en-us/download/details.aspx?id=5555" target="_blank">Microsoft Visual C++ 2010 Redistributable Package</a></li>
			<li>Windows XP or newer</li>
			<li>OpenGL 2.1 and GLSL 1.2 capable driver</li>
		</ul>
	</div>
<?php
		if (count($files) > 1) {
?>
	<h3>Older Releases</h3>
<?php
			for ($i = 1; $i < count($files); $i++) {
?>
		<a href="<?php echo $dir . '/' . urlencode($files[$i]) ?>"><?php echo $files[$i] ?></a><br />
<?php
			}
		}
	}
}
?>
	</div>
	<div class="downloads" style="margin-left: 20px;">
		<h2>Mac OS X</h2>
<?php
$dir = "downloads/mac";
if (is_dir($dir) && $dir_handle = opendir($dir)) {
	$files = array();
	while (($file = readdir($dir_handle)) !== false)
		if (!is_dir($file))
			$files[] = $file;
	rsort($files);

	if (count($files) > 0) {
		$latest = $files[0];
?>
	<div class="latest">
		<h3>Latest Release</h3>
		<a href="<?php echo $dir . '/' . urlencode($latest) ?>"><?php echo $latest ?></a>
	</div>
	<div class="requirements">
		<h3>Requirements</h3>
		<ul>
			<li>Mac OS X 10.6 or newer</li>
		</ul>
	</div>
<?php
		if (count($files) > 1) {
?>
	<h3>Older Releases</h3>
<?php
			for ($i = 1; $i < count($files); $i++) {
?>
		<a href="<?php echo $dir . '/' . urlencode($files[$i]) ?>"><?php echo $files[$i] ?></a><br />
<?php
			}
		}
	}
}
?>
	</div>
</div>
<?php include 'footer.php' ?>
