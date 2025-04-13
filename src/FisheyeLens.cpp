#include "FisheyeLens.hpp"

// sets a default distortion strength
FisheyeLens::FisheyeLens() {
    distortionStrength = 0.5f;
}

//sets up the FBO (frame buffer object) for the distorted output and allows initial strength to be passed in
void FisheyeLens::setup(float _distortionStrength) {
    distortionStrength = _distortionStrength;

    // Allocates the FBO the same size as the window
    distortedFrame.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
}

//Applies the fisheye effect to a texture
void FisheyeLens::update(const ofTexture &videoTexture) {
    //Rendering into the offscreen FBO
    distortedFrame.begin();
    ofClear(0, 0, 0, 255); // Clear with full transparency

    int width = videoTexture.getWidth();
    int height = videoTexture.getHeight();

    // Reads pixels from the texture into an ofImage
    ofPixels pixels;
    ofImage image;
    image.allocate(width, height, OF_IMAGE_COLOR_ALPHA); // Allocate the image buffer
    videoTexture.readToPixels(image.getPixels());        // Copy pixels from the texture
    pixels = image.getPixels();
    image.update();                                      // Push pixel changes to the GPU

    // Mesh that will be used to remap the image using fisheye distortion
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);

    int step = 10; // The step controls mesh resolution, lower num - finer res, higher - faster loading

    // Loopx through the image in a grid pattern, adding vertices and distorted texture coordinates
    for (int y = 0; y < height; y += step) {
        for (int x = 0; x < width; x += step) {
            // Normalised UV coordinates (0-1)
            float u = x / float(width);
            float v = y / float(height);

            // Convertx UV to normalised screen space (-1 to 1)
            float nx = u * 2.0 - 1.0;
            float ny = v * 2.0 - 1.0;
            float r = sqrt(nx * nx + ny * ny); // Distance from center

            float distortion = 1.0;
            if (r < 1.0) {
                // Radial fisheye distortion
                distortion = 1.0 + distortionStrength * (r * r);
            }

            // Applies distortion to normalised coordinates
            float dx = nx * distortion;
            float dy = ny * distortion;

            // Converts back to pixel space
            float distortedX = (dx + 1.0) * 0.5 * width;
            float distortedY = (dy + 1.0) * 0.5 * height;

            // Adds original screen vertex
            mesh.addVertex(glm::vec3(x, y, 0));
            // Uses distorted UVs for sampling the texture
            mesh.addTexCoord(glm::vec2(distortedX, distortedY));
        }
    }

    // Binds the images texture and draw the distorted mesh
    image.getTexture().bind();
    mesh.draw();
    image.getTexture().unbind();

    // End FBO rendering
    distortedFrame.end();
}

// Draws the distorted result at a certain position and size on screen
void FisheyeLens::apply(float x, float y, float width, float height) {
    ofSetColor(255); // Resets tint color to white, so theres no colour/tint
    distortedFrame.draw(x, y, width, height);
}

// Allows real-time control of the distortion strength
void FisheyeLens::setDistortionStrength(float strength) {
    distortionStrength = strength;
}

// Getter for current distortion strength
float FisheyeLens::getDistortionStrength() const {
    return distortionStrength;
}
