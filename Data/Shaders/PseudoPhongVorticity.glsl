-- Vertex

#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoordinates;

out vec4 fragmentColor;
out vec3 fragmentNormal;
out vec3 fragmentPositonLocal;
out vec3 screenSpacePosition;
out float vorticity;

// Color of the object
uniform vec4 color;

void main()
{
	fragmentColor = color;
	fragmentNormal = vertexNormal;
	fragmentPositonLocal = (vec4(vertexPosition, 1.0)).xyz;
	screenSpacePosition = (vMatrix * mMatrix * vec4(vertexPosition, 1.0)).xyz;
	vorticity = textureCoordinates.x;
	gl_Position = mvpMatrix * vec4(vertexPosition, 1.0);
}


-- Fragment

#version 430 core

in vec3 screenSpacePosition;

#ifndef DIRECT_BLIT_GATHER
#include OIT_GATHER_HEADER
#endif

in vec4 fragmentColor;
in vec3 fragmentNormal;
in vec3 fragmentPositonLocal;
in float vorticity;

#ifdef DIRECT_BLIT_GATHER
out vec4 fragColor;
#endif

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float specularExponent;
uniform float opacity;
uniform int bandedColorShading = 1;

uniform float minVorticity;
uniform float maxVorticity;

void main()
{
	// Use vorticity
	float linearFactor = (vorticity - minVorticity) / (maxVorticity - minVorticity);
	vec4 diffuseColorVorticity = mix(vec4(1.0,1.0,1.0,0.0), vec4(1.0,0.0,0.0,1.0), linearFactor);

	vec3 lightDirection = vec3(1.0,0.0,0.0);
	vec3 ambientShading = ambientColor * 0.1;
	vec3 diffuseShadingVorticity = diffuseColorVorticity.rgb * clamp(dot(fragmentNormal, lightDirection)/2.0+0.75, 0.0, 1.0);
	vec3 diffuseShading = diffuseColor * clamp(dot(fragmentNormal, lightDirection)/2.0+0.75, 0.0, 1.0) * 0.00001;
	vec3 specularShading = specularColor * specularExponent * 0.00001; // In order not to get an unused warning
	vec4 color = vec4(ambientShading + diffuseShadingVorticity + diffuseShading + specularShading,
	    opacity * 0.00001 + diffuseColorVorticity.a * fragmentColor.a);
	color = vec4(diffuseShadingVorticity, diffuseColorVorticity.a * fragmentColor.a);

    if (color.a < 1.0/255.0) {
        discard;
    }

#ifdef DIRECT_BLIT_GATHER
	// Direct rendering
	fragColor = color;
#else
#ifdef REQUIRE_INVOCATION_INTERLOCK
	// Area of mutual exclusion for fragments mapping to the same pixel
	beginInvocationInterlockARB();
	gatherFragment(color);
	endInvocationInterlockARB();
#else
	gatherFragment(color);
#endif
#endif
}