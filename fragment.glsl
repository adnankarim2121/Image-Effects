// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage

in vec2 Texture;
in vec2 FragmentPosition;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;
uniform sampler2DRect tex;
uniform vec3 luminance;
uniform int sepia;
uniform int sob;
uniform int gauss;

//help from : gist.github.com/Hebali/6ebfc66106459aacee6a9fac029d0115
vec4 edge_effect(){
	float so_horizontal[9] = float[9](1.0,0.0,-1.0,2.0,0.0,-2.0,1.0,-2.0,-1.0);
	float so_vertical[9] = float[9](-1.0,-2.0,-1.0,0.0,0.0,0.0,1.0,2.0,1.0);
	float so_unsharp[9] = float[9](0.0,-1.0,0.0,-1.0,5.0,-1.0,0.0,-1.0,0.0);

	vec4 result = vec4(0.0,0.0,0.0,1.0);
	vec2 offset[9];

	offset[0] = vec2(-1,-1);
	offset[1] = vec2(0.0,-1);
	offset[2] = vec2(1,-1);

	offset[3] = vec2(-1,0.0);
	offset[4] = vec2(0.0,0.0);
	offset[5] = vec2(1,0.0);

	offset[6] = vec2(-1,1);
	offset[7] = vec2(0.0,1);
	offset[8] = vec2(1,1);

	for (int i = 0; i<9; i++)
	{
		vec4 temp = texture(tex, Texture + offset[i]);
		if (sob == 1)
		{
			result+=temp * so_horizontal[i];
		}

		else if( sob == 2)
		{
			result+=temp * so_vertical[i];
		}

		else if( sob == 3)
		{
			result+=temp * so_unsharp[i];
		}
	}

	return result;

}

vec4 gaussian_blur()
{

	//for sigma, i just ended up hardcoding the values for the specific gauss values.
	//for some reason, when i tried sigma = gauss/2 and proceeded with the calculations,
	//a black screen just appears, which is odd because when i hardcode 0.667 it works,
	// but 2/gauss when gauss is 3 doesnt work. 
	float sigmaVal;
	
	if(gauss == 3)
	{
		sigmaVal = 0.6666666;
	}

	if(gauss == 5)
	{
		sigmaVal = 0.997;

	}

	if(gauss == 7)
	{
		sigmaVal = 1.50;
	}

	float middleOfKernal = floor(gauss/2);

	vec4 result = vec4(0.0,0.0,0.0,1.0);

	for(int i=0; i<gauss; i++)
	{
		for(int j=0; j<gauss; j++)
		{
			vec4 storage = texture(tex,Texture + i - middleOfKernal + j - middleOfKernal);

			float gaussCalc = exp(-1 * (pow(i-middleOfKernal,2) + pow(j-middleOfKernal,2)) / (2*pow(sigmaVal,2))) / (2 * 3.1415 * pow(sigmaVal,2));

			result.r += storage.r * gaussCalc;
			result.g += storage.g * gaussCalc;
			result.b += storage.b * gaussCalc;
		}
	}

	return result;
}

void main(void)
{

	vec4 change = texture(tex, Texture);
    
    if(luminance.r < 1 ){
    	float factor = change.r*luminance.r + change.g*luminance.g + change.b*luminance.b;
    	change.r = factor;
    	change.g = factor;
    	change.b = factor;
    }

    //sepia color from: alastaira.wordpress.com/2013/12/02/sepia-shader/
    if(sepia > 0)
    {
    	change.r = change.r*0.393 + change.g*0.769 + change.b*0.189;
    	change.g = change.r*0.349 + change.g*0.686 + change.b*0.168;
    	change.b = change.r*0.272 + change.g*0.534 + change.b*0.131;
    }

    if(sob > 0)
    {
    	change = edge_effect();
    }

    if(gauss > 0)
    {
    	change = gaussian_blur();
    }

    // write colour output without modification
    //FragmentColour = vec4(Colour, 0);
 	FragmentColour = change;
}
