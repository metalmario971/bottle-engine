/**
*  @file LairManager.h
*  @date April 30, 2015
*  @author MetalMario971
*/
#pragma once
#ifndef __LairManager_2266920212277623054825_H__
#define __LairManager_2266920212277623054825_H__

#include "../bottle/BottleHeader.h"

namespace BR2 {
/**
*    The former lair manager.
* This makes the dungeons "lairs".
*/
class WorldMaker : public VirtualMemory {
public:
  WorldMaker(std::shared_ptr<World25> pWorld25, std::shared_ptr<SpriteBucket> pBucket, std::vector<LairSpec*>& mapLairs, std::vector<WalkerSpec*>& mapWalkers);
  virtual ~WorldMaker() override;
  void initialize();

  FORCE_INLINE size_t getNumLairSpecs() { return _mapLairSpecs.size(); }
  FORCE_INLINE  std::map<Hash32, LairSpec*>& getLairSpecs() { return _mapLairSpecs; }

  LairSpec* getCurrentLair() { return _pCurrentLair; }
  void selectRandomLair();

  std::shared_ptr<WorldGrid> loadGrid(ivec3(v));
  void saveGrid(std::shared_ptr<WorldGrid>);

  void makeNewWorld();

  void loadAllGrids(std::set<std::shared_ptr<WorldGrid>>& __out_ grids);
private:
  const int c_iGlobDigits = 4;

  std::map<Hash32, LairSpec*> _mapLairSpecs;
  std::set<WalkerSpec*> _mapWalkerSpecs;
  std::shared_ptr<SpriteBucket> _pMotionBucket = nullptr;
  LairSpec* _pDefaultLair = nullptr;
  std::shared_ptr<World25> _pWorld25 = nullptr;

  LairSpec* getLairSpecById(Hash32 id);

  std::set<MorphTile*> _setMorphTileGroups;
  LairSpec* _pCurrentLair = nullptr;

  std::map<ivec3*, WipGrid*, vec3::Vec3xCompLess> _wipGrids;// For creating the world.

  string_t makeGridFileName(string_t strGameName, string_t strWorldName, int32_t ix, int32_t iy, int32_t iz);
  string_t getWorldGridsDir(string_t strGameName, string_t strWorldName);
  std::shared_ptr<WorldGrid> loadGridFromFile(string_t strFileName);
};
}//ns game

#endif
