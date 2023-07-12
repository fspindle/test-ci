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

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#if (TARGET_OS_IOS == 0) && !defined(__ANDROID__)
void replaceAll(std::string& str, const std::string& search, const std::string& replace) {
  size_t start_pos = 0;
  while ((start_pos = str.find(search, start_pos)) != std::string::npos) {
    str.replace(start_pos, search.length(), replace);
    start_pos += replace.length(); // Handles case where 'replace' is a
    // substring of 'search'
    }
  }
#endif
#endif

std::string path(const std::string& pathname) {
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


bool checkFilename(const std::string& filename) {
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

bool checkDirectory(const std::string& dirname) {
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

bool checkFifo(const std::string& fifofilename) {
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
  printf("ERROR: Fifo files are not supported on Windows platforms.");
  return false;
#endif
  }

bool remove(const std::string& file_or_dir) {
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
    printf("ERROR: Cannot remove %s: not implemented on iOS Platform", file_or_dir.c_str());
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
    printf("ERROR: Cannot remove %s: not implemented on Universal Windows Platform",
      file_or_dir.c_str());
#endif
#endif
    }
  else {
    std::cout << "Cannot remove: " << file_or_dir << std::endl;
    return false;
    }
  }

void getUserName(std::string& username) {
  // With MinGW, UNIX and _WIN32 are defined
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) // UNIX
  // Get the user name.
  char* _username = ::getenv("LOGNAME");
  if (!_username) {
    username = "unknown";
    }
  else {
    username = _username;
    }
#elif defined(_WIN32)
#if (!defined(WINRT))
  unsigned int info_buffer_size = 1024;
  TCHAR* infoBuf = new TCHAR[info_buffer_size];
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

int mkdir_p(const char* path, int mode) {
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
  for (char* p = _path + 1; *p; p++) { // path cannot be empty
    if (*p == sep) {
      /* Temporarily truncate */
      *p = '\0';

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
      if (mkdir(_path, static_cast<mode_t>(mode)) != 0)
#elif defined(_WIN32)
      (void)mode; // var not used
      if (!checkDirectory(_path) && _mkdir(_path) != 0)
#endif
        {
        if (errno != EEXIST)
          return -1;
        }
      *p = sep;
      }
    }

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
  if (mkdir(_path, static_cast<mode_t>(mode)) != 0)
#elif defined(_WIN32)
  if (_mkdir(_path) != 0)
#endif
    {
    if (errno != EEXIST)
      return -1;
    }

  return 0;
  }

void makeDirectory(const std::string& dirname) {
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
    printf("ERROR: invalid directory name");
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
      printf("ERROR: Unable to create directory '%s'", dirname.c_str());
      }
    }

  if (checkDirectory(dirname) == false) {
    printf("ERROR: Unable to create directory '%s'", dirname.c_str());
    }
  }

int main() {
  std::cout << "This is a wonderful test" << std::endl;

#if defined(_WIN32)
  std::string tmp_dir = "C:/temp/";
#else
  std::string tmp_dir = "/tmp/";
#endif

  // Get the user login name
  std::string username;
  getUserName(username);

  tmp_dir += username + "/test_xml_parser_rect_oriented/";
  remove(tmp_dir);
  std::cout << "Create: " << tmp_dir << std::endl;
  makeDirectory(tmp_dir);

  return EXIT_SUCCESS;
  }

