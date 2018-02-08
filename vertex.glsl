// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec2 VertexTexture;

// output to be interpolated between vertices and passed to the fragment stage
out vec2 FragmentPosition;
out vec2 Texture;
out vec2 newVertexPosition;

uniform float zoom;
uniform vec2 shift;
uniform float theta;

void main()
{
	newVertexPosition.x = (VertexPosition.x * zoom) + shift.x/512;
	newVertexPosition.y = (VertexPosition.y * zoom) - shift.y/512;

	float tempx = newVertexPosition.x;
	float tempy = newVertexPosition.y;
	newVertexPosition.x = tempx*cos(theta) - tempy*sin(theta);
	newVertexPosition.y = tempx*sin(theta) + tempy*cos(theta);
    // assign vertex position without modification
    gl_Position = vec4(newVertexPosition, 0.0, 1.0);
    FragmentPosition = VertexPosition;
    // assign output colour to be interpolated
    Texture = VertexTexture;
}
