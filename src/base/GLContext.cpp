#include "../base/GlobalIncludes.h"
#include "../base/BaseHeader.h"
#include "../base/TypeConv.h"
#include "../base/StringUtil.h"
#include "../base/Logger.h"
#include "../base/GLContext.h"
#include "../base/Gu.h"
#include "../base/InputManager.h"
#include "../base/Sequencer.h"
#include "../base/GraphicsWindow.h"
#include "../base/Logger.h" 
#include "../base/FrameSync.h"
#include "../base/SoundCache.h"
#include "../base/Logger.h"
#include "../base/SDLUtils.h"
#include "../base/oglErr.h"
#include "../base/GraphicsWindow.h"
#include "../base/EngineConfig.h"
#include "../math/MathAll.h"
#include "../gfx/RenderUtils.h"
#include "../gfx/ParticleManager.h"
#include "../gfx/TexCache.h"
#include "../gfx/ShaderMaker.h"
#include "../gfx/LightManager.h"
#include "../gfx/Picker.h"
#include "../gfx/UiControls.h"
#include "../gfx/CameraNode.h"
#include "../gfx/RenderSettings.h"
#include "../gfx/GraphicsContext.h"
#include "../model/VertexFormat.h"
#include "../model/ModelCache.h"
#include "../world/Scene.h"

namespace BR2 {
GLContext::GLContext(std::shared_ptr<GraphicsApi> api, std::shared_ptr<GLProfile> profile, SDL_Window* sdl_win) : GraphicsContext(api) {
  _profile = profile;
  _pSDLWindow = sdl_win;

  _context = SDL_GL_CreateContext(sdl_win);
  if (!_context) {
    //Eat the "context failed" error.  It's not informative.
    SDLUtils::checkSDLErr(false, false);
    _bValid = false;
  }
  else {
    int ver, subver, shad_ver, shad_subver;
    getOpenGLVersion(ver, subver, shad_ver, shad_subver);

    //Make sure we have a good depth value.
    int iDepth = 0;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &iDepth);
    _iSupportedDepthSize = iDepth;
    if (iDepth < profile->_iDepthBits) {
      BRLogWarn("Depth buffer size " + profile->_iDepthBits = " not supported in profile.");
      _bValid = false;
    }
    else if (checkOpenGlMinimumVersionInfo(profile->_iMinVersion, profile->_iMinSubVersion)) {
      printHelpfulDebug();

      if (profile->_iMSAABuffers == 0) {
        Gu::getEngineConfig()->setEnableMsaa(false);
        BRLogWarn("This configuration did not support MSAA.");
      }

      if (!loadOpenGLFunctions()) {
        BRLogError("Failed to load context.");
        SDL_GL_DeleteContext(_context);
        _bValid = false;
      }
      else {
        _bForwardCompatible = profile->_bForwardCompatible;

        //Quick GL test.
        glUseProgram(0);

        //Check that OpenGL initialized successfully
        loadCheckProc();

        _bValid = true;
      }
    }
  }
}
GLContext::~GLContext() {
  if (_context) {
    /* SDL_GL_MakeCurrent(0, NULL); *//* doesn't do anything */
    SDL_GL_DeleteContext(_context);
  }
}
void GLContext::setPolygonMode(PolygonMode p) {
  if (Gu::getCoreContext()->getForwardCompatible() == true) {
    GLenum mode = GL_FILL;
    if (p == PolygonMode::Line) { mode = GL_LINE; }
    else if (p == PolygonMode::Point) { mode = GL_POINT; }
    //These must be #ifdef out because glPolygonMOde is not present in gl330 core 
    glPolygonMode(GL_FRONT, mode);
    glPolygonMode(GL_BACK, mode);
  }
  else {
    //Uh..No
    BRLogErrorOnce("glPolygonMode not supported in core compatibility context.");
  }
}
bool GLContext::chkErrRt(bool bDoNotBreak, bool doNotLog) {
  //Enable runtime errors.
  return OglErr::chkErrRt(shared_from_this(), bDoNotBreak, doNotLog);
}
bool GLContext::chkErrDbg(bool bDoNotBreak, bool doNotLog) {
  return OglErr::chkErrDbg(shared_from_this(), bDoNotBreak, doNotLog);
}
bool GLContext::loadOpenGLFunctions() {
  bool bValid = true;

#define SDLGLP(x, y, z) do{ \
if( !(x = (y)SDL_GL_GetProcAddress(z))) \
{ \
BRLogError("GL Method "+ z+" not supported by your GPU, or there was another error somewhere.."); \
bValid = false; \
Gu::debugBreak(); \
} \
} while(0)

#define SDLGLP2(x, y) do{ \
if( !(y = (x)SDL_GL_GetProcAddress(#y))) \
{ \
BRLogError("GL Method "+ #y+ " not supported by your GPU, or there was another error somewhere.."); \
bValid = false; \
Gu::debugBreak(); \
} \
} while(0)

  SDLGLP(glUseProgram, PFNGLUSEPROGRAMPROC, "glUseProgram");
  SDLGLP(glBindBuffer, PFNGLBINDBUFFERPROC, "glBindBuffer");
  SDLGLP(glGenBuffers, PFNGLGENBUFFERSPROC, "glGenBuffers");
  SDLGLP(glBufferData, PFNGLBUFFERDATAPROC, "glBufferData");
  SDLGLP(glBufferSubData, PFNGLBUFFERSUBDATAPROC, "glBufferSubData");
  SDLGLP(glShaderSource, PFNGLSHADERSOURCEPROC, "glShaderSource");
  SDLGLP(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC, "glGetShaderInfoLog");
  SDLGLP(glCreateProgram, PFNGLCREATEPROGRAMPROC, "glCreateProgram");
  SDLGLP(glAttachShader, PFNGLATTACHSHADERPROC, "glAttachShader");
  SDLGLP(glLinkProgram, PFNGLLINKPROGRAMPROC, "glLinkProgram");
  SDLGLP(glDetachShader, PFNGLDETACHSHADERPROC, "glDetachShader");
  SDLGLP(glDeleteShader, PFNGLDELETESHADERPROC, "glDeleteShader");
  SDLGLP(glCompileShader, PFNGLCOMPILESHADERPROC, "glCompileShader");
  SDLGLP(glCreateShader, PFNGLCREATESHADERPROC, "glCreateShader");
  SDLGLP(glDeleteProgram, PFNGLDELETEPROGRAMPROC, "glDeleteProgram");

  SDLGLP(glGetProgramiv, PFNGLGETPROGRAMIVPROC, "glGetProgramiv");
  SDLGLP(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC, "glGetProgramInfoLog");
  SDLGLP(glGetShaderiv, PFNGLGETSHADERIVPROC, "glGetShaderiv");

  SDLGLP(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray");
  SDLGLP(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC, "glVertexAttribPointer");
  SDLGLP(glVertexAttribIPointer, PFNGLVERTEXATTRIBIPOINTERPROC, "glVertexAttribIPointer");
  SDLGLP(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC, "glDisableVertexAttribArray");

  SDLGLP(glActiveTexture, PFNGLACTIVETEXTUREPROC, "glActiveTexture");

  SDLGLP(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC, "glGetUniformLocation");
  SDLGLP(glGetDebugMessageLog, PFNGLGETDEBUGMESSAGELOGPROC, "glGetDebugMessageLog");

  SDLGLP(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC, "glGenVertexArrays");
  SDLGLP(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray");
  SDLGLP(glDeleteBuffers, PFNGLDELETEBUFFERSPROC, "glDeleteBuffers");
  SDLGLP(glIsBuffer, PFNGLISBUFFERPROC, "glIsBuffer");

  SDLGLP(glMapBuffer, PFNGLMAPBUFFERPROC, "glMapBuffer");
  SDLGLP(glUnmapBuffer, PFNGLUNMAPBUFFERPROC, "glUnmapBuffer");
  SDLGLP(glGetBufferParameteriv, PFNGLGETBUFFERPARAMETERIVPROC, "glGetBufferParameteriv");
  SDLGLP(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC, "glDeleteVertexArrays");

  //** Uniforms
  SDLGLP(glUniformMatrix2fv, PFNGLUNIFORMMATRIX2FVPROC, "glUniformMatrix2fv");
  SDLGLP(glUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC, "glUniformMatrix3fv");
  SDLGLP(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC, "glUniformMatrix4fv");

  SDLGLP(glUniform1i, PFNGLUNIFORM1IPROC, "glUniform1i");

  SDLGLP(glUniform1iv, PFNGLUNIFORM1IVPROC, "glUniform1iv");
  SDLGLP(glUniform2iv, PFNGLUNIFORM2IVPROC, "glUniform2iv");
  SDLGLP(glUniform3iv, PFNGLUNIFORM3IVPROC, "glUniform3iv");
  SDLGLP(glUniform4iv, PFNGLUNIFORM4IVPROC, "glUniform4iv");

  SDLGLP(glUniform1fv, PFNGLUNIFORM1FVPROC, "glUniform1fv");
  SDLGLP(glUniform2fv, PFNGLUNIFORM2FVPROC, "glUniform2fv");
  SDLGLP(glUniform3fv, PFNGLUNIFORM3FVPROC, "glUniform3fv");
  SDLGLP(glUniform4fv, PFNGLUNIFORM4FVPROC, "glUniform4fv");

  SDLGLP(glUniform1uiv, PFNGLUNIFORM1UIVPROC, "glUniform1uiv");
  SDLGLP(glUniform2uiv, PFNGLUNIFORM2UIVPROC, "glUniform2uiv");
  SDLGLP(glUniform3uiv, PFNGLUNIFORM3UIVPROC, "glUniform3uiv");
  SDLGLP(glUniform4uiv, PFNGLUNIFORM4UIVPROC, "glUniform4uiv");

  //Framebuffers
  SDLGLP(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC, "glBindFramebuffer");
  SDLGLP(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC, "glBindRenderbuffer");
  SDLGLP(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC, "glGenFramebuffers");
  SDLGLP(glFramebufferParameteri, PFNGLFRAMEBUFFERPARAMETERIPROC, "glFramebufferParameteri");
  SDLGLP(glDrawBuffers, PFNGLDRAWBUFFERSPROC, "glDrawBuffers");
  SDLGLP(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC, "glFramebufferTexture2D");
  SDLGLP(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC, "glCheckFramebufferStatus");
  SDLGLP(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC, "glDeleteFramebuffers");

  SDLGLP(glTexStorage2D, PFNGLTEXSTORAGE2DPROC, "glTexStorage2D");
  SDLGLP(glGenerateMipmap, PFNGLGENERATEMIPMAPPROC, "glGenerateMipmap");

  SDLGLP(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC, "glGenRenderbuffers");
  SDLGLP(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC, "glRenderbufferStorage");
  SDLGLP(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC, "glFramebufferRenderbuffer");


  SDLGLP(glGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC, "glGetUniformBlockIndex");// = nullptr;
  SDLGLP(glUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC, "glUniformBlockBinding");//= nullptr;
  SDLGLP(glBindBufferBase, PFNGLBINDBUFFERBASEPROC, "glBindBufferBase");//        = nullptr;
  SDLGLP(glGetActiveUniform, PFNGLGETACTIVEUNIFORMPROC, "glGetActiveUniform");


  SDLGLP(glGetVertexAttribIuiv, PFNGLGETVERTEXATTRIBIUIVPROC, "glGetVertexAttribIuiv");
  SDLGLP(glGetVertexAttribfv, PFNGLGETVERTEXATTRIBFVPROC, "glGetVertexAttribfv");
  SDLGLP(glGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC, "glGetVertexAttribiv");
  SDLGLP(glGetVertexAttribIiv, PFNGLGETVERTEXATTRIBIIVPROC, "glGetVertexAttribIiv");
  SDLGLP(glGetFramebufferAttachmentParameteriv, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC, "glGetFramebufferAttachmentParameteriv");
  SDLGLP(glGetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC, "glGetActiveUniformBlockiv");

  SDLGLP(glTexImage2DMultisample, PFNGLTEXIMAGE2DMULTISAMPLEPROC, "glTexImage2DMultisample");
  SDLGLP(glBlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC, "glBlitFramebuffer");


  SDLGLP(glGetProgramBinary, PFNGLGETPROGRAMBINARYPROC, "glGetProgramBinary");// = nullptr;

  SDLGLP(glIsProgram, PFNGLISPROGRAMPROC, "glIsProgram");// = nullptr;
  SDLGLP(glProgramBinary, PFNGLPROGRAMBINARYPROC, "glProgramBinary");// = nullptr;
  SDLGLP(glValidateProgram, PFNGLVALIDATEPROGRAMPROC, "glValidateProgram");

  SDLGLP(glGetActiveAttrib, PFNGLGETACTIVEATTRIBPROC, "glGetActiveAttrib");
  SDLGLP(glGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC, "glGetAttribLocation");
  SDLGLP(glMemoryBarrier, PFNGLMEMORYBARRIERPROC, "glMemoryBarrier");
  SDLGLP(glMapBufferRange, PFNGLMAPBUFFERRANGEPROC, "glMapBufferRange");
  SDLGLP(glGetActiveUniformBlockName, PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC, "glGetActiveUniformBlockName");

  SDLGLP(glFenceSync, PFNGLFENCESYNCPROC, "glFenceSync");
  SDLGLP(glIsSync, PFNGLISSYNCPROC, "glIsSync");
  SDLGLP(glGetSynciv, PFNGLGETSYNCIVPROC, "glGetSynciv");
  SDLGLP(glDeleteSync, PFNGLDELETESYNCPROC, "glDeleteSync");
  SDLGLP(glDispatchCompute, PFNGLDISPATCHCOMPUTEPROC, "glDispatchCompute");
  SDLGLP(glGetIntegeri_v, PFNGLGETINTEGERI_VPROC, "glGetIntegeri_v");
  SDLGLP(glShaderStorageBlockBinding, PFNGLSHADERSTORAGEBLOCKBINDINGPROC, "glShaderStorageBlockBinding");
  SDLGLP(glGetProgramResourceIndex, PFNGLGETPROGRAMRESOURCEINDEXPROC, "glGetProgramResourceIndex");

  // SDLGLP2(PFNGLFENCESYNCPROC, glFenceSync);
  // SDLGLP2(PFNGLISSYNCPROC, glIsSync);
  // SDLGLP2(PFNGLGETSYNCIVPROC, glGetSynciv);
  // SDLGLP2(PFNGLDELETESYNCPROC, glDeleteSync);
  // SDLGLP2(PFNGLDISPATCHCOMPUTEPROC, glDispatchCompute);
  // SDLGLP2(PFNGLGETINTEGERI_VPROC, glGetIntegeri_v);
  // SDLGLP2(PFNGLSHADERSTORAGEBLOCKBINDINGPROC, glShaderStorageBlockBinding);
  // SDLGLP2(PFNGLGETPROGRAMRESOURCEINDEXPROC, glGetProgramResourceIndex);


  SDLGLP2(PFNGLCOPYIMAGESUBDATAPROC, glCopyImageSubData);
  SDLGLP2(PFNGLDELETETEXTURESEXTPROC, glDeleteTextures);

  return bValid;
}
void GLContext::setLineWidth(float w) {
  if (getForwardCompatible()==false) {
    BRLogErrorOnce("glLineWidth not supported in core mode.");
  }
  else {
    glLineWidth(w);
  }
}

void GLContext::pushCullFace() {
  GLint cull;
  glGetIntegerv(GL_CULL_FACE, &cull);

  if (_eLastCullFaceStack.size() > MaxStackSize) {
    BRLogWarn("Stack count has been exceeded.  Stack invalid somewhere");
  }
  else {
    _eLastCullFaceStack.push(cull);
  }
}
void GLContext::popCullFace() {
  GLint cull = _eLastCullFaceStack.top();

  if (_eLastCullFaceStack.size() > 0) {
    _eLastCullFaceStack.pop();
  }
  if (_eLastCullFaceStack.size() == 0) {
    _eLastCullFaceStack.push(1);
  }

  if (cull) {
    glEnable(GL_CULL_FACE);
  }
  else {
    glDisable(GL_CULL_FACE);
  }
}
void GLContext::pushBlend() {
  GLint cull;
  glGetIntegerv(GL_BLEND, &cull);

  if (_eLastBlendStack.size() > MaxStackSize) {
    BRLogWarn("Stack count has been exceeded.  Stack invalid somewhere");
  }
  else {
    _eLastBlendStack.push(cull);
  }
}
void GLContext::popBlend() {
  GLint cull = _eLastBlendStack.top();

  if (_eLastBlendStack.size() > 0) {
    _eLastBlendStack.pop();
  }
  if (_eLastBlendStack.size() == 0) {
    _eLastBlendStack.push(1);
  }

  if (cull) {
    glEnable(GL_BLEND);
  }
  else {
    glDisable(GL_BLEND);
  }
}
void GLContext::pushDepthTest() {
  GLint cull;
  glGetIntegerv(GL_DEPTH_TEST, &cull);

  if (_eLastDepthTestStack.size() > MaxStackSize) {
    BRLogWarn("Stack count has been exceeded.  Stack invalid somewhere");
  }
  else {
    _eLastDepthTestStack.push(cull);
  }
}
void GLContext::popDepthTest() {
  GLint cull = _eLastDepthTestStack.top();

  if (_eLastDepthTestStack.size() > 0) {
    _eLastDepthTestStack.pop();
  }
  if (_eLastDepthTestStack.size() == 0) {
    _eLastDepthTestStack.push(1);
  }

  if (cull) {
    glEnable(GL_DEPTH_TEST);
  }
  else {
    glDisable(GL_DEPTH_TEST);
  }
}
//
//void GLContext::beginWin32InlineDebug() {
//#ifdef _WIN32
//#ifdef _DEBUG
//    //**INLINE MODE -- REMOVE FOR RELEASE
//    Gu::getShaderMaker()->shaderBound(nullptr);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//    chkErrDbg();
//
//    mat4 mProj = Gu::getCamera()->getProj();
//    // mProj.transpose();
//    mat4 mView = Gu::getCamera()->getView();
//    // mView.transpose();
//    glMatrixMode(GL_PROJECTION);
//    glLoadMatrixf((GLfloat*)& mProj);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadMatrixf((GLfloat*)& mView);
//#endif
//#endif
//}

void GLContext::enableCullFace(bool enable) {
  if (enable)glEnable(GL_CULL_FACE);
  else glDisable(GL_CULL_FACE);
}
void GLContext::enableBlend(bool enable) {
  if (enable)glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
}

void GLContext::enableDepthTest(bool enable) {
  if (enable)glEnable(GL_DEPTH_TEST);
  else glDisable(GL_DEPTH_TEST);
}


void GLContext::getOpenGLVersion(int& ver, int& subver, int& shad_ver, int& shad_subver) {
  char* tmp;
  string_t glver;
  string_t glslver;
  ver = subver = shad_ver = shad_subver = 0;

  tmp = (char*)glGetString(GL_VERSION);
  if (tmp != nullptr) {
    glver = tmp;
  }
  else {
    glver = "";
  }

  std::vector<string_t> sv;

  if (glver.length() > 0) {
    sv = StringUtil::split(glver, '.');
    if (sv.size() < 2) {
      BRThrowException("Failed to get OpenGL version. Got '" + glver + "'.  Check that you have OpenGL installed on your machine. You may have to update your 'graphics driver'.");
    }
    ver = TypeConv::strToInt(sv[0]);
    subver = TypeConv::strToInt(sv[1]);
  }
  else {
    BRLogError("Failed to get OpenGL version.");
  }
  if (ver > 3) {
    //This will fail if we try to get an OpenGL version greater than what is supported, returning GL 1.1.  Shade is only supported on GL > 2.1.
    tmp = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (tmp != nullptr) {
      glslver = tmp;
    }
    else {
      glslver = "";
    }
    if (glslver.length() > 0) {
      sv = StringUtil::split(glslver, '.');
      if (sv.size() < 2) {
        BRThrowException("Failed to get OpenGL Shade version. Got '" + glslver + "'.  Check that you have OpenGL installed on your machine. You may have to update your 'graphics driver'.");
      }
      shad_ver = TypeConv::strToInt(sv[0]);
      shad_subver = TypeConv::strToInt(sv[1]);
    }
    else {
      BRLogWarn("Failed to get OpenGL Shade version.");
    }
  }

}
bool GLContext::checkOpenGlMinimumVersionInfo(int required_version, int required_subversion) {
  string_t rver = Stz "" + required_version + "." + required_subversion;
  int ver, subver, shad_ver, shad_subver;
  getOpenGLVersion(ver, subver, shad_ver, shad_subver);

  if (ver < required_version || (ver >= required_subversion && subver < required_subversion)) {
    return false;
  }
  else {
    string_t vendor = string_t((char*)glGetString(GL_VENDOR));
    string_t renderer = string_t((char*)glGetString(GL_RENDERER));

    BRLogInfo("\n"
      + "   OpenGL version " + ver + "." + subver + ".\n"
      + "   GLSL version:          " + shad_ver + "." + shad_subver + "\n"
      + "   GPU:         " + renderer + "\n"
      + "   GPU Vendor:  " + vendor + "\n"
    );
  }

  return true;
}



void GLContext::loadCheckProc() {
  //Check that OpenGL initialized successfully by checking a library pointer.
  PFNGLUSEPROGRAMPROC proc = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
  if (proc == nullptr) {
    string_t exep;
    exep += "glUseProgram was not found in your graphics driver.  There can be a few reasons for this:\n";
    exep += ("  1. Your primary graphics card is not correct.  You can set your primary graphics card in Windows.\n");
    exep += ("  2. Your graphics card is outdated.  Consider upgrading.\n");
    exep += ("  3. Your Operating System isn't Windows 7 or above.\n");
    BRThrowException(exep);
  }
}
void GLContext::printHelpfulDebug() {
  int tmp = 0;
  SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &tmp);
  BRLogInfo("SDL_GL_DOUBLEBUFFER: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &tmp);
  BRLogInfo("SDL_GL_BUFFER_SIZE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &tmp);

  BRLogInfo("SDL_GL_DEPTH_SIZE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &tmp);
  BRLogInfo("SDL_GL_STENCIL_SIZE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &tmp);
  BRLogInfo("SDL_GL_ACCELERATED_VISUAL: " + tmp);

  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &tmp);
  BRLogInfo("SDL_GL_CONTEXT_PROFILE_MASK: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, &tmp);
  BRLogInfo("SDL_GL_SHARE_WITH_CURRENT_CONTEXT: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &tmp);
  BRLogInfo("SDL_GL_CONTEXT_FLAGS: " + tmp);

  SDL_GL_GetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, &tmp);
  BRLogInfo("SDL_GL_FRAMEBUFFER_SRGB_CAPABLE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &tmp);
  BRLogInfo("SDL_GL_MULTISAMPLESAMPLES: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &tmp);
  BRLogInfo("SDL_GL_MULTISAMPLEBUFFERS: " + tmp);

  SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &tmp);
  BRLogInfo("SDL_GL_RED_SIZE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &tmp);
  BRLogInfo("SDL_GL_GREEN_SIZE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &tmp);
  BRLogInfo("SDL_GL_BLUE_SIZE: " + tmp);
  SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &tmp);
  BRLogInfo("SDL_GL_ALPHA_SIZE: " + tmp);
}

void GLContext::setWindowAndOpenGLFlags(std::shared_ptr<GLProfile> prof) {
  //Attribs
  SDL_GL_ResetAttributes();
  SDLUtils::checkSDLErr();

  //We want SRGB in the final render, so this should be requested.
  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, prof->_bSRGB);
  SDLUtils::checkSDLErr();

  //Context sharing will be necessary with multiple-window rendering (we are required to create 1 context per window)
  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
  SDLUtils::checkSDLErr();

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  //Not sure, imagine we'd use our own buffer blending to create a VR scene.
  //SDL_GL_SetAttribute(SDL_GL_STEREO, _pGlState->gl_stereo);

  if (prof->_iMinVersion > 0 && prof->_iMinSubVersion > 0) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, prof->_iMinVersion);
    SDLUtils::checkSDLErr();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, prof->_iMinSubVersion);
    SDLUtils::checkSDLErr();
  }

  //Desktop Debug = Compatibility, Runtime = Core, Phone = ES
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, prof->_iProfile);
  SDLUtils::checkSDLErr();

  int debug_flag = 0;
#ifdef _DEBUG
  debug_flag = SDL_GL_CONTEXT_DEBUG_FLAG;
#endif

  //Only in GL 3.0 (apparently)
  //Forward compatible deprecates old funcionality for a gain in speed (apparently)
  //https://wiki.libsdl.org/SDL_GLcontextFlag#SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, debug_flag | (prof->_bForwardCompatible ? SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG : 0));
  SDLUtils::checkSDLErr();

  //Depth size is finicky. Sometimes it will only give us 16 bits. Trying to set stencil to zero MIGHT help
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, prof->_iDepthBits);
  SDLUtils::checkSDLErr();
  //Attempt to zero out the stencil buffer to request a 32b depth.

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDLUtils::checkSDLErr();

  //Multisampling
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, prof->_iMSAASamples);
  SDLUtils::checkSDLErr();
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, prof->_iMSAABuffers);
  SDLUtils::checkSDLErr();

}










}//ns Game
