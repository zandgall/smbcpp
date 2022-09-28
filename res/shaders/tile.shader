#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;

varying vec2 uv;

uniform mat4 screenspace, transform;
uniform mat3 uvtransform;
void main() {
	gl_Position = screenspace * transform * vec4(aPos, 1);
	uv = (uvtransform * vec3(aPos.xy.xy*vec2(0.5, 0.5) + vec2(0.5), 1)).xy;
	uv.y = 1 - uv.y;
}

#shader fragment
#version 460 core

varying vec2 uv;

uniform sampler2D sprite; // A texture containing UV data, pointing to coordinates on the object_palette
uniform sampler2D object_palette; // A texture containing UV data pointing to coordinates on the game palette. Used in blit shader

uniform bool check_transparency;
out vec4 glColor;
void main() {
	if (check_transparency && texture2D(sprite, uv - vec2(0, 0.5)).w == 0)
		discard;
	glColor = texture2D(object_palette, vec2(texture2D(sprite, uv).x, 1));
}