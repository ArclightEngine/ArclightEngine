#version 300 es

precision mediump float;

uniform sampler2D texSampler; // Texture sampler

in vec4 fragColour; // Fragment colour
in vec2 fragTexCoord; // Texture coordinate

out vec4 outColour; // Output colour

void main() {
    outColour = fragColour * texture(texSampler, fragTexCoord);
}
