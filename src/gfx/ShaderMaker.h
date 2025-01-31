/**
*  @file ShaderMaker.h
*  @date May 5, 2017
*  @author MetalMario971
*/
#pragma once
#ifndef __SHADERMAKER_14940026902175372902_H__
#define __SHADERMAKER_14940026902175372902_H__

#include "../gfx/GfxHeader.h"

namespace BR2 {
/**
*  @class ShaderMaker
*  @brief Compiles GLSL shaders.
*/
class ShaderMaker : public VirtualMemory {
  typedef std::map<std::shared_ptr<VertexFormat>, std::shared_ptr<ShaderBase>> ShaderMap;
public:
  ShaderMaker();
  virtual ~ShaderMaker() override;

  void shaderBound(std::shared_ptr<ShaderBase> sb);
  std::shared_ptr<ShaderBase> makeShader(std::vector<string_t>& vecFiles);

  std::shared_ptr<ShaderUniformBlock> getUniformBlockByName(string_t& blockName);

  void initialize();
  std::shared_ptr<ShaderBase> getDiffuseShader(std::shared_ptr<VertexFormat> fmt);
  std::shared_ptr<ShaderBase> getGlassShader(std::shared_ptr<VertexFormat> fmt);
  std::shared_ptr<ShaderBase> getFlatShader(std::shared_ptr<VertexFormat> fmt);
  std::shared_ptr<ShaderBase> getShadowShader(std::shared_ptr<VertexFormat> fmt);

  std::shared_ptr<ShaderBase> getImageShader_F() { return _pImageShader; }
  std::shared_ptr<ShaderBase> getNormalsShader_v3n3() { return _pNormalsShader; }
  std::shared_ptr<ShaderBase> getShadowBlendShader() { return _pShadowBlendShader; }
  std::shared_ptr<ShaderBase> getDepthOfFieldShader() { return _pDepthOfField; }
  std::shared_ptr<ShaderBase> getSmoothGenShader() { return _pSmoothGen; }

  std::shared_ptr<ShaderBase> getGuiShader() { return _pGuiShader; }

  std::shared_ptr<ShaderBase> getShaderById(GLuint glId);
  std::shared_ptr<ShaderBase> getShaderByName(const string_t& name);
  string_t getShaderNameForId(GLuint id);

  static bool isGoodStatus(ShaderStatus::e);
  static string_t systemTypeToSTring(OpenGLShaderVarType::e eType);
  std::shared_ptr<ShaderBase> getBound() { return _pBound; }
  void setUfBlock(string_t name, void* value, size_t copySizeBytes, bool bIgnore = false);

  int32_t* getMaxWorkGroupDims() { return _maxWorkGroupDims; }
  int32_t getMaxBufferBindings() { return _maxBufferBindings; }
  int32_t getMaxSsboBindingIndex() { return _iMaxSsboBindingIndex; }

protected:
  std::shared_ptr<ShaderBase> _pBound = nullptr;
  std::map<Hash32, std::shared_ptr<ShaderBase>> _mapPrograms;
  std::set<std::shared_ptr<ShaderSubProgram>> _setSubPrograms;
  std::map<Hash32, std::shared_ptr<ShaderUniformBlock>> _mapUniformBlocks;

  std::shared_ptr<GLContext> _pContext = nullptr;
  std::shared_ptr<ShaderCompiler> _pShaderCompiler = nullptr;
  std::shared_ptr<ShaderCache> _pShaderCache = nullptr;
  std::vector<std::shared_ptr<ShaderBase>> _vecShaders;
  std::vector<string_t> _vecGeneralErrors;

  //Default Shaders
  //std::shared_ptr<ShaderBase> _pFlatShader = nullptr;
  std::shared_ptr<ShaderBase> _pImageShader = nullptr;
  std::shared_ptr<ShaderBase> _pNormalsShader = nullptr;
  std::shared_ptr<ShaderBase> _pGuiShader = nullptr;
  std::shared_ptr<ShaderBase> _pShadowBlendShader = nullptr;
  std::shared_ptr<ShaderBase> _pSmoothGen = nullptr;
  std::shared_ptr<ShaderBase> _pDepthOfField = nullptr;
  ShaderMap _pShadowShaders;
  ShaderMap _pDiffuseShaders;
  ShaderMap _pGlassShaders;
  ShaderMap _pFlatShaders;

  int32_t _maxWorkGroupDims[3];//Compute Shader Only
  int32_t _maxBufferBindings;//Compute Shader Only
  int32_t _iMaxSsboBindingIndex;//Compute Shader Only
  void getComputeLimits();//Compute Shader Only

  bool checkForErrors(std::vector<std::shared_ptr<ShaderSubProgram>>& vx, std::shared_ptr<ShaderBase> sp);
  std::shared_ptr<ShaderSubProgram> getShaderSubProgramByLocation(DiskLoc l);
  void compileShaderSubProgram(std::shared_ptr<ShaderSubProgram> pShader);

  void deleteShader(std::shared_ptr<ShaderBase> prog);

  // void initialize();
  void addGeneralError(string_t str);
  void parseAttributes(std::shared_ptr<ShaderBase> sb);
  void parseUniforms(std::shared_ptr<ShaderBase> sb);
  void removeDuplicateSourceFiles(std::vector<string_t>& vecFiles);
  void loadSubPrograms(std::vector<string_t>& vecFiles, std::vector<std::shared_ptr<ShaderSubProgram>>& __out_ subProgs);
  bool validateShadersForProgram(std::shared_ptr<ShaderBase> psp);
  void getProgramErrorLog(std::shared_ptr<ShaderBase> psp, std::vector<string_t>& __out_ log);
  string_t getShaderNameFromFileNames(std::vector<string_t>& vecFiles);
  void fullyQualifyFiles(std::vector<string_t>& vecFiles);
  string_t getGeneralErrorsAsString(bool clear = true);
  std::shared_ptr<ShaderSubProgram> preloadShaderSubProgram(DiskLoc loc);
  std::shared_ptr<ShaderBase> makeProgram(std::vector<std::shared_ptr<ShaderSubProgram>>& vecpsp, string_t& programName);
  bool validateSubProgram(std::shared_ptr<ShaderSubProgram> prog, std::shared_ptr<ShaderBase> psp);
  // std::vector<t_string> getProgramErrorLog(ShaderProgram* sp);
   //GLenum getGLShaderEnum(ShaderType::e type);
  void parseUniformBlocks(std::shared_ptr<ShaderBase> sb);
  std::shared_ptr<ShaderBase> getValidShaderForVertexType(ShaderMap& shaders, std::shared_ptr<VertexFormat> fmt);
};

}//ns Game



#endif
