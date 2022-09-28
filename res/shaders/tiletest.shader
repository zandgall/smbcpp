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
uniform vec3 col;
uniform sampler2D sprite;
uniform sampler2D objectPalette;
uniform sampler2D gamePalette;
out vec4 glColor;
void main() {
	if (texture2D(sprite, uv - vec2(0, 0.5)).w * texture2D(sprite, uv).w == 0)
		discard;
	glColor = texture2D(gamePalette, vec2(texture2D(objectPalette, vec2(texture2D(sprite, uv).x, 1)).x, 1));
	//glColor = uvtransform * vec3()
	//glColor = vec4(objpalcoords.x / 64.f, 0, 0, 1);
	//glColor = texture2D(objectPalette, uv);
}