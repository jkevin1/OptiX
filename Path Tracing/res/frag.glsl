#version 330 core

uniform sampler2D image;
uniform float invWidth;
uniform float invHeight;

out vec3 color;

// Blurs on axes with weights 0.1, 0.2, 0.4, 0.2, 0.1
void main() {
	float weights[3] = float[3](0.0, 0.5, 0.25);
	vec2 pos = gl_FragCoord.xy * vec2(invWidth, invHeight);
	vec4 data = texture(image, pos);

	vec3 accum = 2.0 * data.xyz;
	float sum = 2.0;

	// Unrolled
	for (int i = 1; i < 3; i++) {
		if (pos.x > i * invWidth) {
			vec4 c = texture(image, pos + vec2(i * invWidth, 0));
			if (c.w == data.w) {
				accum += weights[i] * c.xyz;
				sum += weights[i];
			}
		}

		if (pos.y > i * invHeight) {
			vec4 c = texture(image, pos + vec2(0, i * invWidth));
			if (c.w == data.w) {
				accum += weights[i] * c.xyz;
				sum += weights[i];
			}
		}

		if (pos.x < 1.0 - i * invWidth) {
			vec4 c = texture(image, pos - vec2(i * invWidth, 0));
			if (c.w == data.w) {
				accum += weights[i] * c.xyz;
				sum += weights[i];
			}
		}

		if (pos.y > 1.0 - i * invHeight) {
			vec4 c = texture(image, pos - vec2(0, i * invWidth));
			if (c.w == data.w) {
				accum += weights[i] * c.xyz;
				sum += weights[i];
			}
		}
	}

	vec2 uv = gl_FragCoord.xy * vec2(invWidth, invHeight);
    color = accum / sum;
}