textures/fail/test // overrides an existing texture name
{

    // has existing editor image
    qer_editorimage textures/test/editor_image.jpg
    surfaceparm noimpact
}} // parse error here

textures/fail/not_existing // does not override an existing name
{

    // has existing editor image
    qer_editorimage textures/test/editor_image.jpg
}

textures/fail/test2 // overrides an existing texture name
{
    // has missing editor image
    qer_editorimage textures/test/editor_image_missing.jpg
}

textures/fail/not_existing2 // does not override an existing name
{
    // no editor image
}
