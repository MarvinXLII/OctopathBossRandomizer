#ifndef ENTRY_H
#define ENTRY_H

#include <cstdint>
#include <vector>
#include <cstring>

struct ENTRY {
  uint64_t base;
  uint64_t size;
  uint64_t decomp_size;
  uint32_t is_comp;
  unsigned char sha1[20];
  int num_zips;
  std::vector<uint64_t> chunk_sizes;
  int zip_size;
};

#endif
