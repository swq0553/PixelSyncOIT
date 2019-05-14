-- Compute

#version 430

layout (local_size_x = 64, local_size_y = 4, local_size_z = 1) in;

struct LineSegment
{
    vec3 v1; // Vertex position
    float a1; // Vertex attribute
    vec3 v2; // Vertex position
    float a2; // Vertex attribute
    uint lineID;
};

struct LineSegmentQuantized
{
    uint faceIndex1; // 3 bits
    uint faceIndex2; // 3 bits
    uint facePositionQuantized1; // 2*log2(QUANTIZATION_RESOLUTION) bits
    uint facePositionQuantized2; // 2*log2(QUANTIZATION_RESOLUTION) bits
    uint lineID; // 8 bits
    float a1; // Attribute 1
    float a2; // Attribute 2
};

// Works until quantization resolution of 64^2 (6 + 2 * 2log2(64) = 30)
struct LineSegmentCompressed
{
    // Bit 0-2, 3-5: Face ID of start/end point.
    // For c = log2(QUANTIZATION_RESOLUTION^2) = 2*log2(QUANTIZATION_RESOLUTION):
    // Bit 6-(5+c), (6+c)-(5+2c): Quantized face position of start/end point.
    uint linePosition;
    // Bit 11-15: Line ID (5 bits for bitmask of 2^5 bits = 32 bits).
    // Bit 16-23, 24-31: Attribute of start/end point (normalized to [0,1]).
    uint attributes;
};


struct LinePoint {
    vec3 linePoint;
    float lineAttribute;
};

layout (std430, binding = 2) buffer LinePointBuffer
{
    LinePoint linePoints[];
};

layout (std430, binding = 3) buffer LineOffsetBuffer
{
    uint lineOffsets[];
};

layout (std430, binding = 4) buffer NumSegmentsBuffer
{
    uint numSegments[];
};

// MAX_NUM_LINES_PER_VOXEL * "voxel grid size"
layout (std430, binding = 5) buffer LineSegmentsBuffer
{
    LineSegmentCompressed lineSegments[];
};

uint getVoxelIndex1D(ivec3 voxelIndex)
{
    return voxelIndex.x + voxelIndex.y*gridResolution.x + voxelIndex.z*gridResolution.x*gridResolution.y;
}

uniform uint numLines;



#define BIAS 0.001

/**
 * Helper function for rayBoxIntersection (see below).
 */
bool rayBoxPlaneIntersection(float rayOriginX, float rayDirectionX, float lowerX, float upperX,
        inout float tNear, inout float tFar)
{
    if (abs(rayDirectionX) < BIAS) {
        // Ray is parallel to the x planes
        if (rayOriginX < lowerX || rayOriginX > upperX) {
            return false;
        }
    } else {
        // Not parallel to the x planes. Compute the intersection distance to the planes.
        float t0 = (lowerX - rayOriginX) / rayDirectionX;
        float t1 = (upperX - rayOriginX) / rayDirectionX;
        if (t0 > t1) {
            // Since t0 intersection with near plane
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        if (t0 > tNear) {
            // We want the largest tNear
            tNear = t0;
        }
        if (t1 < tFar) {
            // We want the smallest tFar
            tFar = t1;
        }
        if (tNear > tFar) {
            // Box is missed
            return false;
        }
        if (tFar < 0.0) {
            // Box is behind ray
            return false;
        }
    }
    return true;
}

/**
 * Implementation of ray-box intersection (idea from A. Glassner et al., "An Introduction to Ray Tracing").
 * For more details see: https://www.siggraph.org//education/materials/HyperGraph/raytrace/rtinter3.htm
 */
int rayBoxIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 lower, vec3 upper,
        inout float tNear, inout float tFar)
{
    tNear = -1e7;
    tFar = 1e7;
    for (int i = 0; i < 3; i++) {
        if (!rayBoxPlaneIntersection(rayOrigin[i], rayDirection[i], lower[i], upper[i], tNear, tFar)) {
            return 0;
        }
    }

    //entrancePoint = rayOrigin + tNear * rayDirection;
    //exitPoint = rayOrigin + tFar * rayDirection;
    return (tNear >= 0.0f && tNear <= 1.0f ? 1 : 0) + (tFar >= 0.0f && tNear <= 1.0f ? 1 : 0);
}

void quantizePoint(vec3 v, out ivec2 qv, int faceIndex)
{
    int dimensions[2];
    if (faceIndex == 0 || faceIndex == 1) {
        // x face
        dimensions[0] = 1;
        dimensions[1] = 2;
    } else if (faceIndex == 2 || faceIndex == 3) {
        // y face
        dimensions[0] = 0;
        dimensions[1] = 2;
    } else {
        // z face
        dimensions[0] = 0;
        dimensions[1] = 1;
    }

    // Iterate over all dimensions
    for (int i = 0; i < 2; i++) {
        int quantizationPos = floor(v[dimensions[i]] * quantizationResolution[dimensions[i]]);
        qv[i] = clamp(quantizationPos, 0, quantizationResolution[dimensions[i]]-1);
    }
}

int computeFaceIndex(vec3 v, ivec3 voxelIndex)
{
    ivec3 lower = voxelIndex, upper = voxelIndex + ivec3(1);
    for (int i = 0; i < 3; i++) {
        if (abs(v[i] - lower[i]) < 0.00001) {
            return 2*i;
        }
        if (abs(v[i] - upper[i]) < 0.00001) {
            return 2*i+1;
        }
    }
    return 0;
}

void quantizeLineSegment(vec3 voxelPos, LineSegment lineSegment, out LineSegmentQuantized lineQuantized,
        int faceIndex1, int faceIndex2)
{
    lineQuantized.a1 = lineSegment.a1;
    lineQuantized.a2 = lineSegment.a2;

    ivec2 facePosition3D1, facePosition3D2;
    quantizePoint(lineSegment.v1 - voxelPos, facePosition3D1, faceIndex1);
    quantizePoint(lineSegment.v2 - voxelPos, facePosition3D2, faceIndex2);
    lineQuantized.lineID = lineSegment.lineID;
    lineQuantized.faceIndex1 = faceIndex1;
    lineQuantized.faceIndex2 = faceIndex2;
    lineQuantized.facePositionQuantized1 = facePosition3D1.x + facePosition3D1.y*quantizationResolution.x;
    lineQuantized.facePositionQuantized2 = facePosition3D2.x + facePosition3D2.y*quantizationResolution.x;
}

int intlog2(int x) {
    int exponent = 0;
    while (x > 1) {
        x /= 2;
        exponent++;
    }
    return exponent;
}

void compressLineSegment(ivec3 voxelIndex, LineSegment lineSegment, out LineSegmentCompressed lineCompressed)
{
    LineSegmentQuantized lineQuantized;
    int faceIndex1 = computeFaceIndex(lineSegment.v1, voxelIndex);
    int faceIndex2 = computeFaceIndex(lineSegment.v2, voxelIndex);
    quantizeLineSegment(vec3(voxelIndex), lineSegment, lineQuantized, faceIndex1, faceIndex2);

    uint attr1Unorm = round(lineQuantized.a1*255.0f);
    uint attr2Unorm = round(lineQuantized.a2*255.0f);

    int c = round(2*intlog2(quantizationResolution.x));
    lineCompressed.linePosition = lineQuantized.faceIndex1;
    lineCompressed.linePosition |= lineQuantized.faceIndex2 << 3;
    lineCompressed.linePosition |= lineQuantized.facePositionQuantized1 << 6;
    lineCompressed.linePosition |= lineQuantized.facePositionQuantized2 << (6 + c);
    lineCompressed.attributes = 0;
    lineCompressed.attributes |= (lineQuantized.lineID & 31u) << 11;
    lineCompressed.attributes |= attr1Unorm << 16;
    lineCompressed.attributes |= attr2Unorm << 24;
}


void addLineSegment(ivec3 voxelIndex, LineSegment lineSegment)
{
    LineSegmentCompressed lineSegmentCompressed = compressLineSegment(lineSegment);

    uint voxelIndex1D = getVoxelIndex1D(voxelIndex);
    uint segmentPosition = atomicAdd(numSegments[voxelIndex1D], 1u);
    if (segmentPosition < MAX_NUM_LINES_PER_VOXEL) {
        lineSegments[voxelIndex1D*MAX_NUM_LINES_PER_VOXEL+segmentPosition] = lineSegmentCompressed;
    } else {
        atomicExchange(numSegments[voxelIndex1D], MAX_NUM_LINES_PER_VOXEL);
    }
}

/**
 * Code inspired by "A Fast Voxel Traversal Algorithm for Ray Tracing" written by John Amanatides, Andrew Woo.
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf
 * Value initialization code adapted from: https://stackoverflow.com/questions/12367071/how-do-i-initialize-the-t-
 * variables-in-a-fast-voxel-traversal-algorithm-for-ray
 *
 * Traverses the voxel grid from "startPoint" until "endPoint".
 * Calls "nextVoxel" each time a voxel is entered.
 * Returns the accumulated color using voxel raytracing.
 */
void traverseVoxelGrid(uint lineID, vec3 startPoint, float startAttribute, vec3 endPoint, float endAttribute,
        inout int currentVoxelNumIntersections, inout vec3 currentVoxelIntersection,
        inout float currentVoxelIntersectionAttribute)
{
    uvec3 startVoxel = uvec3(startPoint);
    uvec3 endVoxel = uvec3(endPoint);

    float tMaxX, tMaxY, tMaxZ, tDeltaX, tDeltaY, tDeltaZ;
    ivec3 voxelIndex;

    int stepX = int(sign(endPoint.x - startPoint.x));
    if (stepX != 0)
    tDeltaX = min(stepX / (endPoint.x - startPoint.x), 1e7);
    else
    tDeltaX = 1e7; // inf
    if (stepX > 0)
    tMaxX = tDeltaX * (1.0 - fract(startPoint.x));
    else
    tMaxX = tDeltaX * fract(startPoint.x);
    voxelIndex.x = int(startPoint.x);

    int stepY = int(sign(endPoint.y - startPoint.y));
    if (stepY != 0)
    tDeltaY = min(stepY / (endPoint.y - startPoint.y), 1e7);
    else
    tDeltaY = 1e7; // inf
    if (stepY > 0)
    tMaxY = tDeltaY * (1.0 - fract(startPoint.y));
    else
    tMaxY = tDeltaY * fract(startPoint.y);
    voxelIndex.y = int(startPoint.y);

    int stepZ = int(sign(endPoint.z - startPoint.z));
    if (stepZ != 0)
    tDeltaZ = min(stepZ / (endPoint.z - startPoint.z), 1e7);
    else
    tDeltaZ = 1e7; // inf
    if (stepZ > 0)
    tMaxZ = tDeltaZ * (1.0 - fract(startPoint.z));
    else
    tMaxZ = tDeltaZ * fract(startPoint.z);
    voxelIndex.z = int(startPoint.z);

    if (stepX == 0 && stepY == 0 && stepZ == 0) {
        return;
    }
    ivec3 step = ivec3(stepX, stepY, stepZ);
    vec3 tMax = vec3(tMaxX, tMaxY, tMaxZ);
    vec3 tDelta = vec3(tDeltaX, tDeltaY, tDeltaZ);


    vec3 rayDirection = normalize(endPoint - startPoint);
    LineSegment lineSegment;
    int numIntersectionsNew;

    while (true) {
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                voxelIndex.x += stepX;
                tMaxX += tDeltaX;
            } else {
                voxelIndex.z += stepZ;
                tMaxZ += tDeltaZ;
            }
        } else {
            if (tMaxY < tMaxZ) {
                voxelIndex.y += stepY;
                tMaxY += tDeltaY;
            } else {
                voxelIndex.z += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
        if (any(lessThan(voxelIndex, ivec3(0))) || any(greaterThanEqual(voxelIndex, gridResolution)))
            break;

        if (voxelIndex == endVoxel) {
            // Break on last voxel
            break;
        }
        if (voxelIndex == startVoxel) {
            // Skip first voxel
            continue;
        }

        float tNear, tFar;
        numIntersectionsNew = computeRayBoxIntersections(startPoint, endPoint, rayDirection, tNear, tFar);

        if (numIntersectionsNew == 2 || (numIntersectionsNew == 1 && currentVoxelNumIntersections == 1)) {
            lineSegment.lineID = lineID;
            if (numIntersectionsNew == 1) {
                lineSegment.v1 = currentVoxelIntersection;
                lineSegment.a1 = currentVoxelIntersectionAttribute;
            } else {
                lineSegment.v1 = v1 + tNear * (v2 - v1);
                lineSegment.a1 = a1 + tNear * (a2 - a1);
            }
            lineSegment.v2 = v1 + tFar * (v2 - v1);
            lineSegment.a2 = a1 + tFar * (a2 - a1);
            addLineSegment(voxelIndex, lineSegment);
            currentVoxelNumIntersections = 0;
        } else if (numIntersectionsNew == 1) {
            currentVoxelIntersection = v1 + tNear * (v2 - v1);
            currentVoxelIntersectionAttribute = a1 + tNear * (a2 - a1);
            currentVoxelNumIntersections = 1;
        }
    }
}

void main() {
    uint lineNumber = gl_GlobalInvocationID.x;
    if (lineNumber >= numLines) {
        return;
    }

    uint lineOffset = lineOffsets[lineNumber];
    uint numLinePoints = lineOffsets[lineNumber + 1] - lineOffset;

    int currentVoxelNumIntersections = 0;
    vec3 currentVoxelIntersection; // Carry-over-field across line segments
    float currentVoxelIntersectionAttribute; // Carry-over-field across line segments

    for (int i = 0; i < numLinePoints-1; i++) {
        LinePoint p1 = linePoints[lineOffset + i];
        LinePoint p2 = linePoints[lineOffset + i + 1];

        // Remove invalid line points (used in many scientific datasets to indicate invalid lines).
        const float MAX_VAL = 1e10;
        if (abs(center.x) > MAX_VAL || abs(center.y) > MAX_VAL || abs(center.z) > MAX_VAL) {
            outputLinePoints[lineOffset + i].valid = 0;
            continue;
        }

        vec3 tangent;
        if (i == 0) {
            // First node
            tangent = inputLinePoints[lineOffset + i+1].linePoint - center;
        } else if (i == numLinePoints-1) {
            // Last node
            tangent = center - inputLinePoints[lineOffset + i-1].linePoint;
        } else {
            // Node with two neighbors - use both tangents.
            tangent = inputLinePoints[lineOffset + i+1].linePoint - center;
        }
        if (length(tangent) < 0.0001) {
            // In case the two vertices are almost identical, just skip this path line segment.
            outputLinePoints[lineOffset + i].valid = 0;
            continue;
        }
        tangent = normalize(tangent);

        // DDA algorithm
        traverseVoxelGrid(lineNumber, p1.linePoint, p1.lineAttribute, p2.linePoint, p2.lineAttribute,
                currentVoxelNumIntersections, currentVoxelIntersection, currentVoxelIntersectionAttribute);
    }
}
