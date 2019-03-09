//
// Created by christoph on 27.09.18.
//

#include "../OIT/OIT_LinkedList.hpp"
#include "InternalState.hpp"

void getTestModesNoOIT(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_DUMMY;
    state.name = "No OIT";
    states.push_back(state);
}

void getTestModesMBOIT(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_MBOIT;

    for (int useTrigMoments = 0; useTrigMoments <= 1; useTrigMoments++) {
        for (int unorm = 0; unorm <= 1; unorm++) {
            for (int numMoments = 4; numMoments <= 8; numMoments += 2) {
                state.name = std::string() + "MBOIT " + sgl::toString(numMoments)
                             + (useTrigMoments ? " Trigonometric Moments " : " Power Moments ")
                             + (unorm == 0 ? "Float" : "UNORM");
                state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
                        { "usePowerMoments", (useTrigMoments ? "false" : "true") },
                        { "numMoments", sgl::toString(numMoments) },
                        { "pixelFormat", (unorm == 0 ? "Float" : "UNORM") },
                });
                states.push_back(state);
            }
        }
    }

    state.name = std::string() + "MBOIT 4 Power Moments Float beta 0.1";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
            { "overestimationBeta", "0.1" },
            { "usePowerMoments", "true" },
            { "numMoments", sgl::toString(4) },
            { "pixelFormat", "Float" },
    });
    states.push_back(state);

    state.name = std::string() + "MBOIT 4 Power Moments Float beta 0.25";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
            { "overestimationBeta", "0.25" },
            { "usePowerMoments", "true" },
            { "numMoments", sgl::toString(4) },
            { "pixelFormat", "Float" },
    });
    states.push_back(state);
}

void getTestModesMLAB(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_MLAB;

    for (int numLayers = 1; numLayers <= 32; numLayers *= 2) {
        state.name = std::string() + "MLAB " + sgl::toString(numLayers) + " Layers";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numLayers", sgl::toString(numLayers) },
        });
        states.push_back(state);
    }
}

void getTestModesHT(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_HT;

    for (int numLayers = 1; numLayers <= 32; numLayers *= 2) {
        state.name = std::string() + "HT " + sgl::toString(numLayers) + " Layers";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numLayers", sgl::toString(numLayers) },
        });
        states.push_back(state);
    }
}

void getTestModesKBuffer(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_KBUFFER;

    for (int numLayers = 1; numLayers <= 32; numLayers *= 2) {
        state.name = std::string() + "K-Buffer " + sgl::toString(numLayers) + " Layers";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numLayers", sgl::toString(numLayers) },
        });
        states.push_back(state);
    }
}

#ifndef IM_ARRAYSIZE
#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#endif

void getTestModesLinkedListAll(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_LINKED_LIST;

    // sortingModeStrings
    for (int expectedDepthComplexity = 64; expectedDepthComplexity <= 64; expectedDepthComplexity *= 2) {
        for (int sortingModeIdx = 0; sortingModeIdx < IM_ARRAYSIZE(sortingModeStrings); sortingModeIdx++) {
            for (int maxNumFragmentsSorting = 256; maxNumFragmentsSorting <= 256; maxNumFragmentsSorting *= 2) {
                std::string sortingMode = sortingModeStrings[sortingModeIdx];
                state.name = std::string() + "Linked List " + sortingMode + + " "
                             + sgl::toString(maxNumFragmentsSorting) + " Layers, "
                             + sgl::toString(expectedDepthComplexity) + " Nodes per Pixel";
                state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                        { "sortingMode", sortingMode },
                        { "maxNumFragmentsSorting", sgl::toString(maxNumFragmentsSorting) },
                        { "expectedDepthComplexity", sgl::toString(expectedDepthComplexity) },
                });
                states.push_back(state);
            }
        }
    }
}

void getTestModesLinkedList(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_LINKED_LIST;

    int maxNumFragmentsSorting = 256;
    int expectedDepthComplexity = 64;
    if (state.modelName == "Turbulence") {
        // Highest depth complexity measured for this dataset
        expectedDepthComplexity = 104;
    }

    for (int sortingModeIdx = 0; sortingModeIdx < IM_ARRAYSIZE(sortingModeStrings); sortingModeIdx++) {
        std::string sortingMode = sortingModeStrings[sortingModeIdx];
        state.name = std::string() + "Linked List " + sortingMode + + " "
                     + sgl::toString(maxNumFragmentsSorting) + " Layers, "
                     + sgl::toString(expectedDepthComplexity) + " Nodes per Pixel";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "sortingMode", sortingMode },
                { "maxNumFragmentsSorting", sgl::toString(maxNumFragmentsSorting) },
                { "expectedDepthComplexity", sgl::toString(expectedDepthComplexity) },
        });
        states.push_back(state);
    }
}

void getTestModesDepthPeeling(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_DEPTH_PEELING;
    state.name = std::string() + "Depth Peeling";
    states.push_back(state);
}


// Test: No atomic operations, no pixel sync. Performance difference?
void getTestModesNoSync(std::vector<InternalState> &states, InternalState state)
{
    state.testNoInvocationInterlock = true;
    state.testNoAtomicOperations = true;

    state.oitAlgorithm = RENDER_MODE_OIT_MLAB;
    for (int numLayers = 1; numLayers <= 32; numLayers *= 2) {
        state.name = std::string() + "MLAB " + sgl::toString(numLayers) + " Layers, No Sync";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numLayers", sgl::toString(numLayers) },
        });
        states.push_back(state);
    }


    state.oitAlgorithm = RENDER_MODE_OIT_MBOIT;
    state.name = std::string() + "MBOIT " + sgl::toString(4) + " Power Moments Float, No Sync";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
            { "usePowerMoments", "true" },
            { "numMoments", sgl::toString(4) },
            { "pixelFormat", "Float" },
    });
    states.push_back(state);


    /*state.oitAlgorithm = RENDER_MODE_OIT_LINKED_LIST;
    state.name = std::string() + "Linked List Priority Queue "
                 + sgl::toString(1024) + " Layers, "
                 + sgl::toString(32) + " Nodes per Pixel, No Atomic Operations";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "sortingMode", "Priority Queue" },
            { "maxNumFragmentsSorting", sgl::toString(1024) },
            { "expectedDepthComplexity", sgl::toString(32) },
    });
    states.push_back(state);*/
}


// Test: Pixel sync using
void getTestModesOrderedSync(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_MLAB;
    for (int numLayers = 1; numLayers <= 32; numLayers *= 2) {
        state.name = std::string() + "MLAB " + sgl::toString(numLayers) + " Layers (Ordered)";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numLayers", sgl::toString(numLayers) },
        });
        states.push_back(state);
    }


    state.oitAlgorithm = RENDER_MODE_OIT_MBOIT;
    state.name = std::string() + "MBOIT " + sgl::toString(4) + " Power Moments Float (Ordered)";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
            { "usePowerMoments", "true" },
            { "numMoments", sgl::toString(4) },
            { "pixelFormat", "Float" },
    });
    states.push_back(state);
}


// Test: Different tiling modes for different algorithms
void getTestModesTiling(std::vector<InternalState> &states, InternalState state)
{
    std::string tilingString = std::string() + sgl::toString(state.tilingWidth)
            + "x" + sgl::toString(state.tilingHeight);
    if (state.useMortonCodeForTiling) {
        tilingString += " Morton";
    }

    state.oitAlgorithm = RENDER_MODE_OIT_MLAB;
    state.name = std::string() + "MLAB " + sgl::toString(8) + " Layers, Tiling " + tilingString;
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "numLayers", sgl::toString(8) },
    });
    states.push_back(state);


    /*state.oitAlgorithm = RENDER_MODE_OIT_MBOIT;
    state.name = std::string() + "MBOIT " + sgl::toString(4) + " Power Moments Float, Tiling " + tilingString;
    state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
            { "usePowerMoments", "true" },
            { "numMoments", sgl::toString(4) },
            { "pixelFormat", "Float" },
    });
    states.push_back(state);*/
}


// Quality test: Shuffle geometry randomly
void getTestModesShuffleGeometry(std::vector<InternalState> &states, InternalState state, int runNumber)
{
    state.oitAlgorithm = RENDER_MODE_OIT_MLAB;
    state.name = std::string() + "MLAB " + sgl::toString(8) + " Layers, Shuffled " + sgl::toString(runNumber);
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "numLayers", sgl::toString(8) },
    });
    states.push_back(state);


    state.oitAlgorithm = RENDER_MODE_OIT_MBOIT;
    state.name = std::string() + "MBOIT " + sgl::toString(4) + " Power Moments Float, Shuffled " + sgl::toString(runNumber);
    state.oitAlgorithmSettings.set(std::map<std::string, std::string> {
            { "usePowerMoments", "true" },
            { "numMoments", sgl::toString(4) },
            { "pixelFormat", "Float" },
    });
    states.push_back(state);


    state.oitAlgorithm = RENDER_MODE_OIT_HT;
    state.name = std::string() + "HT " + sgl::toString(8) + " Layers, Shuffled " + sgl::toString(runNumber);
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "numLayers", sgl::toString(8) },
    });
    states.push_back(state);


    state.oitAlgorithm = RENDER_MODE_OIT_KBUFFER;
    state.name = std::string() + "K-Buffer " + sgl::toString(8) + " Layers, Shuffled " + sgl::toString(runNumber);
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "numLayers", sgl::toString(8) },
    });
    states.push_back(state);


    state.oitAlgorithm = RENDER_MODE_OIT_LINKED_LIST;
    state.name = std::string() + "Linked List Priority Queue "
                 + sgl::toString(1024) + " Layers, "
                 + sgl::toString(32) + " Nodes per Pixel, Shuffled " + sgl::toString(runNumber);
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "sortingMode", "Priority Queue" },
            { "maxNumFragmentsSorting", sgl::toString(512) },
            { "expectedDepthComplexity", sgl::toString(32) },
    });
    states.push_back(state);
}


// Performance test: Pixel sync vs. atomic operations
void getTestModesPixelSyncVsAtomicOps(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_TEST_PIXEL_SYNC_PERFORMANCE;
    state.name = std::string() + "Pixel Sync Performance Test Compute (Unordered)";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "testMode", "usePixelSync" },
            { "testType", "compute" },
    });
    states.push_back(state);
    state.testPixelSyncOrdered = true;
    state.name = std::string() + "Pixel Sync Performance Test Compute (Ordered)";
    states.push_back(state);
    state.testPixelSyncOrdered = false;


    state.name = std::string() + "Atomic Operations Performance Test Compute";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "testMode", "useAtomicOps" },
            { "testType", "compute" },
    });
    states.push_back(state);

    state.name = std::string() + "No Synchronization Performance Test Compute";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "testMode", "noSync" },
            { "testType", "compute" },
    });
    states.push_back(state);


    state.testPixelSyncOrdered = true;
    state.name = std::string() + "Pixel Sync Performance Test Sum (Unordered)";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "testMode", "usePixelSync" },
            { "testType", "sum" },
    });
    states.push_back(state);
    state.testPixelSyncOrdered = true;
    state.name = std::string() + "Pixel Sync Performance Test Sum (Ordered)";
    states.push_back(state);
    state.testPixelSyncOrdered = false;

    state.name = std::string() + "Atomic Operations Performance Test Sum";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "testMode", "useAtomicOps" },
            { "testType", "sum" },
    });
    states.push_back(state);

    state.name = std::string() + "No Synchronization Performance Test Sum";
    state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
            { "testMode", "noSync" },
            { "testType", "sum" },
    });
    states.push_back(state);
}

void getTestModesMLABBucketsAll(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_MLAB_BUCKET;

    for (int bucketMode = 0; bucketMode < 5; bucketMode++) {
        state.name = std::string() + "MLAB Buckets 4x4 Mode " + sgl::toString(bucketMode) + "";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numBuckets", sgl::toString(4) },
                { "nodesPerBucket", sgl::toString(4) },
                { "bucketMode", sgl::toString(bucketMode) },
        });
        states.push_back(state);
    }

    for (int nodesPerBucket = 2; nodesPerBucket <= 8; nodesPerBucket *= 2) {
        state.name = std::string() + "MLAB Min Depth Buckets " + sgl::toString(nodesPerBucket) + " Layers";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numBuckets", sgl::toString(1) },
                { "nodesPerBucket", sgl::toString(nodesPerBucket) },
                { "bucketMode", sgl::toString(4) },
        });
        states.push_back(state);
    }
}


void getTestModesMLABBuckets(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_OIT_MLAB_BUCKET;

    for (int nodesPerBucket = 2; nodesPerBucket <= 8; nodesPerBucket *= 2) {
        state.name = std::string() + "MLAB Min Depth Buckets " + sgl::toString(nodesPerBucket) + " Layers";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "numBuckets", sgl::toString(1) },
                { "nodesPerBucket", sgl::toString(nodesPerBucket) },
                { "bucketMode", sgl::toString(4) },
        });
        states.push_back(state);
    }
}


void getTestModesVoxelRaytracing(std::vector<InternalState> &states, InternalState state)
{
    state.oitAlgorithm = RENDER_MODE_VOXEL_RAYTRACING_LINES;

    for (int gridResolution = 128; gridResolution <= 128; gridResolution *= 2) {
        state.name = std::string() + "Voxel Ray Casting (Grid " + sgl::toString(gridResolution) + ", Quantization 64, Neighbor Search)";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "gridResolution", sgl::toString(gridResolution) },
                { "quantizationResolution", sgl::toString(64) },
                { "useNeighborSearch", "true" },
        });
        states.push_back(state);
        state.name = std::string() + "Voxel Ray Casting (Grid " + sgl::toString(gridResolution) + ", Quantization 64, No Neighbor Search)";
        state.oitAlgorithmSettings.set(std::map<std::string, std::string>{
                { "gridResolution", sgl::toString(gridResolution) },
                { "quantizationResolution", sgl::toString(64) },
                { "useNeighborSearch", "false" },
        });
        states.push_back(state);
    }
}


void getTestModesPaperForMesh(std::vector<InternalState> &states, InternalState state)
{
    getTestModesDepthPeeling(states, state);
    getTestModesNoOIT(states, state);
    //getTestModesMLAB(states, state);
    //getTestModesMBOIT(states, state);
    //getTestModesLinkedList(states, state);
    getTestModesMLABBuckets(states, state);
    getTestModesVoxelRaytracing(states, state);
}

std::vector<InternalState> getTestModesPaper()
{
    std::vector<InternalState> states;
    std::vector<std::string> modelNames = {"Aneurism Streamlines", "Ponytail"}; // , "Turbulence"
    InternalState state;

    for (size_t i = 0; i < modelNames.size(); i++) {
        state.modelName = modelNames.at(i);
        getTestModesPaperForMesh(states, state);
    }

    // Append model name to state name if more than one model is loaded
    if (modelNames.size() > 1) {
        for (InternalState &state : states) {
            state.name = state.modelName + " " + state.name;
        }
    }

    return states;
}

std::vector<InternalState> getAllTestModes()
{
    std::vector<InternalState> states;
    InternalState state;
    //state.modelName = "Monkey";
    //state.modelName = "Streamlines";
    state.modelName = "Aneurism Streamlines";

    getTestModesDepthPeeling(states, state);
    getTestModesNoOIT(states, state);
    getTestModesMLAB(states, state);
    getTestModesMBOIT(states, state);
    getTestModesHT(states, state);
    getTestModesKBuffer(states, state);
    getTestModesLinkedList(states, state);

    // Performance test: Different values for tiling
    InternalState stateTiling = state;
    stateTiling.tilingWidth = 1;
    stateTiling.tilingHeight = 1;
    getTestModesTiling(states, stateTiling);
    stateTiling.tilingWidth = 2;
    stateTiling.tilingHeight = 2;
    getTestModesTiling(states, stateTiling);
    stateTiling.tilingWidth = 2;
    stateTiling.tilingHeight = 8;
    getTestModesTiling(states, stateTiling);
    stateTiling.tilingWidth = 8;
    stateTiling.tilingHeight = 2;
    getTestModesTiling(states, stateTiling);
    stateTiling.tilingWidth = 4;
    stateTiling.tilingHeight = 4;
    getTestModesTiling(states, stateTiling);
    stateTiling.tilingWidth = 8;
    stateTiling.tilingHeight = 8;
    getTestModesTiling(states, stateTiling);
    stateTiling.tilingWidth = 8;
    stateTiling.tilingHeight = 8;
    stateTiling.useMortonCodeForTiling = true;
    getTestModesTiling(states, stateTiling);

    // Performance test: No synchronization operations
    InternalState stateNoSync = state;
    stateNoSync.testNoInvocationInterlock = true;
    stateNoSync.testNoAtomicOperations = true;
    getTestModesNoSync(states, stateNoSync);

    // Performance test: Ordered pixel sync
    InternalState stateOrderedSync = state;
    stateOrderedSync.testPixelSyncOrdered = true;
    getTestModesOrderedSync(states, stateOrderedSync);

    // Quality test: Shuffle geometry randomly
    if (state.modelName == "Aneurism (Lines)") {
        InternalState stateShuffleGeometry = state;
        stateShuffleGeometry.testShuffleGeometry = true;
        getTestModesShuffleGeometry(states, stateShuffleGeometry, 1);
        getTestModesShuffleGeometry(states, stateShuffleGeometry, 2);
    }

    // Performance test: Pixel sync vs. atomic operations
    getTestModesPixelSyncVsAtomicOps(states, state);

    // MLAB with buckets
    getTestModesMLABBuckets(states, state);

    // Voxel ray casting
    getTestModesVoxelRaytracing(states, state);

    return states;
}