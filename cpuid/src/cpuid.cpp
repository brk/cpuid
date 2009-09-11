// Copyright (c) 2009 Ben Karel. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <iostream>
#include <cassert>
#include <string>

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

// http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html

// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13 and 14

char vendor_id[13];
struct tag_processor_signature {
  uint full_bit_string;
  uint extended_family;
  uint extended_model;
  uint processor_type;
  uint family_code;
  uint model_number;
  uint stepping_id;
} processor_signature;

struct tag_processor_features {
  bool page_size_extension;
  bool time_stamp_counter;
  bool model_specific_registers;
  bool cmpxchg8;
  bool cmov;
  bool clflush;
  bool debug_store;
  bool mmx;
  bool sse;
  bool sse2;
  bool sse3;
  bool pclmuldq;
  bool cmpxchg16;
  bool debug_store_64;
  bool monitor;
  bool vmx;
  bool sse3b;
  bool perf_cap_msr;
  bool sse41;
  bool sse42;
  bool movbe;
  bool popcnt;
  bool aes;
  
  int threads; // total, or per core?
} processor_features;

struct tag_processor_cache {
  int entries_or_linesize; // 0 if no relevant cache; linesize for non-TLB caches 
  int set_assoc_ways; // 0 for fully associative
  bool sectored;
  int size; // page size for TLB caches, total size for non-TLB caches
};

struct tag_processor_caches {
  tag_processor_cache TLBi, TLBd, L1i, L1d, L2, L3;
} processor_caches;

// global register variables
uint eax, ebx, ecx, edx;

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

void cpuid_with_eax(uint in_eax) {
  __asm__("cpuid"
          : "=a"(eax), "=b"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax) // input in_eax to %eax
          );
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

void intel_fill_processor_features() {
  cpuid_with_eax(1);
  processor_features.page_size_extension       = BIT_IS_SET(edx,  3);
  processor_features.time_stamp_counter        = BIT_IS_SET(edx,  4);
  processor_features.model_specific_registers  = BIT_IS_SET(edx,  5);
  processor_features.cmpxchg8                  = BIT_IS_SET(edx,  8);
  processor_features.cmov                      = BIT_IS_SET(edx, 15);
  processor_features.clflush                   = BIT_IS_SET(edx, 19);
  processor_features.debug_store               = BIT_IS_SET(edx, 21);
  processor_features.mmx                       = BIT_IS_SET(edx, 23);
  processor_features.sse                       = BIT_IS_SET(edx, 25);
  processor_features.sse2                      = BIT_IS_SET(edx, 26);
  
  processor_features.threads = MASK_RANGE_IN(ebx, 23, 16);
             
  processor_features.sse3                      = BIT_IS_SET(ecx,  0);
  processor_features.pclmuldq                  = BIT_IS_SET(ecx,  1);
  processor_features.debug_store_64            = BIT_IS_SET(ecx,  2);
  processor_features.monitor                   = BIT_IS_SET(ecx,  3);
  processor_features.vmx                       = BIT_IS_SET(ecx,  5);
  processor_features.sse3b                     = BIT_IS_SET(ecx,  9);
  processor_features.cmpxchg16                 = BIT_IS_SET(ecx, 13);
  processor_features.perf_cap_msr              = BIT_IS_SET(ecx, 15);
  processor_features.sse41                     = BIT_IS_SET(ecx, 19);
  processor_features.sse42                     = BIT_IS_SET(ecx, 20);
  processor_features.movbe                     = BIT_IS_SET(ecx, 22);
  processor_features.popcnt                    = BIT_IS_SET(ecx, 23);
  processor_features.aes                       = BIT_IS_SET(ecx, 25);
}

void intel_set_cache_properties(tag_processor_cache& cache,
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
  case 0x01: return intel_set_cache_properties(processor_caches.TLBi, 4*KB, 4, 32);
  case 0x02: return intel_set_cache_properties(processor_caches.TLBi, 4*MB, 0, 2);
  case 0x03: return intel_set_cache_properties(processor_caches.TLBd, 4*KB, 4, 64);
  case 0x04: return intel_set_cache_properties(processor_caches.TLBd, 4*MB, 4, 8);
  case 0x05: return intel_set_cache_properties(processor_caches.TLBd, 4*MB, 4, 32);
  case 0x06: return intel_set_cache_properties(processor_caches.L1i, 8*KB, 4, 32);
  case 0x08: return intel_set_cache_properties(processor_caches.L1i, 16*KB, 4, 32);
  case 0x09: return intel_set_cache_properties(processor_caches.L1i, 32*KB, 4, 64);
  case 0x0A: return intel_set_cache_properties(processor_caches.L1d, 8*KB, 2, 32);
  case 0x0C: return intel_set_cache_properties(processor_caches.L1d, 16*KB, 4, 32);
  case 0x0D: return intel_set_cache_properties(processor_caches.L1d, 16*KB, 4, 64); // also ECC...
  case 0x21: return intel_set_cache_properties(processor_caches.L2, 256*KB, 8, 64);
  case 0x22: return intel_set_cache_properties(processor_caches.L3, 512*KB, 4, 64, true);
  case 0x23: return intel_set_cache_properties(processor_caches.L3, 1*MB, 8, 64, true);
  case 0x25: return intel_set_cache_properties(processor_caches.L3, 2*MB, 8, 64, true);
  case 0x29: return intel_set_cache_properties(processor_caches.L3, 4*MB, 8, 64, true);
  case 0x2C: return intel_set_cache_properties(processor_caches.L1d, 32*KB, 8, 64);
  case 0x30: return intel_set_cache_properties(processor_caches.L1i, 32*KB, 8, 64);
  case 0x39: return intel_set_cache_properties(processor_caches.L2, 128*KB, 8, 64, true);
  case 0x3A: return intel_set_cache_properties(processor_caches.L2, 192*KB, 6, 64, true);
  case 0x3B: return intel_set_cache_properties(processor_caches.L2, 128*KB, 2, 64, true);
  case 0x3C: return intel_set_cache_properties(processor_caches.L2, 256*KB, 4, 64, true);
  case 0x3D: return intel_set_cache_properties(processor_caches.L2, 384*KB, 6, 64, true);
  case 0x3E: return intel_set_cache_properties(processor_caches.L2, 512*KB, 4, 64, true);
  case 0x40: return; // no L2 (or L3) cache
  case 0x41: return intel_set_cache_properties(processor_caches.L2, 128*KB, 4, 32);
  case 0x42: return intel_set_cache_properties(processor_caches.L2, 256*KB, 4, 32);
  case 0x43: return intel_set_cache_properties(processor_caches.L2, 512*KB, 4, 32);
    // TODO: http://www.intel.com/Assets/PDF/appnote/241618.pdf page 27
    // TODO: http://www.intel.com/Assets/PDF/appnote/241618.pdf page 28
  }
}

void intel_fill_processor_caches() {
  processor_caches.TLBi.entries_or_linesize = 0;
  processor_caches.TLBd.entries_or_linesize = 0;
  processor_caches.L1i.entries_or_linesize = 0;
  processor_caches.L1d.entries_or_linesize = 0;
  processor_caches.L2.entries_or_linesize = 0;
  processor_caches.L3.entries_or_linesize = 0;
  
  cpuid_with_eax(2);
 
  // TODO: function 4
}

// TODO: test EFLAGS first
// TODO: function 5
// TODO: function 8000_000[234]
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




int main() {
  vendor_id[12] = '\0';
  uint max_basic_eax = cpuid_vendor_id_and_max_basic_eax_input();
  std::cout << "max basic %eax cpuid input is " << max_basic_eax << std::endl;
  std::cout << "vendor id is '" << vendor_id << "'" << std::endl;
  
  if (std::string("GenuineIntel") == vendor_id) {
    intel_fill_processor_signature();
    std::cout << processor_signature << std::endl;
    std::cout << processor_signature.full_bit_string << std::endl;
    std::cout << "Processor signature: " << intel_processor_signature_string() << std::endl;
  } else {
    std::cout << "Unknown vendor id!" << std::endl;
  }
  
	return 0;
}



