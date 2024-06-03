/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Simona Ceskova <xcesko00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    12.12.2023
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

//mGridSizebuild = velikost hrany
unsigned TreeMeshBuilder::buildCubeTree(unsigned mGridSizebuild, float OffsetX, float OffsetY, float OffsetZ, const ParametricScalarField &fieldBuild)
{
    unsigned totalTriangles = 0;

    //cut-off
    if (mGridSizebuild <= 1) {
        Vec3_t<float> cubeOffset(OffsetX, OffsetY, OffsetZ);
        return buildCube(cubeOffset, fieldBuild);
    }
    else if (mGridSizebuild > 1)
    {
        unsigned mGridSizebuildHalfed = mGridSizebuild/2;
        float x = (OffsetX + mGridSizebuildHalfed)*mGridResolution;
        float y = (OffsetY + mGridSizebuildHalfed)*mGridResolution;
        float z = (OffsetZ + mGridSizebuildHalfed)*mGridResolution;
        
        //prazdnost bloku
        Vec3_t<float> p(x,y,z);
        if (evaluateFieldAt(p, fieldBuild) > fieldBuild.getIsoLevel() + (sqrt(3) / 2) * (mGridSizebuild * mGridResolution)) {
            return 0;
        }

        //kombinace pro vypocet jednotlivych kosticek podle toho kde jsou ve vetsi kostce
        // {0, 0, 0, 0, 1, 1, 1, 1};
        // {0, 0, 1, 1, 0, 0, 1, 1};
        // {0, 1, 0, 1, 0, 1, 0, 1};

        // 1 = 001
        // 2 = 010
        // 4 = 100
        // --------------
        // 0 = 000
        // 1 = 001
        // 2 = 010
        // 3 = 011
        // 4 = 100
        // 5 = 101
        // 6 = 110
        // 7 = 111
        //8 kombinaci protoze delim kostku na 8 mensich kostek
        for (int i = 0; i < 8; i++) {
            #pragma omp task shared(totalTriangles)
            {
                float xx = OffsetX + ((i & 1) ? mGridSizebuildHalfed : 0);
                float yy = OffsetY + ((i & 2) ? mGridSizebuildHalfed : 0);
                float zz = OffsetZ + ((i & 4) ? mGridSizebuildHalfed : 0);
                #pragma omp atomic
                totalTriangles += buildCubeTree(mGridSizebuildHalfed,xx,yy,zz,fieldBuild);
            }
        }
    }
    #pragma omp taskwait
    return totalTriangles;
}
//rodicovske procesy pockaji na child procesy
unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    unsigned totalTriangles = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {   
            totalTriangles = buildCubeTree(mGridSize, 0, 0, 0, field);
        }
    }
    return totalTriangles;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();
    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);
        value = std::min(value, distanceSquared);
    }
    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    //kriticka sekce
    #pragma omp critical
    mTriangles.push_back(triangle);
}
