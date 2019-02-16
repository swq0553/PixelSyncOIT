//
// Created by christoph on 09.02.19.
//

#include <boost/algorithm/string/predicate.hpp>

#include <Utils/File/FileUtils.hpp>
#include <Math/Geometry/MatrixUtil.hpp>

#include "../VoxelRaytracing/VoxelData.hpp"
#include "../VoxelRaytracing/VoxelCurveDiscretizer.hpp"

#include "VoxelAO.hpp"

void VoxelAOHelper::loadAOFactorsFromVoxelFile(const std::string &filename)
{
    // Check if voxel grid is already created
    // Pure filename without extension (to create compressed .voxel filename)
    std::string modelFilenamePure = sgl::FileUtils::get()->removeExtension(filename);

    std::string modelFilenameVoxelGrid = modelFilenamePure + ".voxel";

    // Can be either hair dataset or trajectory dataset
    bool isHairDataset = boost::starts_with(modelFilenamePure, "Data/Hair");

    VoxelGridDataCompressed compressedData;
    if (!sgl::FileUtils::get()->exists(modelFilenameVoxelGrid)) {
        VoxelCurveDiscretizer discretizer(glm::ivec3(128), glm::ivec3(64));
        if (isHairDataset) {
            std::string modelFilenameHair = modelFilenamePure + ".hair";
            float lineRadius;
            glm::vec4 hairStrandColor;
            discretizer.createFromHairDataset(modelFilenameHair, lineRadius, hairStrandColor);
        } else {
            std::string modelFilenameObj = modelFilenamePure + ".obj";
            std::vector<float> attributes;
            float maxVorticity;
            discretizer.createFromTrajectoryDataset(modelFilenameObj, attributes, maxVorticity);
        }
        compressedData = discretizer.compressData();
        saveToFile(modelFilenameVoxelGrid, compressedData);
    } else {
        loadFromFile(modelFilenameVoxelGrid, compressedData);
    }

    aoTexture = generateDensityTexture(compressedData.voxelAOLODs, compressedData.gridResolution);
    worldToVoxelGridMatrix = compressedData.worldToVoxelGridMatrix;
    gridResolution = compressedData.gridResolution;
}

void VoxelAOHelper::setUniformValues(sgl::ShaderProgramPtr transparencyShader)
{
    sgl::TexturePtr aoTexture = getAOTexture();
    transparencyShader->setUniform("aoTexture", aoTexture, 4);
    glm::mat4 worldSpaceToVoxelTextureSpace =
            sgl::matrixScaling(1.0f / glm::vec3(gridResolution)) * worldToVoxelGridMatrix;
    transparencyShader->setUniform("worldSpaceToVoxelTextureSpace", worldSpaceToVoxelTextureSpace);
}