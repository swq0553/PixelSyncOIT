//
// Created by christoph on 02.09.18.
//

#ifndef PIXELSYNCOIT_OIT_AT_HPP
#define PIXELSYNCOIT_OIT_AT_HPP

#include "OIT_Renderer.hpp"

#define AT_NUM_FRAGMENTS 8

// A fragment node stores rendering information about a list of fragments
struct ATFragmentNode_compressed
{
    // Linear depth, i.e. distance to viewer
    float depth[AT_NUM_FRAGMENTS];
    // RGB color (3 bytes), translucency (1 byte)
    uint premulColor[AT_NUM_FRAGMENTS];
};

/**
 * An order independent transparency renderer using pixel sync.
 *
 * (To be precise, it doesn't use the Intel-specific Pixel Sync extension
 * INTEL_fragment_shader_ordering, but the vendor-independent ARB_fragment_shader_interlock).
 */
class OIT_AT : public OIT_Renderer {
public:
    /**
     *  The gather shader is used to render our transparent objects.
     *  Its purpose is to store the fragments in an offscreen-buffer.
     */
    virtual sgl::ShaderProgramPtr getGatherShader() { return gatherShader; }

    OIT_AT();
    virtual void create();
    virtual void resolutionChanged();

    virtual void gatherBegin();
    // In between "gatherBegin" and "gatherEnd", we can render our objects using the gather shader
    virtual void gatherEnd();

    // Blit accumulated transparent objects to screen
    virtual void renderToScreen();

private:
    void clear();

    sgl::ShaderProgramPtr gatherShader;
    sgl::ShaderProgramPtr blitShader;
    sgl::ShaderProgramPtr clearShader;
    sgl::GeometryBufferPtr fragmentNodes;
    sgl::GeometryBufferPtr numFragmentsBuffer;

    // Blit data (ignores model-view-projection matrix and uses normalized device coordinates)
    sgl::ShaderAttributesPtr blitRenderData;
    sgl::ShaderAttributesPtr clearRenderData;

    bool clearBitSet;
};

#endif //PIXELSYNCOIT_OIT_AT_HPP