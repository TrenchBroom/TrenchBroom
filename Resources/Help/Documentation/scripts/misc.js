function cycle_image(img, image_array) {
	var current_src = $(img).attr("src");
	for (i = 0; i < image_array.length; i++) {
		if (current_src == image_array[i]) {
			$(img).attr({src : image_array[(i + 1) % image_array.length]});
			break;
		}
	}
}
