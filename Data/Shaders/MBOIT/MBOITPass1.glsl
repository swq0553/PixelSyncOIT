
#define REQUIRE_INVOCATION_INTERLOCK
#define MOMENT_GENERATION 1

#include "MBOITHeader.glsl"
#include "MomentOIT.glsl"

out vec4 fragColor;

void gatherFragment(vec4 color)
{
	float depth = gl_FragCoord.z;//logDepthWarp(gl_FragCoord.z, logDepthMin, logDepthMax); // gl_FragCoord.z
	float transmittance = 1.0 - color.a;

	memoryBarrierImage();
	generateMoments(depth, transmittance, gl_FragCoord.xy, MomentOIT.wrapping_zone_parameters);

	fragColor = vec4(color);
	//fragColor = vec4(0.0,0.0,1.0,1.0);
	//fragColor = vec4(0.0, 0.0, 1.0, 1.0);
}
