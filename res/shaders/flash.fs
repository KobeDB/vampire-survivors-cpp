#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;

void main(void) {
    // Fetch the original color
    vec4 color = texture(texture0, fragTexCoord) * fragColor;
    // Preserve alpha, force RGB to white
    gl_FragColor = vec4(1.0, 1.0, 1.0, color.a);
}
