#include "../base/GlobalIncludes.h"
#include "../base/Logger.h"
#include "../base/GLContext.h"
#include "../base/Img32.h"
#include "../base/Gu.h"
#include "../base/Allocator.h"
#include "../base/BinaryFile.h"
#include "../gfx/TexCache.h"
#include "../gfx/ShaderBase.h"
#include "../gfx/Texture2DSpec.h"
#include "../gfx/RenderUtils.h"

namespace BR2 {
Texture2DSpec::Texture2DSpec(string_t name, std::shared_ptr<GLContext> ct) : _pContext(ct) {
  _strName = name;
}
Texture2DSpec::Texture2DSpec(string_t name, string_t loc, std::shared_ptr<GLContext> ctx, bool bRepeatU, bool bRepeatV) : _pContext(ctx) {
  _strName = name;
  load(loc, bRepeatU, bRepeatV);
}
Texture2DSpec::Texture2DSpec(string_t name, const std::shared_ptr<Img32>sp, std::shared_ptr<GLContext> ctx, TexFilter::e eFilter) : _pContext(ctx) {
  _strName = name;
  create((unsigned char*)sp->getData()->ptr(), sp->getWidth(), sp->getHeight(), false, false, false);
  setFilter(eFilter);
}
Texture2DSpec::Texture2DSpec(string_t name, std::shared_ptr<GLContext> ctx, unsigned char* texData, int iWidth, int iHeight, bool mipmaps) : _pContext(ctx) {
  _strName = name;
  create(texData, iWidth, iHeight, mipmaps, false, false);
}
Texture2DSpec::~Texture2DSpec() {
  dispose();
}
void Texture2DSpec::load(string_t imgLoc, bool bRepeatU, bool bRepeatV) {
  _strLocation = imgLoc;
  std::shared_ptr<Img32> sp = Gu::loadImage(imgLoc);
  if (sp != nullptr) {
    create((unsigned char*)(sp->getData()->ptr()), sp->getWidth(), sp->getHeight(), false, bRepeatU, bRepeatV);
    Gu::freeImage(sp);
  }
  else {
    _bLoadFailed = true;
  }
}
void Texture2DSpec::calculateTextureFormat() {
  //All textures are 32 bit
  //AssertOrThrow2(getBytesPerPixel() == 4);
  _eTextureFormat = (GL_RGBA);
  _eTextureMipmapFormat = GL_RGBA8;
}
bool Texture2DSpec::bind(TextureChannel::e eChannel, std::shared_ptr<ShaderBase> pShader, bool bIgnoreIfNotFound) {
  if (getGlId() == 0) {
    //https://stackoverflow.com/questions/1108589/is-0-a-valid-opengl-texture-id
    BRLogErrorCycle("Texture was not created on the GPU.");
    Gu::debugBreak();
    return false;
  }
  else {
    std::dynamic_pointer_cast<GLContext>(Gu::getCoreContext())->glActiveTexture(GL_TEXTURE0 + eChannel);
    glBindTexture(GL_TEXTURE_2D, getGlId());

    if (pShader != nullptr) {
      //Some cases, we just need to call glBindTexture..
      //But this should almost always be set.
      pShader->setTextureUf(eChannel, bIgnoreIfNotFound);
    }
  }
  return true;
}
void Texture2DSpec::unbind() {
  //  if (_iChannel == 0) {
  _pContext->glActiveTexture(GL_TEXTURE0);
  //}
  //else {
  //    BroThrowNotImplementedException();
  //}
  glBindTexture(GL_TEXTURE_2D, 0);
}
int32_t Texture2DSpec::generateMipmapLevels() {
  // - Create log2 mipmaps
  int numMipMaps = 0;
  int x = MathUtils::brMax(getWidth(), getHeight());
  for (; x; x = x >> 1) {
    numMipMaps++;
  }
  return numMipMaps;
}
// - Make the texture known to OpenGL
void Texture2DSpec::create(unsigned char* imageData, uint32_t w, uint32_t h, bool genMipmaps, bool bRepeatU, bool bRepeatV) {
  _iWidth = w;
  _iHeight = h;
  if (h != 0.0f) {
    _fSizeRatio = (float)w / (float)h;
  }
  // Bind texture
  _pContext->glActiveTexture(GL_TEXTURE0);
  _pContext->chkErrRt();
  glGenTextures(1, &_glId);
  _pContext->chkErrRt();
  glBindTexture(GL_TEXTURE_2D, _glId);
  _pContext->chkErrRt();

  //Calc format
  calculateTextureFormat();
  _pContext->chkErrRt();

  _bHasMipmaps = genMipmaps;
  _bRepeatU = bRepeatU;
  _bRepeatV = bRepeatV;

  //Specify storage mode
  if (genMipmaps) {
    //Create nice mipmaps. This does... nothing?
    //**2016 - this function FAILS in most profiles.
    // It's probably because it's legacy **DEPRECATED in 3.0
    //glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    _pContext->chkErrRt();

    int numMipMaps = generateMipmapLevels();
    _pContext->glTexStorage2D(GL_TEXTURE_2D, numMipMaps, _eTextureMipmapFormat, getWidth(), getHeight());
    _pContext->chkErrRt();
  }
  else {
    _pContext->glTexStorage2D(GL_TEXTURE_2D, 1, _eTextureMipmapFormat, getWidth(), getHeight());
    _pContext->chkErrRt();
  }

  // Copy texture date
  glTexSubImage2D(
    GL_TEXTURE_2D,
    0, 0, 0,        //level, x, y
    w,
    h,
    _eTextureFormat,                //format
    GL_UNSIGNED_BYTE,    //type
    (void*)imageData    //pixels
  );

  _pContext->chkErrRt();

  // Specify Parameters
  if (bRepeatU) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  }
  if (bRepeatV) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  if (genMipmaps) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
  else {
    //Default to nearest filtering * this is needed because the
    //texture atlas shows edges with linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  //if(Gu::GetEngineDisplayParams()->getEnableAnisotropicFiltering())
  //{
  //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Gu::GetEngineDisplayParams()->getTextureAnisotropyLevel());
  //    Gu::checkErrors();
  //}

  // - Generate the mipmaps
  if (genMipmaps) {
    //GL 3.0 mipmaps
    _pContext->glGenerateMipmap(GL_TEXTURE_2D);
    _pContext->chkErrRt();
  }

  //Unbind so we don't modify it
  glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture2DSpec::dispose() {
  glDeleteTextures(1, &_glId);
}
void Texture2DSpec::setWrapU(TexWrap::e wrap) {
  bind(TextureChannel::e::Channel0, nullptr);
  if (wrap == TexWrap::e::Clamp) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  }
  else if (wrap == TexWrap::e::Repeat) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  }
  else {
    BRThrowNotImplementedException();
  }
  unbind();
}
void Texture2DSpec::setWrapV(TexWrap::e wrap) {
  bind(TextureChannel::e::Channel0, nullptr);
  if (wrap == TexWrap::e::Clamp) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  else if (wrap == TexWrap::e::Repeat) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else {
    BRThrowNotImplementedException();
  }
  unbind();
}
void Texture2DSpec::setFilter(TexFilter::e filter) {
  _eFilter = filter;
  bind(TextureChannel::e::Channel0, nullptr);
  Gu::checkErrorsDbg();

  if (filter == TexFilter::e::Linear) {
    if (_bHasMipmaps) {
      _pContext->glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      //Note: GL_GENERATE_MIPMAP is deprecated.
    }
    else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      //GL_GENERATE_MIPMAP is deprecated.
    }
  }
  else if (filter == TexFilter::e::Nearest) {
    if (_bHasMipmaps) {
      _pContext->glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    }
    else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
  }
  else {
    BRThrowNotImplementedException();
  }
  _pContext->chkErrRt();

  unbind();
}
bool Texture2DSpec::getTextureDataFromGpu(std::shared_ptr<Img32> __out_ image) {
  return RenderUtils::getTextureDataFromGpu(image, _glId, GL_TEXTURE_2D);
}
void Texture2DSpec::serialize(std::shared_ptr<BinaryFile> fb) {
  std::shared_ptr<Img32> img = std::make_shared<Img32>();
  getTextureDataFromGpu(img);

  fb->writeVersion();
  fb->writeString(std::move(_strName));
  fb->writeString(std::move(_strLocation));
  fb->writeUint32(getWidth());
  fb->writeUint32(getHeight());
  fb->writeBool(std::move(_bHasMipmaps));
  fb->writeBool(std::move(_bRepeatU));
  fb->writeBool(std::move(_bRepeatV));

  fb->writeUint32(std::move((uint32_t)img->getData()->byteSize()));
  fb->write((const char*)img->getData()->ptr(), img->getData()->byteSize());
}
void Texture2DSpec::deserialize(std::shared_ptr<BinaryFile> fb) {
  fb->readVersion();
  fb->readString(_strName);
  fb->readString(_strLocation);
  uint32_t iWidth;
  fb->readUint32(iWidth);
  uint32_t iHeight;
  fb->readUint32(iHeight);
  bool bHasMipmaps;
  fb->readBool(bHasMipmaps);
  bool bRepeatU;
  fb->readBool(bRepeatU);
  bool bRepeatV;
  fb->readBool(bRepeatV);

  uint32_t dataSize;
  fb->readUint32(dataSize);

  unsigned char* data = new unsigned char[dataSize];
  fb->read((const char*)data, dataSize);

  create(data, iWidth, iHeight, bHasMipmaps, bRepeatU, bRepeatV);
}
}//ns game