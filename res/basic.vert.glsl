#version 450

vec2 c_positions[] = vec2[] (
                             vec2( 0.0, -0.5),
                             vec2(-0.5,  0.5),
                             vec2( 0.5,  0.5)
                             );

vec4 c_colors[] = vec4[] (
                          vec4(0.8, 0.2, 0.3, 1.0),
                          vec4(0.3, 0.9, 0.4, 1.0),
                          vec4(0.5, 0.4, 0.8, 1.0)
                          );

layout (location = 0) out vec4 v_color;

void main() {
    gl_Position = vec4(c_positions[gl_VertexIndex], 0.0, 1.0);
    v_color = c_colors[gl_VertexIndex];
}
