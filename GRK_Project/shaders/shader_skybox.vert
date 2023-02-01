#version 430 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 transformation;

out vec3 texCoord;

void main()
{
	vec4 pos = transformation * vec4(vertexPosition, 1.0f);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	texCoord = vec3(vertexPosition.x, vertexPosition.y, vertexPosition.z);
}