/**
*  @file GameSaveFile.h
*  @date December 22, 2017
*/
#pragma once
#ifndef __GAMESAVEFILE_15139585672481540946_H__
#define __GAMESAVEFILE_15139585672481540946_H__

#include "../base/PoundFile.h"
#include "../world/WorldHeader.h"

namespace BR2 {
/**
*    @class GameSaveFile
*/
class GameFile : public PoundFile {
public:
  GameFile();
  virtual ~GameFile() override;

  string_t getWorldName() { return _strWorldName; }
  void save(string_t loc, std::shared_ptr<PhysicsWorld> pw);
  
private:
  //Inherited
  virtual void pkp(std::vector<string_t>& tokens);
  virtual void preLoad();
  virtual void postLoad();

  string_t _strWorldName;


};

}//ns Game



#endif
