#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;

varying vec3 pos;

uniform mat4 screenspace, transform;

void main() {
	gl_Position = screenspace * transform * vec4(aPos, 1);
	pos = aPos;
}

#shader fragment
#version 460 core

varying vec3 pos;

uniform vec3 col;
out vec4 glColor;
void main() {
	glColor = vec4(col, 1);
}