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
    int width = videoTexture.getWidth();
    int height = videoTexture.getHeight();


    float maxDim = std::max(width, height); // keep proportions
    float scaleX = (float)width / maxDim;
    float scaleY = (float)height / maxDim;

    distortedFrame.begin();
    ofClear(0, 0, 0, 255);

    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);

    int step = 10; // controls how fine the mesh is

    for (int y = 0; y < height - step; y += step) {
        for (int x = 0; x < width - step; x += step) {
            for (int dy = 0; dy <= step; dy += step) {
                for (int dx = 0; dx <= step; dx += step) {
                    float srcX = x + dx;
                    float srcY = y + dy;

                    // Normalise coordinates to [-1, 1] with aspect ratio preserved
                    float nx = ((srcX / width) * 2.0f - 1.0f) / scaleX;
                    float ny = ((srcY / height) * 2.0f - 1.0f) / scaleY;

                    // Compute radius from center
                    float r = sqrt(nx * nx + ny * ny);

                    // Applies fisheye distortion
                    float theta = atan(r);
                    float distortedR = (r > 0.0f) ? theta / r : 1.0f;
                    distortedR = 1.0 + distortionStrength * (distortedR - 1.0);

                    float distortedX = nx * distortedR;
                    float distortedY = ny * distortedR;

                    // Bring back to 0 width, 0 height space
                    float u = ((distortedX * scaleX) + 1.0f) * 0.5f * width;
                    float v = ((distortedY * scaleY) + 1.0f) * 0.5f * height;

                    mesh.addVertex(glm::vec3(srcX, srcY, 0));
                    mesh.addTexCoord(glm::vec2(u, v));
                }
            }

            int i = mesh.getNumVertices();
            mesh.addIndex(i - 4);
            mesh.addIndex(i - 3);
            mesh.addIndex(i - 2);

            mesh.addIndex(i - 4);
            mesh.addIndex(i - 2);
            mesh.addIndex(i - 1);
        }
    }

    videoTexture.bind();
    mesh.draw();
    videoTexture.unbind();

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
