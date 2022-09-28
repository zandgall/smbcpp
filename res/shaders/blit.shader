#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;

varying vec2 uv;

//uniform mat4 screenspace, transform;
//uniform mat3 uvtransform;
void main() {
	//gl_Position = screenspace * transform * vec4(aPos, 1);
	gl_Position = vec4(aPos, 1);
	//uv = (uvtransform * vec3(aPos.xy.xy*vec2(0.5, 0.5) + vec2(0.5), 1)).xy;
	uv = aPos.xy * 0.5 + vec2(0.5);
	//uv.y = 1 - uv.y;
}

#shader fragment
#version 460 core

varying vec2 uv;
//uniform vec3 col;
uniform sampler2D image;
uniform sampler2D gamepalette;
//uniform bool objectMode;
out vec4 glColor;
void main() {
	//if (objectMode && texture2D(sprite, uv - vec2(0, 0.5)).w == 0)
	//	discard;
	glColor = texture2D(gamepalette, vec2(texture2D(image, uv).x, 1));
	glColor.w = texture2D(image, uv).w;

	//glColor = texture2D(image, uv);
}