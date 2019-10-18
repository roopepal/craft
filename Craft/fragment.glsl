#version 130

in vec4 texcoord;
uniform sampler2D u_texture;
out vec4 out_color;

const vec4 fog_color = vec4(0.6, 0.8, 1.0, 1.0);
const float fog_density = 0.00005;

void main()
{
	vec4 color;

	float w = 6.0; // columns in atlas
	float h = 3.0; // rows in atlas
	
	if (texcoord.w < 0) // top and bottom
	{
		float type = abs(texcoord.w);
		float x = clamp(fract(texcoord.x), 0.01, 0.99) / w + mod((type - 1.0), w) / w;
		float y = clamp(fract(texcoord.z), 0.01, 0.99) / h + int((type - 1.0 - 0.001) / w) / h;
		color = texture2D(u_texture, vec2(x, y));
	}
	else
	{
		float x = clamp(fract(texcoord.x + texcoord.z), 0.01, 0.99) / w + mod((texcoord.w - 1.0), w) / w;
		float y = clamp(fract(texcoord.y), 0.01, 0.99) / h + int((texcoord.w - 1.0 - 0.001) / w) / h;
		color = texture2D(u_texture, vec2(x, y)) * 0.8;
	}

	float z = gl_FragCoord.z / gl_FragCoord.w; // distance to camera
	float fog_strength = clamp(exp(-fog_density * z * z), 0.2, 1);

	out_color = mix(fog_color, color, fog_strength);
}