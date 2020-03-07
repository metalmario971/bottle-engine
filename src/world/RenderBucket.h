/**
*  @file RenderBucket.h
*  @date January 22, 2018
*  @author MetalMario971
*/
#pragma once
#ifndef __RenderBucket_15166495182133110211_H__
#define __RenderBucket_15166495182133110211_H__

#include "../gfx/GfxHeader.h"
#include "../world/WorldHeader.h"

namespace BR2 {
class BvhCollectionParams : public VirtualMemory {
public:
  std::shared_ptr<FrustumBase> _pFrustum = nullptr;    // the frustum to collect from
  std::shared_ptr<RenderBucket> _pRenderBucket = nullptr;
  std::shared_ptr<CameraNode> _pVisibleCamera = nullptr;
  float _fMaxDist = 80;// max distance that we collect

  BvhCollectionParams() {}
  BvhCollectionParams(RenderBucket* pSet, vec3& rootPt, std::shared_ptr<FrustumBase> collectionFrustum, bool createMissingNodes) {}
};

/**
*  @class RenderBucket
*  @brief Collects scene elements for rendering.
*/
class RenderBucket : public VirtualMemoryShared<RenderBucket> {
public:
  RenderBucket();
  virtual ~RenderBucket() override;

  void collect(std::shared_ptr<SceneNode> bn);
  void clear(std::shared_ptr<CameraNode> visible_camera);
  
  void addObj(std::shared_ptr<SceneNode> bn);
  void addGrid(std::shared_ptr<PhysicsGrid> bn);
  bool hasItems();

  void sortAndDrawMeshes(std::function<std::shared_ptr<ShaderBase>(std::shared_ptr<VertexFormat>)> shaderSearch,
    std::function<void(std::shared_ptr<ShaderBase>)> shaderBind,
    std::function<void(std::shared_ptr<ShaderBase>, std::shared_ptr<MeshNode>)> shaderDraw);

  std::multimap<float, std::shared_ptr<LightNodePoint>>& getPointLights() { return _mapPointLights; }
  std::multimap<float, std::shared_ptr<LightNodeDir>>& getDirLights() { return _mapDirLights; }
  std::multimap<float, std::shared_ptr<LightNodeBase>>& getLights() { return _mapLights; }
  std::multimap<float, std::shared_ptr<MeshNode>>& getMeshes() { return _mapMeshes; }
  std::multimap<float, std::shared_ptr<MeshNode>>& getMeshesTransparent() { return _mapMeshesTransparent; }
  std::multimap<float, std::shared_ptr<SceneNode>>& getObjs() { return _mapObjs; }
  std::multimap<float, std::shared_ptr<PhysicsGrid>>& getGrids() { return _mapGrids; }

private:
  typedef std::map<std::shared_ptr<ShaderBase>, std::vector<std::shared_ptr<MeshNode>>> RenderMap;
  std::multimap<float, std::shared_ptr<SceneNode>> _mapObjs;
  std::multimap<float, std::shared_ptr<PhysicsGrid>> _mapGrids;
  std::multimap<float, std::shared_ptr<LightNodePoint>> _mapPointLights;
  std::multimap<float, std::shared_ptr<LightNodeDir>> _mapDirLights;
  std::multimap<float, std::shared_ptr<LightNodeBase>> _mapLights;
  std::multimap<float, std::shared_ptr<MeshNode>> _mapMeshes;
  std::multimap<float, std::shared_ptr<MeshNode>> _mapMeshesTransparent;
  RenderMap _renderMap;
  std::shared_ptr<CameraNode> _pVisibleCamera = nullptr;
  vec3 _vCachedCamPos;

  // RenderMap _renderMap;
  float distToCam(Box3f* n);
  //  void addToRenderMap(std::shared_ptr<MeshNode> bn);
};

}//ns Game



#endif
