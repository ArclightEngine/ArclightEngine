#version 300 es

precision mediump float;

uniform sampler2D texSampler; // Texture sampler
uniform int textureFormat;

in vec4 fragColour; // Fragment colour
in vec2 fragTexCoord; // Texture coordinate

out vec4 outColour; // Output colour

void main() {
    if(textureFormat == 0){
        outColour = fragColour * texture(texSampler, fragTexCoord);
        if(outColour.a < 0)
            discard;
    } else {
        outColour = fragColour;
        outColour.a = texture(texSampler, fragTexCoord).x;
    }
}
