#include <iostream>
#include <string>

#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(PATH) ::_mkdir(PATH)
#ifndef _S_ISTYPE
#define _S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#define S_ISREG(mode) _S_ISTYPE((mode), _S_IFREG)
#define S_ISDIR(mode) _S_ISTYPE((mode), _S_IFDIR)
#endif
#else
#define MKDIR(PATH) ::mkdir(PATH, 0755)
#endif

static const char separator =
#if defined(_WIN32)
'\\';
#else
'/';
#endif

static bool do_mkdir(const std::string& path) {
  struct stat st;
  if (::stat(path.c_str(), &st) != 0) {
    if (MKDIR(path.c_str()) != 0 && errno != EEXIST) {
      return false;
      }
    }
  else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    return false;
    }
  return true;
  }

bool mkpath(std::string path) {
  std::string build;

  for (size_t pos = 0; (pos = path.find('/')) != std::string::npos;) {
    build += path.substr(0, pos + 1);
    bool ret = do_mkdir(build);
    std::cout << "1 do_mkdir(" << build << ") return: " << ret << std::endl;
    path.erase(0, pos + 1);
    }
  if (!path.empty()) {
    build += path;
    bool ret = do_mkdir(build);
    std::cout << "2 do_mkdir(" << build << ") return: " << ret << std::endl;
    }
  return true;
  }

int main(int argc, char* argv[]) {
  // if (argc >= 2) {
  //   mkpath(argv[1]);
  //   }
  // else {
  //   std::cerr << "please enter a path" << std::endl;
  //   }
  std::string path = "C:/temp/runneradmin/test_xml_parser_rect_oriented-new/";
  std::cout << "Attempt to create: " << path << std::endl;
  mkpath(path);
  return 0;
  }
