//
// Created by christoph on 04.10.18.
//

#include <cstring>
#include <fstream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <Utils/Events/Stream/Stream.hpp>
#include <Utils/File/Logfile.hpp>
#include <Utils/Convert.hpp>
#include <Math/Math.hpp>
#include <Graphics/Renderer.hpp>
#include <Graphics/OpenGL/Texture.hpp>

#include "VoxelData.hpp"

const uint32_t VOXEL_GRID_FORMAT_VERSION = 1u;

void saveToFile(const std::string &filename, const VoxelGridDataCompressed &data)
{
    std::ofstream file(filename.c_str(), std::ofstream::binary);
    if (!file.is_open()) {
        sgl::Logfile::get()->writeError(std::string() + "Error in saveToFile: File \"" + filename + "\" not found.");
        return;
    }

    sgl::BinaryWriteStream stream;
    stream.write((uint32_t)VOXEL_GRID_FORMAT_VERSION);
    stream.write(data.gridResolution);
    stream.write(data.quantizationResolution);
    stream.write(data.worldToVoxelGridMatrix);

    stream.writeArray(data.voxelLineListOffsets);
    stream.writeArray(data.numLinesInVoxel);
    stream.writeArray(data.voxelDensityLODs);
    stream.writeArray(data.lineSegments);

    file.write((const char*)stream.getBuffer(), stream.getSize());
    file.close();
}

void loadFromFile(const std::string &filename, VoxelGridDataCompressed &data)
{
    std::ifstream file(filename.c_str(), std::ifstream::binary);
    if (!file.is_open()) {
        sgl::Logfile::get()->writeError(std::string() + "Error in loadFromFile: File \"" + filename + "\" not found.");
        return;
    }

    file.seekg(0, file.end);
    size_t size = file.tellg();
    file.seekg(0);
    char *buffer = new char[size];
    file.read(buffer, size);

    sgl::BinaryReadStream stream(buffer, size);
    uint32_t version;
    stream.read(version);
    if (version != VOXEL_GRID_FORMAT_VERSION) {
        sgl::Logfile::get()->writeError(std::string() + "Error in loadFromFile: Invalid version in file \""
                                        + filename + "\".");
        return;
    }

    stream.read(data.gridResolution);
    stream.read(data.quantizationResolution);
    stream.read(data.worldToVoxelGridMatrix);

    stream.readArray(data.voxelLineListOffsets);
    stream.readArray(data.numLinesInVoxel);
    stream.readArray(data.voxelDensityLODs);
    stream.readArray(data.lineSegments);

    //delete[] buffer; // BinaryReadStream does deallocation
    file.close();
}


std::vector<float> generateMipmapsForDensity(float *density, glm::ivec3 size)
{
    std::vector<float> allLODs;
    size_t memorySize = 0;
    for (glm::ivec3 lodSize = size; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        memorySize += lodSize.x * lodSize.y * lodSize.z;
    }
    allLODs.reserve(memorySize);

    for (int i = 0; i < size.x * size.y * size.z; i++) {
        allLODs.push_back(density[i]);
    }

    float *lodData = new float[size.x * size.y * size.z];
    float *lodDataLast = new float[size.x * size.y * size.z];
    memcpy(lodDataLast, density, size.x * size.y * size.z * sizeof(float));
    for (glm::ivec3 lodSize = size/2; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        // Averaging operation
        for (int z = 0; z < lodSize.z; z++) {
            for (int y = 0; y < lodSize.y; y++) {
                for (int x = 0; x < lodSize.x; x++) {
                    int childIdx = z*lodSize.y*lodSize.x + y*lodSize.x + x;
                    lodData[childIdx] = 0;
                    for (int offsetZ = 0; offsetZ < 2; offsetZ++) {
                        for (int offsetY = 0; offsetY < 2; offsetY++) {
                            for (int offsetX = 0; offsetX < 2; offsetX++) {
                                int parentIdx = (z*2+offsetZ)*lodSize.y*lodSize.x*4
                                                + (y*2+offsetY)*lodSize.y*2 + x*2+offsetX;
                                lodData[childIdx] += lodDataLast[parentIdx];
                            }
                        }
                    }
                    lodData[childIdx] /= 8.0f;
                    allLODs.push_back(lodData[childIdx]);
                }
            }
        }
        float *tmp = lodData;
        lodData = lodDataLast;
        lodDataLast = tmp;
        /*int N = lodSize.x * lodSize.y * lodSize.z;
        for (int i = 0; i < N; i++) {
            lodData[i] = (lodDataLast[i*8]+lodDataLast[i*8+1]
                          + lodDataLast[i*8 + lodSize.x*2]+lodDataLast[i*8+1 + lodSize.x*2]
                          + lodDataLast[i*8 + lodSize.x*lodSize.y*4]+lodDataLast[i*8+1 + lodSize.x*lodSize.y*4]
                          + lodDataLast[i*8 + lodSize.x*2 + lodSize.x*lodSize.y*4]
                            +lodDataLast[i*8+1 + lodSize.x*2 + lodSize.x*lodSize.y*4]);
            lodData[i] /= 8.0f;
            allLODs.push_back(lodData[i]);
        }*/
    }

    delete[] lodData;
    delete[] lodDataLast;
    return allLODs;
}


std::vector<uint32_t> generateMipmapsForOctree(uint32_t *numLines, glm::ivec3 size)
{
    std::vector<uint32_t> allLODs;
    size_t memorySize = 0;
    for (glm::ivec3 lodSize = size; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        memorySize += lodSize.x * lodSize.y * lodSize.z;
    }
    allLODs.reserve(memorySize);

    for (int i = 0; i < size.x * size.y * size.z; i++) {
        allLODs.push_back(numLines[i]);
    }

    uint32_t *lodData = new uint32_t[size.x * size.y * size.z];
    uint32_t *lodDataLast = new uint32_t[size.x * size.y * size.z];
    memcpy(lodDataLast, numLines, size.x * size.y * size.z * sizeof(uint32_t));
    for (glm::ivec3 lodSize = size/2; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        // Sum operation
        for (int z = 0; z < lodSize.z; z++) {
            for (int y = 0; y < lodSize.y; y++) {
                for (int x = 0; x < lodSize.x; x++) {
                    int childIdx = z*lodSize.y*lodSize.x + y*lodSize.x + x;
                    lodData[childIdx] = 0;
                    for (int offsetZ = 0; offsetZ < 2; offsetZ++) {
                        for (int offsetY = 0; offsetY < 2; offsetY++) {
                            for (int offsetX = 0; offsetX < 2; offsetX++) {
                                int parentIdx = (z*2+offsetZ)*lodSize.y*lodSize.x*4
                                        + (y*2+offsetY)*lodSize.y*2 + x*2+offsetX;
                                lodData[childIdx] += lodDataLast[parentIdx];
                            }
                        }
                    }
                    allLODs.push_back(lodData[childIdx] > 0 ? 1 : 0);
                }
            }
        }
        uint32_t *tmp = lodData;
        lodData = lodDataLast;
        lodDataLast = tmp;
        /*int N = lodSize.x * lodSize.y * lodSize.z;
        for (int i = 0; i < N; i++) {
            lodData[i] = (lodDataLast[i*8]+lodDataLast[i*8+1]
                          + lodDataLast[i*8 + lodSize.x*2]+lodDataLast[i*8+1 + lodSize.x*2]
                          + lodDataLast[i*8 + lodSize.x*lodSize.y*4]+lodDataLast[i*8+1 + lodSize.x*lodSize.y*4]
                          + lodDataLast[i*8 + lodSize.x*2 + lodSize.x*lodSize.y*4]
                            +lodDataLast[i*8+1 + lodSize.x*2 + lodSize.x*lodSize.y*4]);
            allLODs.push_back(lodData[i]);
        }*/
    }

    delete[] lodData;
    delete[] lodDataLast;
    return allLODs;
}


sgl::TexturePtr generateDensityTexture(const std::vector<float> &lods, glm::ivec3 size)
{
    int numMipmapLevels = 0;
    for (glm::ivec3 lodSize = size; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        numMipmapLevels++;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, numMipmapLevels-1);

#ifdef USE_OPENGL_LOD_GENERATION
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, size.x, size.y, size.z, 0, GL_RED, GL_FLOAT, &lods.front());
    glGenerateMipmap(GL_TEXTURE_3D);
#else
    // Now upload the LOD levels
    int lodIndex = 0;
    const float *data = &lods.front();
    for (glm::ivec3 lodSize = size; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        glTexImage3D(GL_TEXTURE_3D, lodIndex, GL_R32F, lodSize.x, lodSize.y, lodSize.z, 0, GL_RED, GL_FLOAT, data);
        lodIndex++;
        data += lodSize.x * lodSize.y * lodSize.z;
    }
#endif

    sgl::TextureSettings textureSettings;
    textureSettings.type = sgl::TEXTURE_3D;
    return sgl::TexturePtr(new sgl::TextureGL(textureID, size.x, size.y, size.z, textureSettings));
}


sgl::TexturePtr generateOctreeTexture(const std::vector<uint32_t> &lods, glm::ivec3 size)
{
    int numMipmapLevels = 0;
    for (glm::ivec3 lodSize = size; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        numMipmapLevels++;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, numMipmapLevels-1);

#ifdef USE_OPENGL_LOD_GENERATION
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, size.x, size.y, size.z, 0, GL_RED, GL_FLOAT, &lods.front());
    glGenerateMipmap(GL_TEXTURE_3D);
#else
    // Now upload the LOD levels
    int lodIndex = 0;
    const uint32_t *data = &lods.front();
    for (glm::ivec3 lodSize = size; lodSize.x > 0 && lodSize.y > 0 && lodSize.z > 0; lodSize /= 2) {
        // TODO: GL_R8UI and only one bit indicator?
        glTexImage3D(GL_TEXTURE_3D, lodIndex, GL_R8UI, lodSize.x, lodSize.y, lodSize.z, 0, GL_RED_INTEGER,
                GL_UNSIGNED_INT, data);
        std::cout << data[0] << std::endl;
        lodIndex++;
        data += lodSize.x * lodSize.y * lodSize.z;
    }
#endif

    std::cout << "LOD: " << lodIndex << std::endl;

    /*if (lods.at(lods.size()-1) != 0) {
        std::cout << "HERE: " << lods.at(lods.size()-1) << std::endl;
    }

    uint32_t array1[] = {1,0, 0,0,
                        0,1, 0,1,};
    std::vector<uint32_t> v1 = generateMipmapsForOctree(array1, glm::ivec3(2));
    for (size_t i = 0; i < v1.size(); i++) {
        std::cout << v1.at(i) << " ";
    }
    std::cout << std::endl;

    uint32_t array2[] = {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
                        0,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,0,
                        0,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,0,
                        1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,1,};
    std::vector<uint32_t> v2 = generateMipmapsForOctree(array2, glm::ivec3(4));
    for (size_t i = 0; i < v2.size(); i++) {
        std::cout << v2.at(i) << " ";
    }
    std::cout << std::endl;*/

    sgl::TextureSettings textureSettings;
    textureSettings.type = sgl::TEXTURE_3D;
    return sgl::TexturePtr(new sgl::TextureGL(textureID, size.x, size.y, size.z, textureSettings));
}


void compressedToGPUData(const VoxelGridDataCompressed &compressedData, VoxelGridDataGPU &gpuData)
{
    gpuData.gridResolution = compressedData.gridResolution;
    gpuData.quantizationResolution = compressedData.quantizationResolution;
    gpuData.worldToVoxelGridMatrix = compressedData.worldToVoxelGridMatrix;

    gpuData.voxelLineListOffsets = sgl::Renderer->createGeometryBuffer(
            sizeof(uint32_t)*compressedData.voxelLineListOffsets.size(),
            (void*)&compressedData.voxelLineListOffsets.front());
    gpuData.numLinesInVoxel = sgl::Renderer->createGeometryBuffer(
            sizeof(uint32_t)*compressedData.numLinesInVoxel.size(),
            (void*)&compressedData.numLinesInVoxel.front());

    /*auto octreeLODs = compressedData.octreeLODs;
    for (uint32_t &value : octreeLODs) {
        value = 1;
    }*/

    gpuData.densityTexture = generateDensityTexture(compressedData.voxelDensityLODs, gpuData.gridResolution);
    gpuData.octreeTexture = generateOctreeTexture(compressedData.octreeLODs, gpuData.gridResolution);

#ifdef PACK_LINES
    int baseSize = sizeof(LineSegmentCompressed);
#else
    int baseSize = sizeof(LineSegment);
#endif

    gpuData.lineSegments = sgl::Renderer->createGeometryBuffer(
            baseSize*compressedData.lineSegments.size(),
            (void*)&compressedData.lineSegments.front());
}