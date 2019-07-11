/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2019, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PIXELSYNCOIT_RTRENDERBACKEND_HPP
#define PIXELSYNCOIT_RTRENDERBACKEND_HPP

#include "../TransferFunctionWindow.hpp"
#include "../Utils/ImportanceCriteria.hpp"
#include "../Utils/TrajectoryFile.hpp"

#include "ospray/ospray.h"
#include "ospray/ospcommon/vec.h"
#include <ospray/ospcommon/box.h>

struct Node
{
    ospcommon::vec3f position;
    float radius;
};

struct Link
{
    int first;
    int second;
};

struct TubePrimitives
{
    std::vector<Node> nodes;
    std::vector<Link> links;
    std::vector<ospcommon::vec4f> colors;
    ospcommon::box3f worldBounds;
};

class RTRenderBackend {
public:
    /**
     * Sets the viewport size to use for rendering.
     * @param width The width of the viewport in pixels.
     * @param height The height of the viewport in pixels.
     */
    void setViewportSize(int width, int height);

    /**
     * This function loads trajectories (i.e., line datasets) and converts them to an internal representation.
     * (The internal representation could in theory also be cached in a file.)
     * @param filename The filename of the trajectory dataset.
     * @param trajectories The trajectories to load.
     */
    void loadTubePrimitives(const std::string &filename);

    /**
     * The data is stored as a list of triangles, i.e., the vertices referenced by three consecutive indices form one
     * triangle.
     * @param filename The filename of the triangle mesh (e.g. necessary when caching the internal representation).
     * @param indices The triangle indices.
     * @param vertices The indexed triangle vertices.
     * @param vertexNormals The vertex normals.
     * @param vertexAttributes The per-vertex attributes (mapped to color and opacity using the transfer function).
     */
    void loadTriangleMesh(
            const std::string &filename,
            const std::vector<uint32_t> &indices,
            const std::vector<glm::vec3> &vertices,
            const std::vector<glm::vec3> &vertexNormals,
            const std::vector<float> &vertexAttributes);

    /**
     * For mapping line attributes (i.e., importance criteria) to colors and opacities, transfer functions are used.
     * 'OpacityPoint' and 'ColorPoint_sRGB' store an attribute value (called 'position') together with the corresponding
     * opacity/color. 'position' is guaranteed to be a value between zero and one
     * Between these control points, the opacity and color are linearly interpolated.
     * The program specifies colors in sRGB and also interpolates between them is sRGB space.
     * However, the final interpolated color is then converted to linear RGB for rendering.
     * @param opacityPoints The control points of the opacity.
     * @param colorPoints_sRGB The control points of the color.
     *
     * TODO: If you rather want an evenly-spaced look-up table of e.g. 256 colors, I could change the interface
     * (Christoph 07/04).
     */
    void setTransferFunction(
            const std::vector<OpacityPoint> &opacityPoints, const std::vector<ColorPoint_sRGB> &colorPoints_sRGB);

    /**
     * Sets the line radius to use for rendering. This can be ignored when rendering a triangle mesh instead of
     * directly rendering lines.
     */
    void setLineRadius(float lineRadius);

    /**
     * Renders the scene to an image in RGBA32 format.
     * @param backgroundColor The background color of the scene.
     * @param pos The position of the camera.
     * @param dir The view direction of the camera.
     * @param up The up vector of the camera.
     * @param fovy The vertical field of view of the camera.
     * @return A pointer to the image the scene was rendered to. It is expected that the image data is in the RGBA32
     * format and stores sRGB data.
     */
    uint32_t *renderToImage(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up, const float fovy);

private:
    // Viewport width and height.
    int width, height;

    // Constant line radius to use for rendering (ignore for triangle meshes).
    float lineRadius;

    // The image data.
    std::vector<uint32_t> image;

    // hold the data 
    TubePrimitives Tube;
};


#endif //PIXELSYNCOIT_RTRENDERBACKEND_HPP
