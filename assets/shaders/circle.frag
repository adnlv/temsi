#version 460 core

in float radius;
in vec2 tex_coords;
in vec3 color;

uniform vec2 u_viewport_px;
uniform vec2 u_half_extent;

out vec4 frag_color;

void main()
{
	float radius_px = radius / u_half_extent.y * u_viewport_px.y * 0.5;
	float fade = 1.0 / radius_px;
	float dist = length(tex_coords);
	float alpha = 1.0 - smoothstep(1.0 - fade, 1.0, dist);

	frag_color = vec4(color, alpha);
}
