#include "Rendering.h"

using namespace Rendering;

GPU_Target *Rendering::gpu;

void Rendering::Render3DPolygon(const GPU_Image *texture, const GPU_Target *target, const VERTEX3D verts[], const uint16_t vertCount, const uint16_t indices[], const uint triCount){

}

void Rendering::Render2DPolygon(const GPU_Image *texture, const GPU_Target *target, const VERTEX2D verts[], const uint16_t vertCount, const uint16_t indices[], const uint triCount){

}

void Rendering::Set3DCamera(){
    //SetCameraPositionAndTarget_UpVecY(VGet(0, SU_TO_FLOAT(cameraY), SU_TO_FLOAT(cameraZ)), VGet(0, SU_LANE_Y_GROUND, SU_TO_FLOAT(cameraTargetZ)));
}

void Rendering::Clear3DCamera(){

}

VECTOR Rendering::WorldToScreen(VECTOR worldspace){
    return worldspace;
}