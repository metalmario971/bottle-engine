#include "../base/DiskFile.h"
#include "../base/StringUtil.h"
#include "../base/FileSystem.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <fstream>

namespace BR2 {
class DiskFile_Internal {
public:
  size_t off=0; // - offset in the file
  DiskLoc loc;  // - location
  std::fstream pStream; // - The saved file stream.
  size_t filesize;
};
DiskFile::DiskFile() {
  _internal = std::make_unique<DiskFile_Internal>();
}
DiskFile::~DiskFile() {
  _internal=nullptr;
}
size_t DiskFile::getPointerOffset() { return _internal->off; }
size_t DiskFile::getFileSize() const { return _internal->filesize; }
bool DiskFile::checkEOF() {
  if (state == file_opened_read || state == file_opened_write) {
    if (_internal->pStream.eof()) {
      return true;
    }
    else {
      return false;
    }
  }
  state = file_error;
  return false;
}

/**
*   @fn create()
*   @brief Create a new file, just creates the file and does not write a thing.
*   NOTE: any previous file will be erased!

*
*    TODO: Remove this function.  It is superfluous and does nothing important.
*/
RetCode DiskFile::create(string_t szloc, size_t offset) {
  std::fstream fs;

  _internal->off = offset;

  state = file_error; // set before returning

  fs.clear();

  fs.open(szloc.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
  //CheckOsErrorsDbg();
  if (!fs.good()) {
    fs.close();
    if (fs.rdstate() | fs.badbit)
      return FILE_CREATEFAILED;
    if (fs.rdstate() | fs.eofbit)
      return FILE_CREATEFAILED;
    if (fs.rdstate() | fs.failbit)
      return FILE_CREATEFAILED;
  }

  state = file_created;
  fs.close();

  return GR_OK;
}
/**
*    @fn openForRead()
*    @brief Open file for read storing the pointer to the file.
*/
RetCode DiskFile::openForRead(DiskLoc& szloc) {
  string_t l;

  if ((state != file_closed) && (state != file_created) && (state != file_empty)) {
    BRThrowException("File was in a bad state.");
  }

  if (!szloc.length()) {
    l = _internal->loc;
  }
  else {
    l = szloc;
  }

  //CheckOsErrorsDbg();
  // std::fstream stream;
  _internal->pStream.clear();
  //CheckOsErrorsDbg();
  _internal->pStream.open(l.c_str(), std::ios::in | std::ios::binary);
  //OperatingSystem::suppressError(OperatingSystem::FileNotFound, false);//In windows error 2 is file not found.
  //OperatingSystem::suppressError(OperatingSystem::PathNotFound, false);//In windows error 2 is file not found.
  //CheckOsErrorsDbg();

  //Globals::getLogger()->logInfo(t_string("Loading ")+l);
  if (!_internal->pStream.good()) {
    _internal->pStream.close();
    BRLogError("Could not open file '" + szloc + "' for read.");
    return GR_FILE_NOT_FOUND_ON_DISK;
    //BroThrowException("File does not exist at the location \'", szloc, "\'");
  }

  size_t size;
  _internal->pStream.seekg(0, std::ios::end);
  size = (size_t)_internal->pStream.tellg();
  _internal->pStream.seekg(0, std::ios::beg);

  _internal->filesize = size;

  if (_internal->off > 0) {
    AssertOrThrow2(_internal->off < size);
    _internal->pStream.seekg((long)_internal->off, std::ios::beg);
  }

  state = file_opened_read;
  return GR_OK;
}
/**
*    @fn openForWrite()
*    @brief Open file for write storing state in pointer.
*/
RetCode DiskFile::openForWrite(DiskLoc& szloc, FileWriteMode::e mode) {
  string_t l;

  if ((state != file_closed) && (state != file_created) && (state != file_empty)) {
    BRThrowException("File " + szloc + " was in a bad state.");
  }

  if (!szloc.length()) {
    l = _internal->loc;
  }
  else l = szloc;

  // - Get Write Mode
  std::ios::_Openmode appOrTrunc;
  if (mode == FileWriteMode::Truncate) {
    appOrTrunc = std::ios::trunc;
    //THis was causing windows to generate error 183
    if (FileSystem::fileExists((string_t)l)) {
      FileSystem::deleteFile((string_t)l);
    }
  }
  else if (mode == FileWriteMode::Append) {
    appOrTrunc = std::ios::app;
  }
  else {
    throw new NotImplementedException();
  }


  // - Open File
  _internal->pStream.open(l.c_str(), std::ios::out | std::ios::binary | appOrTrunc);

  //CheckOsErrorsDbg();

  if (!_internal->pStream.good()) {
    _internal->pStream.close();
    BRLogError("File " + szloc + " does not exist.");
    return GR_FILE_NOT_FOUND_ON_DISK;
    //BroThrowException();
  }

  size_t size;
  _internal->pStream.seekp(0, std::ios::end);
  size = (size_t)_internal->pStream.tellp();
  _internal->pStream.seekp(0, std::ios::beg);

  _internal->filesize = size;

  if (_internal->off > 0) {
    AssertOrThrow2(_internal->off < size);
    _internal->pStream.seekp((long)_internal->off, std::ios::end);
  }

  state = file_opened_write;

  return GR_OK;
}
RetCode DiskFile::write(const char* bytes, size_t len, size_t offset) {
  AssertOrThrow2(state == file_opened_write);

  if (offset != (size_t)-1) {
    BRThrowException("Offset is an invalid parameter in DiskFile - and is not implemented.");
  }

  _internal->pStream.write(bytes, (std::streamsize)len);
  return GR_OK;
}
/**
*   @fn writeStr
*   @brief Writes a string like a pascal string, with [length(32bit)][str...]
*/
RetCode DiskFile::writeStrWithLen(const string_t& str) {
  AssertOrThrow2(state == file_opened_write);

  uint32_t len = (int32_t)str.length();
  RetCode f;
  if ((f = write((char*)&len, sizeof(uint32_t))) != GR_OK) {
    return f;
  }
  f = write(str.c_str(), str.length());
  return f;
}
/**
*    @fn
*    @brief Writes string directly as plain byte text.
*/
RetCode DiskFile::writeStrText(const string_t& str) {
  AssertOrThrow2(state == file_opened_write);

  uint32_t len = (int32_t)str.length();
  RetCode f;
  f = write(str.c_str(), str.length());
  return f;
}
/**
*   @fn readStr
*   @brief Reads a string like a pascal string, with [length(32bit)][str...]
*/
RetCode DiskFile::readStr(char* buf, size_t bufLen) {
  AssertOrThrow2(state == file_opened_read);

  uint32_t len;
  RetCode f;
  if ((f = read((char*)&len, sizeof(uint32_t))) != GR_OK) {
    return f;
  }
  if ((len + 1) > (int32_t)bufLen) {
    return FILE_BUFFEROVERFLOW;
  }
  f = read(buf, len);
  buf[len] = '\0';

  return f;
}
/**
*    @fn read()
*    @brief Read the file in.
*/
RetCode DiskFile::read(char* buf, size_t len, size_t buflen, size_t offset) {
  AssertOrThrow2(state == file_opened_read);

  if (offset != (size_t)-1) {
    BRThrowException("Offset is an invalid parameter in DiskFile - and is not implemented.");
  }

  if (buflen != -1) {
    AssertOrThrow2((size_t)len < buflen);
  }

  _internal->pStream.read(buf, (std::streamsize)len);
  return GR_OK;
}
/**
*    @fn read()
*    @brief Read in some data.
*/
//RetCode DiskFile::read(char* buf, size_t len, size_t offset, size_t buflen) 
//{
//    if(buflen != -1)
//        AssertOrThrow2((size_t)len <= buflen);
//
//    pStream.seekg(0, std::ios::beg);
//    if(offset > 0)
//        pStream.seekg((long)offset, std::ios::end);
//
//    if(!pStream.good())
//    {
//        close();
//        return FILE_EOF;
//    }
//
//    pStream.read(buf, (std::streamsize)len);
//    return GR_OK;
//}
/**
*   Read the file until a delimiter is reached. **ALSO READS THE DELIM**
*   @return FILE_EOF: The file is at eof.
*       FILE_ABRUPTEOF: We read past the end of the file
*       FLIE_BUFFEROVERFLOW: The supplied buffer was too small.
*       GR_OK: success
*/
RetCode DiskFile::readTo(char* buf, const string_t& delims, size_t buflen) {
  AssertOrThrow2(state == file_opened_read);

#ifdef _DEBUG   
  AssertOrThrow2(delims.length() > 0);
#else
  if (delims.length() == 0)
    throw new Exception("DiskFile::readTo, no delimiters.", __LINE__, __FILE__);
#endif

  if (checkEOF())
    return FILE_EOF;

  char lastRead;
  size_t count = 0;
  do {
    if (checkEOF()) {
      setState(file_error);
      return FILE_ABRUPTEOF;
    }
    count++;
    if (count >= buflen) {
      setState(file_error);
      return FILE_BUFFEROVERFLOW;
    }
    _internal->pStream.read(buf + count, 1);
    lastRead = *(buf + count);
  } while (!StringUtil::isDelim(lastRead, delims));

  return GR_OK;
}

void DiskFile::close() {
  if (_internal->pStream.is_open()) {
    _internal->pStream.close();
    _internal->pStream.clear(std::ios::goodbit);
  }
  state = file_closed;
}
/**
*    @fn getWriteStream
*    @brief Return the read/write streams
*    @param newStream The stream to return.
*/
//RetCode DiskFile::getWriteStream(std::fstream& newStream) {
//
//    if(newStream.is_open())
//        return FILE_HANDLEISOPEN;
//
//    newStream.open(loc.c_str(), std::ios::out | std::ios::binary);
//    //CheckOsErrorsDbg();
//
//    if(!newStream.good()) {
//        newStream.close();
//        return FILE_OPENFAILED;//throw new Exception(Game::EX_FILE_OPEN_FAILED,"FILE_OPEN_FAILED",__LINE__,__FILE__);
//    }
//
//    size_t size;
//    newStream.seekp(0, std::ios::end);
//    size = (size_t)newStream.tellp();
//    newStream.seekp(0, std::ios::beg);
//
//    AssertOrThrow2(off < size);
//
//    newStream.seekp((long)this->off, std::ios::end);
//
//    return GR_OK;
//}
/**
*    @fn getReadStream
*    @brief Return the read/write streams
*    @param newStream The stream to return.
*/
RetCode DiskFile::getReadStream(std::fstream& newStream) {

  if (newStream.is_open()) {
    return FILE_HANDLEISOPEN;
  }

  newStream.open(_internal->loc.c_str(), std::ios::in | std::ios::binary);
  //CheckOsErrorsDbg();
  if (!newStream.good()) {
    newStream.close();
    return FILE_OPENFAILED;//throw new Exception(Game::EX_FILE_OPEN_FAILED,"FILE_OPEN_FAILED",__LINE__,__FILE__);
  }

  size_t size;
  newStream.seekg(0, std::ios::end);
  size = (size_t)newStream.tellg();
  newStream.seekg(0, std::ios::beg);

  AssertOrThrow2(_internal->off < size);

  newStream.seekg(_internal->off, std::ios::end);

  return GR_OK;
}
RetCode DiskFile::readAllBytes(string_t loc, Allocator<char>& __out_ outBuf) {
  if (!FileSystem::fileExists(loc)) {
    BRLogError("File " + loc + " did not exist.");
    return GR_FILE_NOT_FOUND_ON_DISK;
  }
  DiskFile df;
  RetCode rc;
  rc = df.openForRead(loc);
  if (rc != GR_OK) {
    BRLogWarn("Failed to read file " + loc);
    return rc;
  }
  size_t sz = df.getFileSize();
  outBuf.realloca(sz + 1); // +1 to put the /0 at the end
  rc = df.read(outBuf.ptr(), sz);
  if (rc != GR_OK) {
    BRLogWarn("Failed to read file (2) " + loc);
    return rc;
  }
  outBuf.at(sz - 1) = '\0';
  df.close();

  return GR_OK;
}
RetCode DiskFile::writeAllBytes(string_t loc, Allocator<char>& __out_ outBuf) {
  //Unnecessary to create file.  We truncate it later.
  //if (!FileSystem::fileExists(loc)) {
  //    t_string strPath = FileSystem::getPathFromPath(loc);
  //    FileSystem::createDirectoryRecursive(strPath);
  //    FileSystem::createFile(loc);
  //}
  RetCode ret;
  DiskFile df;
  ret = df.openForWrite(loc);
  if (ret != GR_OK) {
    BRLogWarn("Failed to write file " + loc);
    return ret;
  }
  ret = df.write(outBuf.ptr(), outBuf.byteSize());
  if (ret != GR_OK) {
    BRLogWarn("Failed to write file (2) " + loc);
    return ret;
  }
  df.close();
  return GR_OK;
}


}//ns game
