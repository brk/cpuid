// Copyright (c) 2009 Ben Karel. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

// Based on revision 036 (August 2009) of Intel's Application Note 485,
// Intel (r) Processor Identification and the CPUID Instruction.
// http://www.intel.com/Assets/PDF/appnote/241618.pdf

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <sstream>

#include "json/json.h"

using Json::Value;

Value Value_from(bool b) { return Value(b); }
Value Value_from(int b) { return Value(b); }
Value Value_from(double b) { return Value(b); }
Value Value_from(const std::string& b) { return Value(b); }

template <typename T>
Value Value_from(const std::map<std::string, T>& amap) {
  Value root;
  typedef std::map<std::string, T> map_type;
  for (typename map_type::const_iterator it = amap.begin(); it != amap.end(); ++it) {
    root[(*it).first] = Value_from((*it).second);
  }
  return root;
}

///////////////////////////////////////////////////////////////

#define CPUID_VERSION_STRING "2009-09-14"

typedef unsigned int uint;

// low bit is bit 0, high bit on IA-32 is bit 31
#define BIT(n) (1U << (n))

// mask of bits from 0 to n, inclusive
// e.g. LO_MASK(3) == 0b1111
#define LO_MASK(n) (BIT((n)+1U) - 1U)

// value of bits from hi to lo, inclusive
#define MASK_RANGE_IN(v, hi, lo) ((v >> (lo)) & LO_MASK((hi) - (lo)))

#define MASK_RANGE_EX(v, hi, lo) (MASK_RANGE_IN(v, hi, lo+1U))

#define BIT_IS_SET(v, bit) ((v & BIT(bit)) != 0)

#define ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))

std::string quote(std::string in) {
  std::stringstream ss;
  ss << "\"" << in << "\"";
  return ss.str();
}

// http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html

// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13 and 14

char vendor_id[13];

// global register locatiosn and variable aliases
enum REGISTER { EAX, EBX, ECX, EDX };
uint reg[4] = { 0 };
uint& eax = reg[0]; uint& ebx = reg[1]; uint& ecx = reg[2]; uint& edx = reg[3];

typedef std::map<std::string, bool> features_map;
features_map features;


struct feature_bit { REGISTER reg; int offset; const char* name; };

feature_bit intel_feature_bits[] = { // EAX = 1
  { EDX,  3, "pse" },
  { EDX,  4, "tsc" },
  { EDX,  5, "msr" },
  { EDX,  8, "cx8" },
  { EDX, 15, "cmov" },
  { EDX, 16, "pat" },
  { EDX, 17, "pse36" },
  { EDX, 19, "clflush" },
  { EDX, 21, "ds" },
  { EDX, 23, "mmx" },
  { EDX, 25, "sse" },
  { EDX, 26, "sse2" },
  { EDX, 27, "ss" },
  { EDX, 28, "htt" },

  { ECX,  0, "sse3" },
  { ECX,  1, "pclmuldq" },
  { ECX,  2, "dtes64" },
  { ECX,  3, "monitor" },
  { ECX,  4, "ds_cpl" },
  { ECX,  5, "vmx" },
  { ECX,  6, "smx" },
  { ECX,  9, "ssse3" },
  { ECX, 13, "cx16" },
  { ECX, 15, "pdcm" },
  { ECX, 19, "sse41" },
  { ECX, 20, "sse42" },
  { ECX, 21, "x2apic" },
  { ECX, 22, "movbe" },
  { ECX, 23, "popcnt" },
  { ECX, 25, "aes" }
};

feature_bit intel_ext_feature_bits[] = { // EAX = 0x80000001
  { EDX, 29, "x86_64" },
  { ECX,  0, "lahf" }
};

feature_bit amd_feature_bits[] = {
  { EDX,  3, "pse" },
  { EDX,  4, "tsc" },
  { EDX,  5, "msr" },
  { EDX,  8, "cx8" },
  { EDX, 15, "cmov" },
  { EDX, 16, "pat" },
  { EDX, 17, "pse36" },
  { EDX, 19, "clflush" },
  { EDX, 23, "mmx" },
  { EDX, 25, "sse" },
  { EDX, 26, "sse2" },
  { EDX, 28, "htt" },

  { ECX,  0, "sse3" },
  { ECX,  3, "monitor" },
  { ECX,  9, "ssse3" },
  { ECX, 13, "cx16" },
  { ECX, 19, "sse41" },
  { ECX, 23, "popcnt" },
  { ECX, 31, "raz" }
};

feature_bit amd_ext_feature_bits[] = { // EAX = 0x80000001
  { ECX, 10, "ibs" },
  { ECX,  8, "3dnowprefetch" },
  { ECX,  7, "misalignsse" },
  { ECX,  6, "sse4a" },
  { ECX,  5, "abm" },
  { ECX,  2, "svm" },

  { EDX, 31, "3dnow" },
  { EDX, 30, "3dnowext" },
  { EDX, 29, "x86_64" },
  { EDX, 27, "rdtscp" },
  { EDX, 22, "mmxext" },
  { EDX, 15, "cmov" },
  { EDX,  8, "cx8" },
  { EDX,  5, "msr" },
  { EDX,  3, "pse" }
};

//////////////////////////////////////////////////////////////////////////////

void init_all_features_to_false() {
  for (int i = 0; i < ARRAY_SIZE(intel_feature_bits); ++i) {
    features[intel_feature_bits[i].name] = false;
  }
  for (int i = 0; i < ARRAY_SIZE(intel_ext_feature_bits); ++i) {
    features[intel_ext_feature_bits[i].name] = false;
  }
  for (int i = 0; i < ARRAY_SIZE(amd_feature_bits); ++i) {
    features[amd_feature_bits[i].name] = false;
  }
  for (int i = 0; i < ARRAY_SIZE(amd_ext_feature_bits); ++i) {
    features[amd_ext_feature_bits[i].name] = false;
  }
}

struct tag_processor_features {;
  int threads; // total, or per core?

  struct tag_monitor_features {
    int min_line_size;
    int max_line_size;
  } monitor_features;
} processor_features;

struct tag_processor_cache_descriptor {
  int entries_or_linesize; // 0 if no relevant cache; linesize for non-TLB caches
  int set_assoc_ways; // 0 for fully associative
  bool sectored;
  int size; // page size for TLB caches, total size for non-TLB caches
};

struct tag_processor_cache_descriptors {
  tag_processor_cache_descriptor TLBi, TLBd, L1i, L1d, L2, L3;
} processor_cache_descriptors;

struct tag_processor_cache_parameter_set {
  int reserved_APICS;
  int sharing_threads;
  bool fully_associative;
  bool self_initializing_cache_level;
  int cache_level;
  int cache_type;
  int ways;
  int physical_line_partitions;
  int system_coherency_line_size;
  int sets;
  bool inclusive;
  // the description for EDX[1] and EDX[0] is virtually identical!
  bool inclusive_behavior;

  int size_in_bytes;
};

std::vector<tag_processor_cache_parameter_set> processor_cache_parameters;

char brand_string[48] = { '\0' };

#if 0
  struct tag_processor_signature {
    uint full_bit_string;
    uint extended_family;
    uint extended_model;
    uint processor_type;
    uint family_code;
    uint model_number;
    uint stepping_id;
  } processor_signature;


  const char* intel_processor_type_string() {
    switch (processor_signature.processor_type) {
      case 0b00: return "Original OEM Processor";
      case 0b01: return "OverDrive Processor";
      case 0b10: return "Dual Processor";
    }
    return "Unknown processor type";
  }

  const char* intel_extmodel_0_signature_string() {
    switch (processor_signature.family_code) {
      case 0b0011: return "i386";
      case 0b0100: return "i486";
      case 0b0101: return "i486";
      case 0b0110:
        switch (processor_signature.model_number) {
          case 0b0001: return "Pentium Pro";
          case 0b0011: // OverDrive
          case 0b0101: return "Pentium II";
          case 0b0110: return "Celeron";
          case 0b0111:
          case 0b1000:
          case 0b1010:
          case 0b1011: return "Pentium III";
          case 0b1001: return "Pentium M";
          case 0b1101: return "Pentium M (90 nm)";
          case 0b1110: return "Core Duo or Core Solo (65nm)";
          case 0b1111: return "Core 2 Duo or Core 2 Quad (65nm)";
        }
        break;
      case 0b1111:
        switch (processor_signature.model_number) {
          case 0b0000: // model 00h
          case 0b0001: return "Pentium 4 (180 nm)"; // model 01h
          case 0b0010: return "Pentium 4 (130 nm)"; // model 02h
          case 0b0011: // model 03h
          case 0b0100: return "Pentium 4 (90 nm)"; // model 04h
          case 0b0110: return "Pentium 4 (65 nm)"; // model 06h
        }
        break;
    }
    return "Unknown processor signature";
  }

  const char* intel_extmodel_1_signature_string() {
    switch (processor_signature.family_code) {
      case 0b0110:
        switch (processor_signature.model_number) {
          case 0b0110: return "Celeron (65 nm)"; // model 16h
          case 0b0111: return "Core 2 Extreme (45 nm)"; // model 17h
          case 0b1100: return "Atom (45 nm)";
          case 0b1010: return "Core i7 (45 nm)";
          case 0b1101: return "Xeon MP (45 nm)";
        }
        break;
    }
    return "Unknown processor signature";
  }

  const char* intel_processor_signature_string() {
    switch (processor_signature.extended_model) {
    case 0: return intel_extmodel_0_signature_string();
    case 1: return intel_extmodel_1_signature_string();
    }
    return "Unknown processor signature (due to unknown extended model)";
  }

  void intel_fill_processor_signature() {
    cpuid_with_eax(1);
    processor_signature.full_bit_string  = eax;
    processor_signature.stepping_id      = MASK_RANGE_IN(eax,  3, 0);
    processor_signature.model_number     = MASK_RANGE_EX(eax,  7, 3);
    processor_signature.family_code      = MASK_RANGE_EX(eax, 11, 7);
    processor_signature.processor_type   = MASK_RANGE_EX(eax, 13, 11);
    processor_signature.extended_model   = MASK_RANGE_EX(eax, 19, 15);
    processor_signature.extended_family  = MASK_RANGE_EX(eax, 27, 19);
  }

  std::ostream& operator<<(std::ostream& out, const tag_processor_signature& sig) {
    return out << "{" << std::endl
                << "\textended family: " << sig.extended_family << std::endl
                << "\textended model:  " << sig.extended_model << std::endl
                << "\tprocessor type:  " << sig.processor_type << std::endl
                << "\tfamily code:     " << sig.family_code << std::endl
                << "\tmodel_number:    " << sig.model_number << std::endl
                << "\tstepping id:     " << sig.stepping_id << std::endl
                << "}";
  }

#endif

// http://linux.derkeiler.com/Newsgroups/comp.os.linux.development.system/2008-01/msg00174.html
void cpuid_with_eax(uint in_eax) {
  __asm__(
      "pushl %%ebx\n\t"       // save %ebx for PIC code on OS X
      "cpuid\n\t"
      "movl %%ebx, %%esi\n\t" // save what cpuid put in ebx
      "popl %%ebx\n\t"       // restore ebx
          : "=a"(eax), "=S"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax) // input in_eax to %eax
          );
}

void cpuid_with_eax_and_ecx(uint in_eax, uint in_ecx) {
  __asm__(
      "pushl %%ebx\n\t"       // save %ebx for PIC code on OS X
      "cpuid\n\t"
      "movl %%ebx, %%esi\n\t" // save what cpuid put in ebx
      "popl %%ebx\n\t"       // restore ebx
          : "=a"(eax), "=S"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax), "c"(in_ecx) // input in_eax to %eax and in_ecx to %ecx
          );
}

void intel_fill_processor_features() {
  cpuid_with_eax(1);
  for (int i = 0; i < ARRAY_SIZE(intel_feature_bits); ++i) {
    feature_bit f(intel_feature_bits[i]);
    features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  processor_features.threads = MASK_RANGE_IN(ebx, 23, 16);

  cpuid_with_eax(0x80000001);
  for (int i = 0; i < ARRAY_SIZE(intel_ext_feature_bits); ++i) {
    feature_bit f(intel_ext_feature_bits[i]);
    features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  if (features["monitor"]) {
    cpuid_with_eax(5);
    processor_features.monitor_features.min_line_size = MASK_RANGE_IN(eax, 15, 0);
    processor_features.monitor_features.max_line_size = MASK_RANGE_IN(ebx, 15, 0);
  } else {
    processor_features.monitor_features.min_line_size = 0;
    processor_features.monitor_features.max_line_size = 0;
  }
}

void amd_fill_processor_features() {
  cpuid_with_eax(1);
  for (int i = 0; i < ARRAY_SIZE(amd_feature_bits); ++i) {
    feature_bit f(amd_feature_bits[i]);
    features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  if (features["htt"]) {
    processor_features.threads = MASK_RANGE_IN(ebx, 23, 16);
  } else {
    processor_features.threads = 1;
  }

  cpuid_with_eax(0x80000001);
  for (int i = 0; i < ARRAY_SIZE(amd_ext_feature_bits); ++i) {
    feature_bit f(amd_ext_feature_bits[i]);
    features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  if (features["monitor"]) {
    cpuid_with_eax(5);
    processor_features.monitor_features.min_line_size = MASK_RANGE_IN(eax, 15, 0);
    processor_features.monitor_features.max_line_size = MASK_RANGE_IN(ebx, 15, 0);
  } else {
    processor_features.monitor_features.min_line_size = 0;
    processor_features.monitor_features.max_line_size = 0;
  }
}

void intel_set_cache_properties(tag_processor_cache_descriptor& cache,
      int size, int ways, int entries_or_linesize, bool sectored = false) {
  cache.size = size;
  cache.set_assoc_ways = ways;
  cache.entries_or_linesize = entries_or_linesize;
  cache.sectored = sectored;
}

void intel_decode_cache_descriptor(unsigned char v) {
  const int KB = 1024;
  const int MB = KB * KB;

  switch (v) {
  case 0x01: return intel_set_cache_properties(processor_cache_descriptors.TLBi, 4*KB, 4, 32);
  case 0x02: return intel_set_cache_properties(processor_cache_descriptors.TLBi, 4*MB, 0, 2);
  case 0x03: return intel_set_cache_properties(processor_cache_descriptors.TLBd, 4*KB, 4, 64);
  case 0x04: return intel_set_cache_properties(processor_cache_descriptors.TLBd, 4*MB, 4, 8);
  case 0x05: return intel_set_cache_properties(processor_cache_descriptors.TLBd, 4*MB, 4, 32);
  case 0x06: return intel_set_cache_properties(processor_cache_descriptors.L1i, 8*KB, 4, 32);
  case 0x08: return intel_set_cache_properties(processor_cache_descriptors.L1i, 16*KB, 4, 32);
  case 0x09: return intel_set_cache_properties(processor_cache_descriptors.L1i, 32*KB, 4, 64);
  case 0x0A: return intel_set_cache_properties(processor_cache_descriptors.L1d, 8*KB, 2, 32);
  case 0x0C: return intel_set_cache_properties(processor_cache_descriptors.L1d, 16*KB, 4, 32);
  case 0x0D: return intel_set_cache_properties(processor_cache_descriptors.L1d, 16*KB, 4, 64); // also ECC...
  case 0x21: return intel_set_cache_properties(processor_cache_descriptors.L2, 256*KB, 8, 64);
  case 0x22: return intel_set_cache_properties(processor_cache_descriptors.L3, 512*KB, 4, 64, true);
  case 0x23: return intel_set_cache_properties(processor_cache_descriptors.L3, 1*MB, 8, 64, true);
  case 0x25: return intel_set_cache_properties(processor_cache_descriptors.L3, 2*MB, 8, 64, true);
  case 0x29: return intel_set_cache_properties(processor_cache_descriptors.L3, 4*MB, 8, 64, true);
  case 0x2C: return intel_set_cache_properties(processor_cache_descriptors.L1d, 32*KB, 8, 64);
  case 0x30: return intel_set_cache_properties(processor_cache_descriptors.L1i, 32*KB, 8, 64);
  case 0x39: return intel_set_cache_properties(processor_cache_descriptors.L2, 128*KB, 8, 64, true);
  case 0x3A: return intel_set_cache_properties(processor_cache_descriptors.L2, 192*KB, 6, 64, true);
  case 0x3B: return intel_set_cache_properties(processor_cache_descriptors.L2, 128*KB, 2, 64, true);
  case 0x3C: return intel_set_cache_properties(processor_cache_descriptors.L2, 256*KB, 4, 64, true);
  case 0x3D: return intel_set_cache_properties(processor_cache_descriptors.L2, 384*KB, 6, 64, true);
  case 0x3E: return intel_set_cache_properties(processor_cache_descriptors.L2, 512*KB, 4, 64, true);
  case 0x40: return; // no L2 (or L3) cache                   _descriptors
  case 0x41: return intel_set_cache_properties(processor_cache_descriptors.L2, 128*KB, 4, 32);
  case 0x42: return intel_set_cache_properties(processor_cache_descriptors.L2, 256*KB, 4, 32);
  case 0x43: return intel_set_cache_properties(processor_cache_descriptors.L2, 512*KB, 4, 32);
    // TODO: http://www.intel.com/Assets/PDF/appnote/241618.pdf page 27
    // TODO: http://www.intel.com/Assets/PDF/appnote/241618.pdf page 28
  }
}

void intel_add_processor_cache_parameters() {
  tag_processor_cache_parameter_set   params;

  params.reserved_APICS                = MASK_RANGE_IN(eax, 31, 26) + 1;
  params.sharing_threads               = MASK_RANGE_IN(eax, 25, 14) + 1;
  params.fully_associative             = BIT_IS_SET(eax, 9);
  params.self_initializing_cache_level = BIT_IS_SET(eax, 8);
  params.cache_level                   = MASK_RANGE_IN(eax, 7, 5);
  params.cache_type                    = MASK_RANGE_IN(eax, 4, 0);
  params.ways                          = MASK_RANGE_IN(ebx, 31, 22) + 1;
  params.physical_line_partitions      = MASK_RANGE_IN(ebx, 21, 12) + 1;
  params.system_coherency_line_size    = MASK_RANGE_IN(ebx, 11, 0) + 1;
  params.sets                          = ecx + 1;
  params.inclusive                     = BIT_IS_SET(edx, 1);
  params.inclusive_behavior            = BIT_IS_SET(edx, 0);

  params.size_in_bytes = params.ways * params.physical_line_partitions
                       * params.system_coherency_line_size * params.sets;

  processor_cache_parameters.push_back(params);
}

void intel_fill_processor_caches() {
  processor_cache_descriptors.TLBi.entries_or_linesize = 0;
  processor_cache_descriptors.TLBd.entries_or_linesize = 0;
  processor_cache_descriptors.L1i.entries_or_linesize = 0;
  processor_cache_descriptors.L1d.entries_or_linesize = 0;
  processor_cache_descriptors.L2.entries_or_linesize = 0;
  processor_cache_descriptors.L3.entries_or_linesize = 0;

  cpuid_with_eax(2);

  // TODO: function 4
  uint in_ecx = 0;
  cpuid_with_eax_and_ecx(4, in_ecx);
  do {
    intel_add_processor_cache_parameters();
    cpuid_with_eax_and_ecx(4, ++in_ecx);
  } while(MASK_RANGE_IN(eax, 4, 0) != 0);
}

// TODO: test EFLAGS first
// TODO: function 5
// TODO: function 8000_0006
// TODO: function 8000_0008
// TODO: denormals-are-zero

uint cpuid_vendor_id_and_max_basic_eax_input() {
  cpuid_with_eax(0);
  ((uint*)vendor_id)[0] = ebx;
  ((uint*)vendor_id)[1] = edx;
  ((uint*)vendor_id)[2] = ecx;
  return eax;
}

// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13
uint cpuid_max_acceptable_extended_eax_input() {
  cpuid_with_eax(0x80000000);
  return eax;
}

// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13
bool intel_was_cpuid_input_acceptable(uint eax) {
  return (eax & BIT(31)) == 0;
}

void intel_fill_brand_string_helper(int offset) {
  ((uint*)brand_string)[offset + 0] = eax;
  ((uint*)brand_string)[offset + 1] = ebx;
  ((uint*)brand_string)[offset + 2] = ecx;
  ((uint*)brand_string)[offset + 3] = edx;
}

void cpuid_fill_brand_string() {
  cpuid_with_eax(0x80000002);
  intel_fill_brand_string_helper(0);
  cpuid_with_eax(0x80000003);
  intel_fill_brand_string_helper(4);
  cpuid_with_eax(0x80000004);
  intel_fill_brand_string_helper(8);
}

void intel_detect_processor_topology() {
  if (features["x2apic"]) {
    cpuid_with_eax_and_ecx(0x0b, 0);
    uint threads_per_core = MASK_RANGE_IN(ebx, 15, 0);

    cpuid_with_eax_and_ecx(0x0b, 1);
    uint logical_cores_per_package = MASK_RANGE_IN(ebx, 15, 0);
    uint physical_cores_per_package = logical_cores_per_package / threads_per_core;

    std::cout << "threads per core: " << threads_per_core << std::endl;
    std::cout << "logical per package: " << logical_cores_per_package << std::endl;
    std::cout << "physical per package: " << physical_cores_per_package << std::endl;
  } else {
    std::cout << "no x2apic support" << std::endl;
    cpuid_with_eax(1);
    uint logical_processors_per_physical_package = MASK_RANGE_IN(ebx, 23, 16);
    std::cout << "logical cores per physical package: " << logical_processors_per_physical_package << std::endl;
  }
}

///////////////////// PRINTING FUNCTIONS ///////////////////

const char* cache_type_tag(int type) {
    switch (type) {
    case 0: return "-";
    case 1: return "d"; // data
    case 2: return "i"; // instruction
    case 3: return ""; // unified
  }
  return "?";
}

Value Value_from(const tag_processor_cache_parameter_set& params) {
  Value root;
  root["reserved_apics"]  = Value(params.reserved_APICS);
  root["sharing_threads"] = Value(params.sharing_threads);
  root["ways"]            = Value(params.ways);
  root["line_size"]       = Value(params.system_coherency_line_size);
  root["sets"]            = Value(params.sets);
  root["total_size"]      = Value(params.size_in_bytes);
  return root;
}

Value& operator<<(Value& root,
                  const tag_processor_cache_parameter_set& params) {
  std::stringstream name;
  name << "L" << params.cache_level << cache_type_tag(params.cache_type);
  root[name.str()] = Value_from(params);
}

Value& operator<<(Value& root, const tag_processor_features& feats) {
  root["threads"] = Value(feats.threads);
  root["monitor_line_size_min"] = Value(feats.monitor_features.min_line_size);
  root["monitor_line_size_max"] = Value(feats.monitor_features.max_line_size);
  return root;
}

///////////////////////////////////////////////////////

int main() {
  vendor_id[12] = '\0';
  uint max_basic_eax = cpuid_vendor_id_and_max_basic_eax_input();
  //std::cout << "max basic %eax cpuid input is " << max_basic_eax << std::endl;

  cpuid_fill_brand_string();

  init_all_features_to_false();

  if (std::string("GenuineIntel") == vendor_id) {
    intel_fill_processor_caches();
    intel_fill_processor_features();
    //intel_detect_processor_topology();
  } else if (std::string("AuthenticAMD") == vendor_id) {
    amd_fill_processor_features();
  } else {
    std::cerr << "Unknown vendor id!" << std::endl;
    return 1;
  }

  Value root;
  root["cpuid_version"] = Value(CPUID_VERSION_STRING);

  root["vendor_id"] = vendor_id;
  root["model_name"] = brand_string;
  for (int i = 0; i < processor_cache_parameters.size(); ++i) {
    root["caches"] << processor_cache_parameters[i];
  }
  root["features"] = Value_from(features);
  root << processor_features;

  std::cout << root.toStyledString() << std::endl;

	return 0;
}



