<?php include 'header.php' ?>
<ul class="menu">
	<li><a href="index.php">About</a></li>
	<li>Download</li>
	<li><a href="http://celephais.net/board/view_thread.php?id=60674">Discuss</a></li>
	<li><a href="mailto:kristian.duske@gmail.com">Contact</a></li>
</ul>
<div id="content">
<?php
$dir = "downloads";
if (is_dir($dir) && $dir_handle = opendir($dir)) {
	$files = array();
	while (($file = readdir($dir_handle)) !== false)
		$files[] = $file;
	rsort($files);

	if (count($files) > 0) {
		$latest = $files[0];
?>
	<div class="latest">
		<p>The latest release of TrenchBroom is <a href="downloads/<?php $file ?>"><?php $file ?></a>.
	</div>
	<table class="releases">
<?PHP
		for ($i = 1; $i < count($files); $i++) {
?>
		<tr><td><a href="downloads/<?php $file ?>"><?php $file ?></a></td></tr>
<?php
		}
	}
}
?>
	</table>
</div>
<?php include 'footer.php' ?>
