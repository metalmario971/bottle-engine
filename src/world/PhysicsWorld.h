/**
*  @file PhysicsWorld.h
*  @date January 10, 2018
*  @author MetalMario971
*/
#pragma once
#ifndef __GRIDMANAGER_15156110951778027750_H__
#define __GRIDMANAGER_15156110951778027750_H__

#include "../world/WorldHeader.h"

namespace BR2 {
/**
*  @class PhysicsWorld
*  @brief Operates on the world grid and manages the entire physical world and objects and collisions.
*  @details Stores all objects in the world.  Also stores all Grids, and performs collision checking against all objectts and grids..  Centerpoint of your game. Inherit from this class to process your game world.  call update() each frame
*/
class PhysicsWorld : public VirtualMemoryShared<PhysicsWorld> {
public:
  typedef std::map<ivec3*, std::shared_ptr<PhysicsGrid>, ivec3::Vec3xCompLess> GridMap;
  typedef std::map<NodeId, std::shared_ptr<PhysicsNode>> ObjMap;
  static void limitVelocity(vec3& __inout_ v);
  static void limitAccelleration(vec3& __inout_ va);
  static float getGravityAmount() { return -0.98f; }
  static ivec3 v3Toi3Any(vec3& v, float w1, float h1);

public:
  PhysicsWorld(std::shared_ptr<Scene> pscene);
  virtual ~PhysicsWorld() override;

  static std::shared_ptr<PhysicsWorld> create(std::shared_ptr<Scene>, float fNodeWidth, float fNodeHeight, vec3& vUp,
      MpFloat awXZ, float awXZInc, MpFloat awY, float awYInc,
      MpInt mpNodesY, uint32_t iGridCountLimit);

  virtual void update(float delta);
  void unloadWorld();
  void getNodeBoxForGridPos(const ivec3& pt, Box3f& __out_ box) const;
  void collectVisibleNodes(std::shared_ptr<RenderBucket> rb);

  std::multimap<float, std::shared_ptr<PhysicsGrid>>& getVisibleGrids();
  std::multimap<float, std::shared_ptr<SceneNode>>& getVisibleNodes();
  float getNodeWidth()const { return _fNodeWidth; }
  float getNodeHeight()const { return _fNodeHeight; }
  std::shared_ptr<PhysicsGridAwareness> getAwareness() { return _pAwareness; }
  std::shared_ptr<PhysicsGrid> getNodeAtPos(ivec3& pos);
  GridMap& getGrids() { return _gridMap; }
  ivec3 v3Toi3Node(vec3& v);
  vec3 i3tov3Node(const ivec3& iNode);
  size_t getNumObjects() { return _mapObjects.size(); }
  size_t getNumActiveObjects() { return _vecActiveFrame.size(); }
  size_t  getNumGrids() { return _gridMap.size(); }
  std::shared_ptr<PhysicsGrid> getNodeForPoint(vec3& pt);
  std::shared_ptr<RenderBucket> getRenderBucket() { return _pRenderBucket; }
  std::shared_ptr<Scene> getScene() { return _pScene; }

  void addObj(std::shared_ptr<PhysicsNode> ob, bool bActivate, bool bRefreshCache);
  bool tryRemoveObj(std::shared_ptr<PhysicsNode> ob);
  std::shared_ptr<PhysicsNode> getObj(NodeId obid);
  void activate(std::shared_ptr<PhysicsNode> pPhy);
  void refreshObjectManifold(std::shared_ptr<PhysicsNode> ob);

  void clearObjectManifoldAndRemoveFromGrid(std::shared_ptr<PhysicsNode> ob);
  void reparentObjectByCustomBox(std::shared_ptr<PhysicsNode> ob, Box3f* pBox);
  void getNodeRangeForBox(Box3f* c, ivec3* __out_ p0, ivec3* __out_ p1, bool bLimitByWorldBox);
  void addGrid(std::shared_ptr<PhysicsGrid> pGrid, const ivec3& cv);
  void debugMakeSureNoDupes(const ivec3& vv);

  std::shared_ptr<PhysicsNode> findNode(string_t specName);
  template < typename Tx > bool findNode(std::shared_ptr<Tx>& __out_ node) {
    for (auto a : _mapObjects) {
      if (a.second->findNode(node)) {
        return true;
      }
    }
    return false;
  }

private:
  float _fNodeWidth = 0;
  float _fNodeHeight = 0;
  GridMap _gridMap;
  ObjMap _mapObjects;
  std::set<ivec3*, ivec3::Vec3xCompLess> _setEmpty;
  uint64_t _iStamp = 1;
  int32_t _nEmbedded = 0;
  int32_t _frameStamp = 0;
  vec3 _vUp;  //For world25 This should be 0,0,-1, for any real 3D world use 0,1,0
  std::set<std::shared_ptr<PhysicsNode>> _vecActiveFrame; //Activated objects per frame.
  std::shared_ptr<PhysicsGridAwareness> _pAwareness = nullptr;
  MpInt _mpNodesY;
  uint32_t _iGridCountLimit = 100;
  std::unique_ptr<Box3f> _pWorldBox = nullptr;
  std::shared_ptr<RenderBucket> _pRenderBucket = nullptr;
  std::shared_ptr<Scene> _pScene = nullptr;

  virtual void init(float fNodeWidth, float fNodeHeight, vec3& vUp,
    MpFloat awXZ, float awXZInc, MpFloat awY, float awYInc,
    MpInt mpNodesY, uint32_t iGridCountLimit);
  virtual std::shared_ptr<PhysicsGrid> loadGrid(const ivec3& pos);
  bool unstick_objs(std::shared_ptr<PhysicsNode> objA, std::shared_ptr<PhysicsNode> objB, Box3f* in_bA, Box3f* in_bB);
  void updateWorldBox();
  void sortObjects_CalculateSpeedboxes_And_CollectManifolds();
  void collisionLoopDual(float delta);
  void collide();
  void postCollide(uint64_t frameId, bool bAccel);
  int32_t unstick();
  int32_t collect_and_resolve_collisions();
  int32_t stabalizeCollisionSystem(std::multimap<float, BoxCollision>& boxCol);
  int32_t collisionTestBox(std::shared_ptr<PhysicsNode> objA, std::shared_ptr<PhysicsNode> objB, std::multimap<float, BoxCollision>& boxCol);
  void resolve_pair_t(BoxCollision& boxCol);
  void collide_loop();
  void unstick_loop();
  bool checkMoved(vec3& p_last, vec3& p_cur, vec3& v_cur);
  void unstick_if_moved(std::shared_ptr<PhysicsNode> obA, std::shared_ptr<PhysicsNode> obB, Box3f* in_bA, Box3f* in_bB,
    vec3& velA_new, vec3& velB_new, float move_t);
  void unstick_ob_v2(vec3& ob_in_p, vec3& ob_in_v, Box3f* boxA, Box3f* boxB,
    float move_t, float ob_friction, vec3& __out_ out_new_v);
  void calc_obj_manifold(std::shared_ptr<PhysicsNode> ob);
  void bb_resolve_setup_data(std::shared_ptr<PhysicsNode> obA, std::shared_ptr<PhysicsNode> obB, vec3& v_last_a, vec3& v_last_b,
    Box3f* boxA, Box3f* boxB, float move_t, int ax_t0);
  void bb_move_and_slide(
    const vec3& ob_last_p, const vec3& ob_last_v,
    const vec3& ob_new_p, const vec3& ob_move_v, Box3f* boxA, Box3f* boxB,
    float move_t, float ob_friction, vec3& __out_ out_new_v, const int ax_t0);
  void makeGrid();
  void makeOrCollectGridForPos(ivec3& cv, std::vector<std::shared_ptr<PhysicsGrid>>& vecGenerated);
  void calcGridManifold(std::shared_ptr<PhysicsGrid> pGrid);
  void addNodeToGrid(std::shared_ptr<PhysicsNode> ob, std::shared_ptr<PhysicsGrid> pGrid);
  bool collideMesh(std::shared_ptr<PhysicsNode> ob, vec3* verts, size_t vOffBytes, size_t vStrideBytes, size_t vCount, v_index32* inds,
    int32_t iOff, int32_t iCount);
  void sweepGridBox(std::function<void(ivec3&)> func, ivec3& viMin, ivec3& viMax);
  void sweepGridFrustum(std::function<void(ivec3&)> func, std::shared_ptr<FrustumBase> pf, float fMaxDist2);
};

}//ns Game



#endif
