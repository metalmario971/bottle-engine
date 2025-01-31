/**
*  @file Gu.h
*  @date November 6, 2016
*  @author MetalMario971
*/
#pragma once
#ifndef __GU_14784108333124451735_H__
#define __GU_14784108333124451735_H__

#include "../base/BaseHeader.h"
#include "../gfx/GfxHeader.h"
#include "../math/MathHeader.h"
#include "../world/WorldHeader.h"
#include "../base/Stopwatch.h"

namespace BR2 {
/**
*  @class Gu
*  @brief Global Utilities.
*/
class Gu : public GameMemory {
public:
  static std::shared_ptr<RenderSettings> getRenderSettings();
  static std::shared_ptr<ApplicationPackage> getPackage();
  static std::shared_ptr<ModelCache> getModelCache();
  static std::shared_ptr<Sequencer> getSequencer();
  static std::shared_ptr<InputManager> getGlobalInput();
  static std::shared_ptr<SoundCache> getSoundCache();
  static std::shared_ptr<ShaderMaker> getShaderMaker();
  static std::shared_ptr<TexCache> getTexCache();
  static std::shared_ptr<EngineConfig> getConfig();
  static std::shared_ptr<Logger> getLogger();
  static std::shared_ptr<EngineConfig> getEngineConfig();
  static std::shared_ptr<GraphicsApi> getGraphicsApi();
  static std::shared_ptr<Net> getNet();
  static std::shared_ptr<GLContext> getCoreContext();

  static void createLogger(string_t logfile_dir, const std::vector<string_t>& args);
  static bool checkArg(const std::vector<string_t>& args, string_t key, string_t value);
  static void initGlobals(const std::vector<std::string>& args);
  static void createManagers();
  static void updateManagers();
  static void deleteManagers();
  static void setPackage(std::shared_ptr<ApplicationPackage> x) { _pPackage = x; }
  static void setGraphicsApi(std::shared_ptr<GraphicsApi> api);

  static void checkErrorsDbg();
  static void checkErrorsRt();
  static void debugBreak();
  static void checkMemory();
  static bool isDebug();
  static bool is64Bit();
  static bool isBigEndian();
  static string_t getOperatingSystemName();
  static uint32_t getCurrentThreadId();
  static std::string getCPPVersion();
  static void sleepThread(uint64_t milliseconds);

  static t_timeval  getMicroSeconds();
  static t_timeval  getMilliSeconds();

  static std::shared_ptr<Img32> loadImage(std::string path);
  static bool saveImage(std::string path, std::shared_ptr<Img32> spec);
  static void freeImage(std::shared_ptr<Img32> b);
  static int loadSound(std::string path, int& iChannels, int& iSampleRate, int16_t*& pData, int& nSamples, int& iDataLenBytes);

  static void inlineDrawBoxCont(const Box3f* b);
  static void guiQuad2d(Box2f& pq, std::shared_ptr<RenderViewport> vp);//Transforms a quad for the matrix-less 
  static float fade(float t);

  static void print(char msg);
  static void print(const char* msg);
  static void print(const string_t& msg);


  static std::vector<string_t> argsToVectorOfString(int argc, char** argv, char delimiter = '=');

  template < typename Tx >
  static bool addIfDoesNotContain(std::vector<Tx>& vec, Tx& rhs) {
    for (size_t i = 0; i < vec.size(); ++i) {
      if (vec[i] == rhs) {
        return false;
      }
    }
    vec.push_back(rhs);
    return true;
  }
  template <typename Tx>
  static void incrementEnum(Tx& __in_ eValue, Tx iMaxValue) {
    eValue = (Tx)((int)eValue + 1);
    if ((int)eValue >= iMaxValue) {
      eValue = (Tx)0;
    }
  }


private:
  static void loadConfig(const std::vector<std::string>& args);

  static std::shared_ptr<TexCache> _pTexCache;
  static std::shared_ptr<ParticleManager> _pParty;
  static std::shared_ptr<Sequencer> _pSequencer;
  static std::shared_ptr<InputManager> _pGlobalInput;
  static std::shared_ptr<SoundCache> _pSoundCache;
  static std::shared_ptr<ShaderMaker> _pShaderMaker;
  static std::shared_ptr<ModelCache> _pModelCache;
  static std::shared_ptr<ApplicationPackage> _pPackage;
  static std::shared_ptr<RenderSettings> _pRenderSettings;
  static std::shared_ptr<EngineConfig> _pEngineConfig;
  static std::shared_ptr<Logger> _pLogger;
  static std::shared_ptr<GraphicsApi> _pGraphicsApi;
  static std::shared_ptr<Net> _pNet;
};

#define Graphics Gu::getCoreContext()
#define Config Gu::getEngineConfig()



}//ns Game



#endif
