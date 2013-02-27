<?php include 'header.php' ?>
<ul class="menu">
	<li><a href="index.php">About</a></li>
	<li>Download</li>
	<li><a href="http://github.com/kduske/TrenchBroom">Source</a></li>
	<li><a href="http://celephais.net/board/view_thread.php?id=60674">Discuss</a></li>
	<li><a href="http://github.com/kduske/TrenchBroom/issues">Contribute</a></li>
	<li><a href="mailto:kristian.duske@gmail.com">Contact</a></li>
</ul>
<div id="content">
	<div class="downloads">
		<h2>Windows</h2>
<?php
$dir = "downloads/windows";
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
		<a href="<?php echo urlencode($dir . '/' . $latest) ?>"><?php echo $latest ?></a>
	</div>
	<div class="requirements">
		<h3>Requirements</h3>
		<a href="http://www.microsoft.com/en-us/download/details.aspx?id=5555" target="_blank">Microsoft Visual C++ 2010 Redistributable Package</a>
	</div>
<?php
		if (count($files) > 1) {
?>
	<h3>Older Releases</h3>
<?php
			for ($i = 1; $i < count($files); $i++) {
?>
		<a href="<?php echo urlencode($dir . '/' . $files[$i]) ?>"><?php echo $files[$i] ?></a><br />
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
		<a href="<?php echo urlencode($dir . '/' . $latest) ?>"><?php echo $latest ?></a>
	</div>
<?php
		if (count($files) > 1) {
?>
	<h3>Older Releases</h3>
<?php
			for ($i = 1; $i < count($files); $i++) {
?>
		<a href="<?php echo urlencode($dir . '/' . $files[$i]) ?>"><?php echo $files[$i] ?></a><br />
<?php
			}
		}
	}
}
?>
	</div>
</div>
<?php include 'footer.php' ?>
