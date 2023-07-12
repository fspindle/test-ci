#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdexcept>
#include <sstream>
#include <exception>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
#include <dirent.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <direct.h>
#include <windows.h>
#endif
#if !defined(_WIN32)
#ifdef __ANDROID__
// Like IOS, wordexp.cpp is not defined for Android
#else
#include <wordexp.h>
#endif
#endif

#if defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <TargetConditionals.h>             // To detect OSX or IOS using TARGET_OS_IOS macro
#endif

#ifndef PATH_MAX
#ifdef _MAX_PATH
#define PATH_MAX _MAX_PATH
#else
#define PATH_MAX 1024
#endif
#endif

static const char separator =
#if defined(_WIN32)
'\\';
#else
'/';
#endif

class vpException : public std::exception
{
protected:
  //! Contains the error code, see the errorCodeEnum table for details.
  int code;

  //! Contains an error message (can be empty)
  std::string message;

  //! Set the message container
  void setMessage(const char *format, va_list args);

  //!  forbid the empty constructor (protected)
  vpException() : code(notInitialized), message("") { };

public:
  enum generalExceptionEnum
  {
    memoryAllocationError,       //!< Memory allocation error
    memoryFreeError,             //!< Memory free error
    functionNotImplementedError, //!< Function not implemented
    ioError,                     //!< I/O error
    cannotUseConstructorError,   //!< Contructor error
    notImplementedError,         //!< Not implemented
    divideByZeroError,           //!< Division by zero
    dimensionError,              //!< Bad dimension
    fatalError,                  //!< Fatal error
    badValue,                    //!< Used to indicate that a value is not in the allowed range.
    notInitialized               //!< Used to indicate that a parameter is not initialized.
  };

  vpException(int code, const char *format, va_list args);
  vpException(int code, const char *format, ...);
  vpException(int code, const std::string &msg);
  explicit vpException(int code);

  /*!
    Basic destructor. Do nothing but implemented to fit the inheritance from
    std::exception
  */
#if VISP_CXX_STANDARD > VISP_CXX_STANDARD_98
  virtual ~vpException() { }
#else
  virtual ~vpException() throw() { }
#endif

  /** @name Inherited functionalities from vpException */
  //@{
  //! Send the object code.
  int getCode() const;

  //! Send a reference (constant) related the error message (can be empty).
  const std::string &getStringMessage() const;
  //! send a pointer on the array of  \e char related to the error string.
  //! Cannot be  \e NULL.
  const char *getMessage() const;
  //@}

  //! Print the error structure.
  friend std::ostream &operator<<(std::ostream &os, const vpException &art);

  const char *what() const throw();
};

vpException::vpException(int id) : code(id), message() { }

vpException::vpException(int id, const std::string &msg) : code(id), message(msg) { }

vpException::vpException(int id, const char *format, ...) : code(id), message()
{
  va_list args;
  va_start(args, format);
  setMessage(format, args);
  va_end(args);
}

vpException::vpException(int id, const char *format, va_list args) : code(id), message() { setMessage(format, args); }
/* ------------------------------------------------------------------------ */
/* --- DESTRUCTORS -------------------------------------------------------- */
/* ------------------------------------------------------------------------ */

/* Destructeur par default suffisant. */
// vpException::
// ~vpException (void)
// {
// }

void vpException::setMessage(const char *format, va_list args)
{
  char buffer[1024];
  vsnprintf(buffer, 1024, format, args);
  std::string msg(buffer);
  message = msg;
}

/* ------------------------------------------------------------------------ */
/* --- ACCESSORS ---------------------------------------------------------- */
/* ------------------------------------------------------------------------ */

const char *vpException::getMessage() const { return (this->message).c_str(); }

const std::string &vpException::getStringMessage() const { return this->message; }

int vpException::getCode() const { return this->code; }

/*!
  Overloading of the what() method of std::exception to return the vpException
  message.

  \return pointer on the array of  \e char related to the error string.
*/
const char *vpException::what() const throw() { return (this->message).c_str(); }

/* -------------------------------------------------------------------------
 */
 /* --- MODIFIORS -----------------------------------------------------------
  */
  /* -------------------------------------------------------------------------
   */

   /* -------------------------------------------------------------------------
    */
    /* --- OP << ---------------------------------------------------------------
     */
     /* -------------------------------------------------------------------------
      */

std::ostream &operator<<(std::ostream &os, const vpException &error)
{
  os << "Error [" << error.code << "]:\t" << error.message << std::endl;

  return os;
}

class vpIoException : public vpException
{
public:
  /*!
  \brief Lists the possible error than can be emitted while calling
  vpIo member.
 */
  enum error
  {
    invalidDirectoryName, /*! Directory name is invalid. */
    cantCreateDirectory,  /*! Unable to create a directory. */
    cantGetUserName,      /*! User name is not available. */
    cantGetenv            /*! Cannot get environment variable value. */
  };

public:
  vpIoException(int id, const char *format, ...)
  {
    this->code = id;
    va_list args;
    va_start(args, format);
    setMessage(format, args);
    va_end(args);
  }
  vpIoException(int id, const std::string &msg) : vpException(id, msg) { ; }
  explicit vpIoException(int id) : vpException(id) { ; }
};


#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#if (TARGET_OS_IOS == 0) && !defined(__ANDROID__)
void replaceAll(std::string &str, const std::string &search, const std::string &replace)
{
  size_t start_pos = 0;
  while ((start_pos = str.find(search, start_pos)) != std::string::npos) {
    str.replace(start_pos, search.length(), replace);
    start_pos += replace.length(); // Handles case where 'replace' is a
    // substring of 'search'
  }
}
#endif
#endif

std::string path(const std::string &pathname)
{
  std::string path(pathname);

#if defined(_WIN32)
  for (unsigned int i = 0; i < path.length(); i++)
    if (path[i] == '/')
      path[i] = '\\';
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
  for (unsigned int i = 0; i < path.length(); i++)
    if (path[i] == '\\')
      path[i] = '/';
#if TARGET_OS_IOS == 0 // The following code is not working on iOS and android since
  // wordexp() is not available
#ifdef __ANDROID__
// Do nothing
#else
  wordexp_t exp_result;

  // escape quote character
  replaceAll(path, "'", "'\\''");
  // add quotes to handle special characters like parentheses and spaces
  wordexp(std::string("'" + path + "'").c_str(), &exp_result, 0);
  path = exp_result.we_wordc == 1 ? exp_result.we_wordv[0] : "";
  wordfree(&exp_result);
#endif
#endif
#endif

  return path;
}


bool checkFilename(const std::string &filename)
{
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  struct stat stbuf;
#elif defined(_WIN32)
  struct _stat stbuf;
#endif

  if (filename.empty()) {
    return false;
  }

  std::string _filename = path(filename);
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  if (stat(_filename.c_str(), &stbuf) != 0)
#elif defined(_WIN32)
  if (_stat(_filename.c_str(), &stbuf) != 0)
#endif
  {
    return false;
  }
  if ((stbuf.st_mode & S_IFREG) == 0) {
    return false;
  }
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  if ((stbuf.st_mode & S_IRUSR) == 0)
#elif defined(_WIN32)
  if ((stbuf.st_mode & S_IREAD) == 0)
#endif
  {
    return false;
  }
  return true;
}

bool checkDirectory(const std::string &dirname)
{
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  struct stat stbuf;
#elif defined(_WIN32) && defined(__MINGW32__)
  struct stat stbuf;
#elif defined(_WIN32)
  struct _stat stbuf;
#endif

  if (dirname.empty()) {
    return false;
  }

  std::string _dirname = path(dirname);

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  if (stat(_dirname.c_str(), &stbuf) != 0)
#elif defined(_WIN32) && defined(__MINGW32__)
  // Remove trailing separator character if any
  // AppVeyor: Windows 6.3.9600 AMD64 ; C:/MinGW/bin/g++.exe  (ver 5.3.0) ;
  // GNU Make 3.82.90 Built for i686-pc-mingw32
  if (_dirname.at(_dirname.size() - 1) == separator)
    _dirname = _dirname.substr(0, _dirname.size() - 1);
  if (stat(_dirname.c_str(), &stbuf) != 0)
#elif defined(_WIN32)
  if (_stat(_dirname.c_str(), &stbuf) != 0)
#endif
  {
    return false;
  }
#if defined(_WIN32) || (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
  if ((stbuf.st_mode & S_IFDIR) == 0)
#endif
  {
    return false;
  }
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  if ((stbuf.st_mode & S_IWUSR) == 0)
#elif defined(_WIN32)
  if ((stbuf.st_mode & S_IWRITE) == 0)
#endif
  {
    return false;
  }
  return true;
}

bool checkFifo(const std::string &fifofilename)
{
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  struct stat stbuf;

  std::string _filename = path(fifofilename);
  if (stat(_filename.c_str(), &stbuf) != 0) {
    return false;
  }
  if ((stbuf.st_mode & S_IFIFO) == 0) {
    return false;
  }
  if ((stbuf.st_mode & S_IRUSR) == 0)

  {
    return false;
  }
  return true;
#elif defined(_WIN32)
  (void)fifofilename;
  throw(vpIoException(vpIoException::notImplementedError, "Fifo files are not supported on Windows platforms."));
#endif
}

bool remove(const std::string &file_or_dir)
{
  // Check if we have to consider a file or a directory
  if (checkFilename(file_or_dir)
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
    || checkFifo(std::string(file_or_dir))
#endif
    ) {
    // std::cout << "remove file: " << file_or_dir << std::endl;
    if (::remove(file_or_dir.c_str()) != 0)
      return false;
    else
      return true;
  }
  else if (checkDirectory(file_or_dir)) {
    // std::cout << "remove directory: " << file_or_dir << std::endl;
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
#if TARGET_OS_IOS == 0 // The following code is not working on iOS since
                       // wordexp() is not available
    std::stringstream cmd;
    cmd << "rm -rf \"";
    cmd << file_or_dir;
    cmd << "\"";
    int ret = system(cmd.str().c_str());
    if (ret) {
    }; // to avoid a warning
    // std::cout << cmd << " return value: " << ret << std::endl;
    return true;
#else
    throw(vpIoException(vpException::fatalError, "Cannot remove %s: not implemented on iOS Platform",
                        file_or_dir.c_str()));
#endif
#elif defined(_WIN32)
#if (!defined(WINRT))
    std::stringstream cmd;
    cmd << "rmdir /S /Q ";
    cmd << path(file_or_dir);
    cmd << "\"";
    int ret = system(cmd.str().c_str());
    if (ret) {
    }; // to avoid a warning
    // std::cout << cmd << " return value: " << ret << std::endl;
    return true;
#else
    throw(vpIoException(vpException::fatalError, "Cannot remove %s: not implemented on Universal Windows Platform",
                        file_or_dir.c_str()));
#endif
#endif
  }
  else {
    std::cout << "Cannot remove: " << file_or_dir << std::endl;
    return false;
  }
}

void getUserName(std::string &username)
{
  // With MinGW, UNIX and _WIN32 are defined
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  // Get the user name.
  char *_username = ::getenv("LOGNAME");
  if (!_username) {
    username = "unknown";
  }
  else {
    username = _username;
  }
#elif defined(_WIN32)
#if (!defined(WINRT))
  unsigned int info_buffer_size = 1024;
  TCHAR *infoBuf = new TCHAR[info_buffer_size];
  DWORD bufCharCount = (DWORD)info_buffer_size;
  // Get the user name.
  if (!GetUserName(infoBuf, &bufCharCount)) {
    username = "unknown";
  }
  else {
    username = infoBuf;
  }
  delete[] infoBuf;
#else
  // Universal platform
  username = "unknown";
#endif
#else
  username = "unknown";
#endif
}

int mkdir_p(const char *path, int mode)
{
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = strlen(path);
  char _path[PATH_MAX];
  const char sep = separator;

  std::fill(_path, _path + PATH_MAX, 0);

  errno = 0;
  if (len > sizeof(_path) - 1) {
    errno = ENAMETOOLONG;
    return -1;
  }
  /* Copy string so its mutable */
  strcpy(_path, path);

  /* Iterate over the string */
  for (char *p = _path + 1; *p; p++) { // path cannot be empty
    if (*p == sep) {
      /* Temporarily truncate */
      *p = '\0';

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
      if (mkdir(_path, static_cast<mode_t>(mode)) != 0)
#elif defined(_WIN32)
      (void)mode; // var not used
      std::cout << "1 in mkdir_p() _path: " << _path << std::endl;
      if (!checkDirectory(_path) && _mkdir(_path) != 0)
#endif
      {
        if (errno != EEXIST) {
          std::cout << "1 in mkdir_p() return -1" << std::endl;
          return -1;
        }
      }
      *p = sep;
    }
  }

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
  if (mkdir(_path, static_cast<mode_t>(mode)) != 0)
#elif defined(_WIN32)
  std::cout << "2 in mkdir_p() _path: " << _path << std::endl;

  if (_mkdir(_path) != 0)
#endif
  {
    if (errno != EEXIST) {
      std::cout << "2 in mkdir_p() return -1" << std::endl;
      return -1;
    }
  }

  return 0;
}

void makeDirectory(const std::string &dirname)
{
#if ((!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))) && !defined(_WIN32)
  std::cerr << "Unsupported platform for vpIoTools::makeDirectory()!" << std::endl;
  return;
#endif

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  struct stat stbuf;
#elif defined(_WIN32) && defined(__MINGW32__)
  struct stat stbuf;
#elif defined(_WIN32)
  struct _stat stbuf;
#endif

  if (dirname.empty()) {
    throw(vpIoException(vpIoException::invalidDirectoryName, "invalid directory name"));
  }

  std::string _dirname = path(dirname);

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  if (stat(_dirname.c_str(), &stbuf) != 0)
#elif defined(_WIN32) && defined(__MINGW32__)
  if (stat(_dirname.c_str(), &stbuf) != 0)
#elif defined(_WIN32)
  if (_stat(_dirname.c_str(), &stbuf) != 0)
#endif
  {
    if (mkdir_p(_dirname.c_str(), 0755) != 0) {
      throw(vpIoException(vpIoException::cantCreateDirectory, "Unable to create directory '%s'", dirname.c_str()));
    }
  }

  if (checkDirectory(dirname) == false) {
    throw(vpIoException(vpIoException::cantCreateDirectory, "Unable to create directory '%s'", dirname.c_str()));
  }
}

std::string getTempPath()
{
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  std::string username;
  getUserName(username);
  return "/tmp/" + username;
#elif defined(_WIN32) && !defined(WINRT)
  // https://docs.microsoft.com/en-us/windows/win32/fileio/creating-and-using-a-temporary-file
  //  Gets the temp path env string (no guarantee it's a valid path).
  TCHAR lpTempPathBuffer[MAX_PATH];
  DWORD dwRetVal = GetTempPath(MAX_PATH /* length of the buffer */, lpTempPathBuffer /* buffer for path */);
  if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
    throw vpIoException(vpIoException::cantGetenv, "Error with GetTempPath() call!");
  }
  std::string temp_path(lpTempPathBuffer);
  if (!temp_path.empty()) {
    if (temp_path.back() == '\\') {
      temp_path.resize(temp_path.size() - 1);
    }
  }
  else {
    temp_path = "C:\temp";
    try {
      makeDirectory(temp_path);
    }
    catch (...) {
      throw(vpException(vpException::fatalError, "Cannot set temp path to %s", temp_path.c_str()));
    }
  }
  return temp_path;
#else
  throw vpIoException(vpException::fatalError, "Not implemented on this platform!");
#endif
}

int main()
{
  std::cout << "This is a wonderful test" << std::endl;

  std::string tmp_dir = getTempPath();
  // #if defined(_WIN32)
  //   std::string tmp_dir = "C:/temp/";
  // #else
  //   std::string tmp_dir = "/tmp/";
  // #endif

    // Get the user login name
  std::string username;
  getUserName(username);

  tmp_dir += username + "/test_xml_parser_rect_oriented/";
  remove(tmp_dir);
  std::cout << "Create: " << tmp_dir << std::endl;
  makeDirectory(tmp_dir);

  return EXIT_SUCCESS;
}
