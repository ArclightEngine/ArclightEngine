#version 310 es

precision mediump float;

layout(binding = 0) uniform sampler2D texSampler; // Texture sampler

layout(location = 0) in vec4 fragColour; // Fragment colour
layout(location = 1) in vec2 fragTexCoord; // Texture coordinate

layout(location = 0) out vec4 outColour; // Output colour

void main() {
    outColour = fragColour * texture(texSampler, fragTexCoord);
}
