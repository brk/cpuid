#ifndef CPUID_H
#define CPUID_H

#define CPUID_VERSION_STRING "2016-11-29"

// Based on revision 036 (August 2009) of Intel's Application Note 485,
// Intel (r) Processor Identification and the CPUID Instruction.
// http://www.intel.com/Assets/PDF/appnote/241618.pdf

#include <cstring>
#include <string>
#include <vector>
#include <map>

typedef unsigned int       uint;
typedef unsigned long long uint64;
typedef          long long  int64;

struct cpuid_info;

bool cpuid_introspect(cpuid_info&);

int cpuid_small_cache_size(cpuid_info&);
int cpuid_large_cache_size(cpuid_info&);

/////////////////////////////////////////////////////////////////////

struct tag_processor_features {
  int max_logical_processors_per_physical_processor_package;
  int     logical_processors_per_physical_processor_package;

  struct tag_monitor_features {
    int min_line_size;
    int max_line_size;
  } monitor_features;

  struct tag_pm_features {
    int version_id;
    int gp_counters_per_processor;
    int gp_counter_bitwidth;
    int gp_counter_events;
    int ff_counter_bitwidth;
    int ff_counter_count;
  } pm_features;
};

struct tag_processor_cache_descriptor {
  int entries_or_linesize; // 0 if no relevant cache; linesize for non-TLB caches
  int set_assoc_ways; // 0 for fully associative
  bool sectored;
  int size; // page size for TLB caches, total size for non-TLB caches
};

struct tag_processor_cache_descriptors {
  tag_processor_cache_descriptor TLBi, TLBd, L1i, L1d, L2, L3;
};


inline const char* cache_type_str(int type) {
  switch (type) {
    case 0: return "-";
    case 1: return "d"; // data
    case 2: return "i"; // instruction
    case 3: return ""; // unified
  }
  return "?";
}

struct tag_processor_cache_parameter_set {
  int reserved_APICS;
  int max_sharing_threads;
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

struct tag_processor_signature {
  uint full_bit_string;
  uint extended_family;
  uint extended_model;
  uint processor_type;
  uint family_code;
  uint model_number;
  uint stepping_id;
};

struct cpuid_info {
  cpuid_info() {
    memset(brand_string,        0, sizeof(brand_string));
    memset(vendor_id,           0, sizeof(vendor_id));
    memset(&processor_signature, 0xFF, sizeof(processor_signature));
    rdtsc_serialized_overhead_cycles = -1;
    rdtsc_unserialized_overhead_cycles = -1;
  }

  // A Core i7 has the following basic leafs, by EAX value:
  //  0x05 for MONITOR/MWAIT
  //  0x0A for Architectural Performance Monitoring
  //  0x0B for Extended Topology Enumeration
  //  0x0C (invalid)
  //  0x80000008 for linear/physical address size data
  int max_basic_eax;
  uint max_ext_eax;
  int max_linear_address_size;
  int max_physical_address_size;
  int cache_line_size;
  int cache_size_bytes;
  double rdtsc_serialized_overhead_cycles;
  double rdtsc_unserialized_overhead_cycles;

  typedef std::vector<tag_processor_cache_parameter_set> cache_parameters;
  cache_parameters processor_cache_parameters;

  tag_processor_features           processor_features;
  tag_processor_signature          processor_signature;
  tag_processor_cache_descriptors  processor_cache_descriptors;

  typedef std::map<std::string, bool> feature_flags;
  feature_flags features;

  char brand_string[48];
  char vendor_id[13];
};

#endif
