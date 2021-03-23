#ifndef PAK_H
#define PAK_H

#include "entry.h"
#include <string>
#include <map>
#include <fstream>
#include <cstdint>

class PAK {

private:
  uint64_t magic;
  uint64_t fileStart;
  uint64_t fileSize;
  char fileSHA1[20];
  uint32_t base_size;
  char *base_dir;
  int num_files;
  std::map<std::string, ENTRY> m;
  const uint64_t CHUNK = 0x10000;

public:
  std::ifstream file;
  std::streampos size;
  std::map<std::string, unsigned char *> decomp_files;
  std::map<std::string, unsigned char *> comp_files;
  std::map<std::string, bool> is_modded;

  PAK(std::wstring filename);
  ~PAK();
  int LoadFiles(std::wstring filename);
  int Decompress(std::string filename);
  int Compress(std::string filename);
  void BuildPak(std::wstring outfile);
  void EditFile(std::string filename, uint64_t address, int value);
  void UpdateBaseDir();
  
};

#endif
