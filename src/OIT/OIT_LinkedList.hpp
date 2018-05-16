/*
 * OIT_LinkedList.hpp
 *
 *  Created on: 14.05.2018
 *      Author: christoph
 */

#ifndef OIT_OIT_LINKEDLIST_HPP_
#define OIT_OIT_LINKEDLIST_HPP_

#include "OIT_Renderer.hpp"

/**
 * An order independent transparency renderer using a per-pixel linked list.
 * The algorithm uses two read-write buffers (SSBOs in OpenGL, UAVs in DirectX).
 * - The fragment-and-link buffer storing all rendered fragments in a linked list.
 * - The start-offset buffer storing the index of the first fragment in the buffer
 *   mentioned above for each pixel.
 */

/*const int linkedListEntriesPerPixel = 8;

class OIT_LinkedList : public OIT_Renderer {
public:*/
	/**
	 *  The gather shader is used to render our transparent objects.
	 *  Its purpose is to store the fragments in an offscreen-buffer.
	 */
	/*virtual sgl::ShaderProgramPtr getGatherShader() { return gatherShader; }

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
	sgl::GeometryBufferPtr fragmentNodes;

	// Blits data (ignores model-view-projection matrix and uses normalized device coordinates)
	sgl::ShaderAttributesPtr blitRenderData;
};*/

#endif /* OIT_OIT_LINKEDLIST_HPP_ */