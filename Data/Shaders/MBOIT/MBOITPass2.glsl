
#define MOMENT_GENERATION 0

#include "MBOITHeader.glsl"
#include "MomentOIT.glsl"

out vec4 fragColor;

void gatherFragment(vec4 color)
{
	float depth = logDepthWarp(-screenSpacePosition.z, logDepthMin, logDepthMax); // gl_FragCoord.z
	float transmittance_at_depth = 1.0;
	float total_transmittance = 1.0;  // exp(-b_0)
	resolveMoments(transmittance_at_depth, total_transmittance, depth, gl_FragCoord.xy);

    if (isinf(transmittance_at_depth)) {
        transmittance_at_depth = 0.0f;
    }

    // Normal back-to-front blending: c_out = c_src * alpha_src + c_dest * (1 - alpha_src)
    // Blended Color = exp(-b_0) * L_n + (1 - exp(-b_0))
    //		/ (Sum from l=0 to n-1:       alpha_l * T(z_f, b, beta))
    //		* (Sum from l=0 to n-1: L_l * alpha_l * T(z_f, b, beta))
    // => Set alpha_src = (1 - exp(-b_0)),
    //    Premultiply c_src with 1 / (Sum from l=0 to n-1: alpha_l * T(z_f, b, beta))
    fragColor = vec4(color.rgb * color.a * transmittance_at_depth, color.a * transmittance_at_depth);
}
