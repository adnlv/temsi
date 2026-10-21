#version 460 core

layout (location = 0) in float a_radius;
layout (location = 1) in vec2 a_center;
layout (location = 2) in vec3 a_pos;
layout (location = 3) in vec3 a_color;

uniform vec2 u_half_extent;

out float radius;
out vec2 tex_coords;
out vec3 color;

void main()
{
	vec2 world_pos = a_pos.xy * a_radius + a_center;
	gl_Position = vec4(world_pos / u_half_extent, a_pos.z, 1.0);

	tex_coords = a_pos.xy;
	radius = a_radius;
	color = a_color;
}
