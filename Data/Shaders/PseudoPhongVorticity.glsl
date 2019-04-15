-- TriangleVertex

#version 430 core

#include "VertexAttributeNames.glsl"

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 3) in float VERTEX_ATTRIBUTE;

out vec3 fragmentNormal;
out vec3 fragmentTangent;
out vec3 fragmentPositonWorld;
out vec3 screenSpacePosition;
out float fragmentAttribute;
out float lineCurvature;
out float lineLength;

void main()
{
    fragmentNormal = vertexNormal;
    vec3 binormal = cross(vec3(0,0,1), vertexNormal);
    fragmentTangent = cross(binormal, fragmentNormal);

    fragmentPositonWorld = (mMatrix * vec4(vertexPosition, 1.0)).xyz;
    screenSpacePosition = (vMatrix * mMatrix * vec4(vertexPosition, 1.0)).xyz;
    fragmentAttribute = VERTEX_ATTRIBUTE;
    gl_Position = mvpMatrix * vec4(vertexPosition, 1.0);
}


-- Vertex

#version 430 core

#include "VertexAttributeNames.glsl"

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexLineNormal;
layout(location = 2) in vec3 vertexLineTangent;
layout(location = 3) in float VERTEX_ATTRIBUTE;

out VertexData
{
    vec3 linePosition;
    vec3 lineNormal;
    vec3 lineTangent;
    float lineAttribute;
};

void main()
{
    linePosition = vertexPosition;
    lineNormal = vertexLineNormal;
    lineTangent = vertexLineTangent;
    lineAttribute = VERTEX_ATTRIBUTE;
}


-- Geometry

#version 430 core

layout(lines) in;
layout(triangle_strip, max_vertices = 32) out;


uniform float radius = 0.001f;

in VertexData
{
    vec3 linePosition;
    vec3 lineNormal;
    vec3 lineTangent;
    float lineAttribute;
} v_in[];

out vec3 fragmentNormal;
out vec3 fragmentTangent;
out vec3 fragmentPositonWorld;
out vec3 screenSpacePosition;
out float fragmentAttribute;

#define NUM_SEGMENTS 5

void main()
{
    vec3 currentPoint = v_in[0].linePosition.xyz;
    vec3 nextPoint = v_in[1].linePosition.xyz;

    vec3 circlePointsCurrent[NUM_SEGMENTS];
    vec3 circlePointsNext[NUM_SEGMENTS];
    vec3 vertexNormalsCurrent[NUM_SEGMENTS];
    vec3 vertexNormalsNext[NUM_SEGMENTS];

    vec3 normalCurrent = v_in[0].lineNormal;
    vec3 tangentCurrent = v_in[0].lineTangent;
    vec3 binormalCurrent = cross(tangentCurrent, normalCurrent);
    vec3 normalNext = v_in[1].lineNormal;
    vec3 tangentNext = v_in[1].lineTangent;
    vec3 binormalNext = cross(tangentNext, normalNext);

    vec3 tangent = normalize(nextPoint - currentPoint);

    mat3 tangentFrameMatrixCurrent = mat3(normalCurrent, binormalCurrent, tangentCurrent);
    mat3 tangentFrameMatrixNext = mat3(normalNext, binormalNext, tangentNext);

    const float theta = 2.0 * 3.1415926 / float(NUM_SEGMENTS);
    const float tangetialFactor = tan(theta); // opposite / adjacent
    const float radialFactor = cos(theta); // adjacent / hypotenuse

    vec2 position = vec2(radius, 0.0);
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        vec3 point2DCurrent = tangentFrameMatrixCurrent * vec3(position, 0.0);
        vec3 point2DNext = tangentFrameMatrixNext * vec3(position, 0.0);
        circlePointsCurrent[i] = point2DCurrent.xyz + currentPoint;
        circlePointsNext[i] = point2DNext.xyz + nextPoint;
        vertexNormalsCurrent[i] = normalize(circlePointsCurrent[i] - currentPoint);
        vertexNormalsNext[i] = normalize(circlePointsNext[i] - nextPoint);

        // Add the tangent vector and correct the position using the radial factor.
        vec2 circleTangent = vec2(-position.y, position.x);
        position += tangetialFactor * circleTangent;
        position *= radialFactor;
    }

    // Emit the tube triangle vertices
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        gl_Position = mvpMatrix * vec4(circlePointsCurrent[i], 1.0);
        fragmentNormal = vertexNormalsCurrent[i];
        fragmentTangent = tangent;
        fragmentPositonWorld = (mMatrix * vec4(circlePointsCurrent[i], 1.0)).xyz;
        screenSpacePosition = (vMatrix * mMatrix * vec4(circlePointsCurrent[i], 1.0)).xyz;
        fragmentAttribute = v_in[0].lineAttribute;
        EmitVertex();

        gl_Position = mvpMatrix * vec4(circlePointsCurrent[(i+1)%NUM_SEGMENTS], 1.0);
        fragmentNormal = vertexNormalsCurrent[(i+1)%NUM_SEGMENTS];
        fragmentPositonWorld = (mMatrix * vec4(circlePointsCurrent[(i+1)%NUM_SEGMENTS], 1.0)).xyz;
        screenSpacePosition = (vMatrix * mMatrix * vec4(circlePointsCurrent[(i+1)%NUM_SEGMENTS], 1.0)).xyz;
        fragmentTangent = tangent;
        fragmentAttribute = v_in[0].lineAttribute;
        EmitVertex();

        gl_Position = mvpMatrix * vec4(circlePointsNext[i], 1.0);
        fragmentNormal = vertexNormalsNext[i];
        fragmentPositonWorld = (mMatrix * vec4(circlePointsNext[i], 1.0)).xyz;
        screenSpacePosition = (vMatrix * mMatrix * vec4(circlePointsNext[i], 1.0)).xyz;
        fragmentTangent = tangent;
        fragmentAttribute = v_in[1].lineAttribute;
        EmitVertex();

        gl_Position = mvpMatrix * vec4(circlePointsNext[(i+1)%NUM_SEGMENTS], 1.0);
        fragmentNormal = vertexNormalsNext[(i+1)%NUM_SEGMENTS];
        fragmentPositonWorld = (mMatrix * vec4(circlePointsNext[(i+1)%NUM_SEGMENTS], 1.0)).xyz;
        screenSpacePosition = (vMatrix * mMatrix * vec4(circlePointsNext[(i+1)%NUM_SEGMENTS], 1.0)).xyz;
        fragmentAttribute = v_in[1].lineAttribute;
        fragmentTangent = tangent;
        EmitVertex();

        EndPrimitive();
    }
}


-- Fragment

#version 430 core

in vec3 screenSpacePosition;

#if !defined(DIRECT_BLIT_GATHER) || defined(SHADOW_MAPPING_MOMENTS_GENERATE)
#include OIT_GATHER_HEADER
#endif

in vec3 fragmentNormal;
in vec3 fragmentTangent;
in vec3 fragmentPositonWorld;
in float fragmentAttribute;
in float lineCurvature;
in float lineLength;

#ifdef DIRECT_BLIT_GATHER
out vec4 fragColor;
#endif


#include "AmbientOcclusion.glsl"
#include "Shadows.glsl"

uniform vec3 lightDirection = vec3(1.0,0.0,0.0);
uniform vec3 cameraPosition; // world space
uniform float aoFactorGlobal = 1.0f;
uniform float shadowFactorGlobal = 1.0f;


uniform float minCriterionValue = 0.0;
uniform float maxCriterionValue = 1.0;
uniform bool transparencyMapping = true;

// Color of the object
uniform vec4 colorGlobal;


// Transfer function color lookup table
uniform sampler1D transferFunctionTexture;

vec4 transferFunction(float attr)
{
    // Transfer to range [0,1]
    float posFloat = clamp((attr - minCriterionValue) / (maxCriterionValue - minCriterionValue), 0.0, 1.0);
    // Look up the color value
    return texture(transferFunctionTexture, posFloat);
}

vec3 to_rgb(vec3 v)
{
    return mix(pow((v + 0.055) / 1.055, vec3(2.4)), v / 12.92, lessThanEqual(v, vec3(0.04045)));
}

//vec4 transferFunction(float attr)
//{
    /*
    <TransferFunction colorspace="sRGB" interpolation_colorspace="sRGB">
        <OpacityPoints>
            <OpacityPoint position="0" opacity="0"/>
            <OpacityPoint position="0.15955056250095367" opacity="0.0099999997764825821"/>
            <OpacityPoint position="0.25168538093566895" opacity="0.0099999997764825821"/>
            <OpacityPoint position="0.36629214882850647" opacity="0"/>
            <OpacityPoint position="0.45842695236206055" opacity="0.5402684211730957"/>
            <OpacityPoint position="0.56629210710525513" opacity="1"/>
            <OpacityPoint position="0.80674159526824951" opacity="0.20805370807647705"/>
            <OpacityPoint position="0.88988763093948364" opacity="0.51677852869033813"/>
            <OpacityPoint position="1" opacity="0.89932882785797119"/>
        </OpacityPoints>
        <ColorPoints>
            <ColorPoint position="0" r="96" g="249" b="251"/>
            <ColorPoint position="0.24402730166912079" r="96" g="249" b="251"/>
            <ColorPoint position="0.57885903120040894" r="171" g="147" b="0"/>
            <ColorPoint position="1" r="255" g="0" b="0"/>
        </ColorPoints>
    </TransferFunction>
    */

    /*vec4 r;

    if(attr < 0.0 || attr > 1.0)
        return vec4(1.0, 0.0, 0.0, 1.0);

    // Opacity
    if(attr <= 0.15955056250095367) r.a = mix(0.0, 0.0099999997764825821, (attr - 0.0) / (0.15955056250095367 - 0.0));
    else if(attr <= 0.25168538093566895) r.a = mix(0.0099999997764825821, 0.0099999997764825821, (attr - 0.15955056250095367) / (0.25168538093566895 - 0.15955056250095367));
    else if(attr <= 0.36629214882850647) r.a = mix(0.0099999997764825821, 0.0, (attr - 0.25168538093566895) / (0.36629214882850647 - 0.25168538093566895));
    else if(attr <= 0.45842695236206055) r.a = mix(0.0, 0.5402684211730957, (attr - 0.36629214882850647) / (0.45842695236206055 - 0.36629214882850647));
    else if(attr <= 0.56629210710525513) r.a = mix(0.5402684211730957, 1.0, (attr - 0.45842695236206055) / (0.56629210710525513 - 0.45842695236206055));
    else if(attr <= 0.80674159526824951) r.a = mix(1.0, 0.20805370807647705, (attr - 0.56629210710525513) / (0.80674159526824951 - 0.56629210710525513));
    else if(attr <= 0.88988763093948364) r.a = mix(0.20805370807647705, 0.51677852869033813, (attr - 0.80674159526824951) / (0.88988763093948364 - 0.80674159526824951));
    else r.a = mix(0.51677852869033813, 0.89932882785797119, (attr - 0.88988763093948364) / (1.0 - 0.88988763093948364));

    // Color
    if(attr <= 0.24402730166912079) r.rgb = mix(vec3(96.0 / 255.0, 249.0 / 255.0, 251.0 / 255.0), vec3(96.0 / 255.0, 249.0 / 255.0, 251.0 / 255.0), (attr - 0.0) / (0.24402730166912079 - 0.0));
    else if(attr <= 0.57885903120040894) r.rgb = mix(vec3(96.0 / 255.0, 249.0 / 255.0, 251.0 / 255.0), vec3(171.0 / 255.0, 147.0 / 255.0, 0.0 / 255.0), (attr - 0.24402730166912079) / (0.57885903120040894 - 0.24402730166912079));
    else r.rgb = mix(vec3(171.0 / 255.0, 147.0 / 255.0, 0.0 / 255.0), vec3(255.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0), (attr - 0.57885903120040894) / (1.0 - 0.57885903120040894));

    return vec4(to_rgb(r.rgb), r.a);
}*/

void main()
{
#if !defined(SHADOW_MAP_GENERATE) && !defined(SHADOW_MAPPING_MOMENTS_GENERATE)
    float occlusionFactor = getAmbientOcclusionFactor(vec4(fragmentPositonWorld, 1.0));
    float shadowFactor = getShadowFactor(vec4(fragmentPositonWorld, 1.0));
    occlusionFactor = mix(1.0f, occlusionFactor, aoFactorGlobal);
    shadowFactor = mix(1.0f, shadowFactor, shadowFactorGlobal);
#else
    float occlusionFactor = 1.0f;
    float shadowFactor = 1.0f;
#endif

    vec4 colorAttribute = transferFunction(fragmentAttribute);
    vec3 normal = normalize(fragmentNormal);

    #if REFLECTION_MODEL == 0 // PSEUDO_PHONG_LIGHTING
    const vec3 lightColor = vec3(1,1,1);
    const vec3 ambientColor = colorAttribute.rgb;
    const vec3 diffuseColor = colorAttribute.rgb;

    const float kA = 0.2 * occlusionFactor * shadowFactor;
    const vec3 Ia = kA * ambientColor;
    const float kD = 0.7;
    const float kS = 0.1;
    const float s = 10;

    const vec3 n = normalize(fragmentNormal);
    const vec3 v = normalize(cameraPosition - fragmentPositonWorld);
//    const vec3 l = normalize(lightDirection);
    const vec3 l = normalize(v);
    const vec3 h = normalize(v + l);

    #ifdef CONVECTION_ROLLS
    vec3 t = normalize(cross(vec3(0, 0, 1), n));
    #else
    vec3 t = normalize(cross(vec3(0, 0, 1), n));
    #endif

    vec3 Id = kD * clamp(abs(dot(n, l)), 0.0, 1.0) * diffuseColor;
    vec3 Is = kS * pow(clamp(abs(dot(n, h)), 0.0, 1.0), s) * lightColor;

    float haloParameter = 1;
    float angle1 = abs( dot( v, n));
    float angle2 = abs( dot( v, normalize(t))) * 0.7;
    float halo = mix(1.0f,((angle1)+(angle2)) , haloParameter);

    vec3 colorShading = Ia + Id + Is;
    colorShading *= clamp(halo, 0, 1) * clamp(halo, 0, 1);
    #elif REFLECTION_MODEL == 1 // COMBINED_SHADOW_MAP_AND_AO
    vec3 colorShading = vec3(occlusionFactor * shadowFactor);
    #elif REFLECTION_MODEL == 2 // LOCAL_SHADOW_MAP_OCCLUSION
    vec3 colorShading = vec3(shadowFactor);
    #elif REFLECTION_MODEL == 3 // AMBIENT_OCCLUSION_FACTOR
    vec3 colorShading = vec3(occlusionFactor);
    #elif REFLECTION_MODEL == 4 // NO_LIGHTING
    vec3 colorShading = colorAttribute.rgb;
    #endif
    vec4 color = vec4(colorShading, colorAttribute.a);

    if (!transparencyMapping) {
        color.a = colorGlobal.a;
    }

    if (color.a < 1.0/255.0) {
        discard;
    }

#ifdef DIRECT_BLIT_GATHER
    // Direct rendering
    fragColor = color;
#else
#if defined(REQUIRE_INVOCATION_INTERLOCK) && !defined(TEST_NO_INVOCATION_INTERLOCK)
    // Area of mutual exclusion for fragments mapping to the same pixel
    beginInvocationInterlockARB();
    gatherFragment(color);
    endInvocationInterlockARB();
#else
    gatherFragment(color);
#endif
#endif
}
