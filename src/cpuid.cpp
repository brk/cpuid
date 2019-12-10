// Copyright (c) 2009 Ben Karel. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file or at http://eschew.org/txt/bsd.txt

#include "cpuid.h"

///////////////////////////////////////////////////////////////

int cache_type_tag(char c) {
  switch (c) {
    case 'd': return 1; // data
    case 'i': return 2; // instruction
    case 'u': return 3; // unified
  }
  return 0; // default
}

int cpuid_small_cache_size(cpuid_info& info) {
  int min_size = 1<<30;
  int instr_cache = cache_type_tag('i');

  cpuid_info::cache_parameters::iterator it;
  for (it = info.processor_cache_parameters.begin();
      it != info.processor_cache_parameters.end();
      ++it) {
    if (it->cache_type == instr_cache) continue;
    if (it->size_in_bytes < min_size) {
      min_size = it->size_in_bytes;
    }
  }

  return min_size;
}

int cpuid_large_cache_size(cpuid_info& info) {
  int max_size = -1;

  cpuid_info::cache_parameters::iterator it;
  for (it = info.processor_cache_parameters.begin();
      it != info.processor_cache_parameters.end();
      ++it) {
    if (it->size_in_bytes > max_size) {
      max_size = it->size_in_bytes;
    }
  }

  return max_size;
}

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

#define MERGE_HI_LO(a, b) ((((uint64)a) << 32) | ((uint64)b))

#define D(expr) (#expr) << " = " << (expr)

// http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html

// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13 and 14

// global register locatiosn and variable aliases
enum REGISTER { EAX, EBX, ECX, EDX };
uint reg[4] = { 0 };
uint& eax = reg[0]; uint& ebx = reg[1]; uint& ecx = reg[2]; uint& edx = reg[3];

struct feature_bit { REGISTER reg; int offset; const char* name; };

//////////////////////////////////////////////////////////////////////////////

#ifndef __LP64__
// http://linux.derkeiler.com/Newsgroups/comp.os.linux.development.system/2008-01/msg00174.html
inline void cpuid_with_eax(uint in_eax) {
  __asm__ __volatile__(
      "pushl %%ebx\n\t"       // save %ebx for PIC code on OS X
      "cpuid\n\t"
      "movl %%ebx, %%esi\n\t" // save what cpuid put in ebx
      "popl %%ebx\n\t"       // restore ebx
          : "=a"(eax), "=S"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax) // input in_eax to %eax
          );
}

inline void cpuid_with_eax_and_ecx(uint in_eax, uint in_ecx) {
  __asm__ __volatile__(
      "pushl %%ebx\n\t"       // save %ebx for PIC code on OS X
      "cpuid\n\t"
      "movl %%ebx, %%esi\n\t" // save what cpuid put in ebx
      "popl %%ebx\n\t"       // restore ebx
          : "=a"(eax), "=S"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax), "c"(in_ecx) // input in_eax to %eax and in_ecx to %ecx
          );
}

uint64 rdtsc_unserialized() {
  uint64 ticks;
  __asm__ __volatile__("rdtsc": "=A" (ticks));
  return ticks;
}

#else // use 64 bit gas syntax
inline void cpuid_with_eax(uint in_eax) {
  __asm__ __volatile__(
      "pushq %%rbx\n\t"       // save %ebx for PIC code on OS X
      "cpuid\n\t"
      "movq %%rbx, %%rsi\n\t" // save what cpuid put in ebx
      "popq %%rbx\n\t"       // restore ebx
          : "=a"(eax), "=S"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax) // input in_eax to %eax
          );
}

inline void cpuid_with_eax_and_ecx(uint in_eax, uint in_ecx) {
  __asm__ __volatile__(
      "pushq %%rbx\n\t"       // save %ebx for PIC code on OS X
      "cpuid\n\t"
      "movq %%rbx, %%rsi\n\t" // save what cpuid put in ebx
      "popq %%rbx\n\t"       // restore ebx
          : "=a"(eax), "=S"(ebx), "=d"(edx), "=c"(ecx) // output
          : "a"(in_eax), "c"(in_ecx) // input in_eax to %eax and in_ecx to %ecx
          );
}

uint64 rdtsc_unserialized() {
  uint a, d;
  __asm__ __volatile__("rdtsc": "=a"(a), "=d"(d));
  return MERGE_HI_LO(d, a);
}
#endif

uint64 rdtsc_serialized() {
  cpuid_with_eax(0);
  uint64 ticks;
  __asm__ __volatile__("rdtsc": "=A" (ticks));
  return ticks;
}

//////////////////////////////////////////////////////////////////////////////

#include "cpuid_intel-inc.h"
#include "cpuid_amd-inc.h"

//////////////////////////////////////////////////////////////////////////////

void init_all_features_to_false(cpuid_info& info) {
  intel_init_all_features_to_false(info);
  amd_init_all_features_to_false(info);
}

uint cpuid_vendor_id_and_max_basic_eax_input(cpuid_info& info) {
  cpuid_with_eax(0);
  ((uint*)info.vendor_id)[0] = ebx;
  ((uint*)info.vendor_id)[1] = edx;
  ((uint*)info.vendor_id)[2] = ecx;
  return eax;
}

// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13
uint cpuid_max_acceptable_extended_eax_input() {
  cpuid_with_eax(0x80000000);
  return eax;
}

void fill_brand_string_helper(cpuid_info& info, int offset) {
  ((uint*)info.brand_string)[offset + 0] = eax;
  ((uint*)info.brand_string)[offset + 1] = ebx;
  ((uint*)info.brand_string)[offset + 2] = ecx;
  ((uint*)info.brand_string)[offset + 3] = edx;
}

void cpuid_fill_brand_string(cpuid_info& info) {
  cpuid_with_eax(0x80000002);
  fill_brand_string_helper(info, 0);
  cpuid_with_eax(0x80000003);
  fill_brand_string_helper(info, 4);
  cpuid_with_eax(0x80000004);
  fill_brand_string_helper(info, 8);
}

void estimate_rdtsc_overhead(cpuid_info& info) {
  uint64 a, b;
  a = rdtsc_serialized();
  b = rdtsc_serialized();
  info.rdtsc_serialized_overhead_cycles = double(b - a);

  a = rdtsc_unserialized();
  b = rdtsc_unserialized();
  info.rdtsc_unserialized_overhead_cycles = double(b - a);
}


bool cpuid_introspect(cpuid_info& info) {
  info.vendor_id[12] = '\0';

  info.max_basic_eax = cpuid_vendor_id_and_max_basic_eax_input(info);
  info.max_ext_eax = cpuid_max_acceptable_extended_eax_input();

  cpuid_fill_brand_string(info);

  init_all_features_to_false(info);

  if (std::string("GenuineIntel") == info.vendor_id) {
    intel_fill_processor_caches(info);
    intel_fill_processor_features(info);
    //intel_detect_processor_topology();
    intel_fill_processor_signature(info.processor_signature);
  } else if (std::string("AuthenticAMD") == info.vendor_id) {
    amd_fill_processor_features(info);
    amd_fill_processor_caches(info);
  } else {
    // Unknown vendor ID!
    return false;
  }

  // Feature/flag bits common to all platforms:
  if (info.max_ext_eax >= 0x80000008) {
    cpuid_with_eax(0x80000008);
    info.max_physical_address_size = MASK_RANGE_IN(eax, 7, 0);
    info.max_linear_address_size   = MASK_RANGE_IN(eax, 15, 8);
  }

  if (info.features["tsc"]) {
    estimate_rdtsc_overhead(info);
  }

  return true;
}

