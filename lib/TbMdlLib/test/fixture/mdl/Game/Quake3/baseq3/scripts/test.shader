textures/test/test // overrides an existing texture name
{

    // has existing editor image
    qer_editorimage textures/test/editor_image.jpg
    surfaceparm noimpact
}

textures/test/not_existing // does not override an existing name
{

    // has existing editor image
    qer_editorimage textures/test/editor_image.jpg
}

textures/test/test2 // overrides an existing texture name
{
    // has missing editor image
    qer_editorimage textures/test/editor_image_missing.jpg
}

textures/test/not_existing2 // does not override an existing name
{
    // no editor image
}

textures/skies/hub1/dusk // is in a subdirectory
{
    // has existing editor image
    qer_editorimage textures/test/editor_image.jpg
}
