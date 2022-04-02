#version 300 es

precision mediump float;

uniform Transform {
    mat4 viewport;
};
uniform mat4 canvas;
uniform mat4 transform;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 colour;

out vec4 fragColour;
out vec2 fragTexCoord;

void main() {
    gl_Position = viewport * canvas * transform * vec4(position, 0.0, 1.0);
    fragColour = colour;
    fragTexCoord = texCoord;
}
