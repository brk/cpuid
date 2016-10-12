feature_bit intel_feature_bits[] = { // EAX = 1
  { EDX,  3, "pse" }, // page size extensions, i.e. 4mb pages
  { EDX,  4, "tsc" },
  { EDX,  5, "msr" },
  { EDX,  8, "cx8" },
  { EDX,  9, "apic" },
  { EDX, 11, "sep" }, // sysenter and sysexit
  { EDX, 12, "mtrr" }, // memory type range registers
  { EDX, 13, "pge" }, // page global bit
  { EDX, 15, "cmov" },
  { EDX, 16, "pat" }, // page attribute table
  { EDX, 17, "pse36" }, // 36-bit page size extension
  { EDX, 19, "clflush" },
  { EDX, 21, "ds" }, // debug store
  { EDX, 23, "mmx" },
  { EDX, 25, "sse" },
  { EDX, 26, "sse2" },
  { EDX, 27, "ss" },
  { EDX, 28, "htt" }, // multi-threading/hyper-threading

  { ECX,  0, "sse3" },
  { ECX,  1, "pclmuldq" },
  { ECX,  2, "dtes64" },
  { ECX,  3, "monitor" },
  { ECX,  4, "ds_cpl" },
  { ECX,  5, "vmx" },
  { ECX,  6, "smx" },
  { ECX,  7, "eist" },
  { ECX,  9, "ssse3" },
  { ECX, 10, "l1-ctx-id" },
  { ECX, 12, "fma" },
  { ECX, 13, "cx16" },
  { ECX, 15, "pdcm" }, // perfmon/debug capability
  { ECX, 17, "process-ctx-ids" },
  { ECX, 18, "direct-cache-access" },
  { ECX, 19, "sse41" },
  { ECX, 20, "sse42" },
  { ECX, 21, "x2apic" },
  { ECX, 22, "movbe" },
  { ECX, 23, "popcnt" },
  { ECX, 24, "tsc-deadline" },
  { ECX, 25, "aes" },
  { ECX, 26, "xsave" },
  { ECX, 27, "osxsave" },
  { ECX, 28, "avx" },
  { ECX, 29, "f16c" },
  { ECX, 30, "rdrand" }
};

feature_bit intel_ext_feature_bits[] = { // EAX = 0x80000001
  { EDX, 11, "syscall" },
  { EDX, 20, "nx" },
  { EDX, 26, "page1gb" },
  { EDX, 27, "rdtscp" },
  { EDX, 29, "x86_64" },
  { ECX,  0, "lahf" },
  { ECX,  5, "lzcnt" }
};

feature_bit intel_st_ext_feature_bits[] = { // EAX = 0x7
  { EBX, 0, "fsgsbase"  },
  { EBX, 1, "ia32_tsc_adjust"  },
  { EBX, 2, "sgx"  },
  { EBX, 3, "bmi1"  },
  { EBX, 4, "hle"  },
  { EBX, 5, "avx2" },
  { EBX, 6, "fdp_excptn_only" },
  { EBX, 7, "smep" },
  { EBX, 8, "bmi2"  },
  { EBX, 9, "en-rep-movsb" },
  { EBX, 10, "invpcid" },
  { EBX, 11, "rtm" },
  { EBX, 12, "rdt-m" },
  { EBX, 14, "mpx" },
  { EBX, 15, "rdt-a" },
  { EBX, 18, "rdseed" },
  { EBX, 19, "adx" },
  { EBX, 20, "smap" },
  { EBX, 23, "clflushopt" },
  { EBX, 24, "clwb" },
  { EBX, 25, "processor-trace" },
  { EBX, 29, "sha-ext" },
  { ECX,  2, "umip" },
  { ECX,  3, "pku" },
  { ECX,  4, "ospke" },
  { ECX, 22, "rdpid" },
  { ECX, 30, "sgx-lc" },
};

#if 0
// Binary literals aren't supported by Apple's gcc 4.2.1,
// and the 2.7svn version of ld chokes on clang passing it -demangle.
// Sadness :-(
const char* intel_processor_type_string(tag_processor_signature& sig) {
  switch (sig.processor_type) {
    case 0b00: return "Original OEM Processor";
    case 0b01: return "OverDrive Processor";
    case 0b10: return "Dual Processor";
  }
  return "Unknown processor type";
}

const char* intel_extmodel_0_signature_string(tag_processor_signature& sig) {
  switch (sig.family_code) {
    case 0b0011: return "i386";
    case 0b0100: return "i486";
    case 0b0101: return "i486";
    case 0b0110:
      switch (sig.model_number) {
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
      switch (sig.model_number) {
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

const char* intel_extmodel_1_signature_string(tag_processor_signature& sig) {
  switch (sig.family_code) {
    case 0b0110:
      switch (sig.model_number) {
        case 0b0110: return "Celeron (65 nm)"; // model 16h
        case 0b0111: return "Core 2 Extreme (45 nm)"; // model 17h
        case 0b1100: return "Atom (45 nm)";
        case 0b1010: return "Core i7 (45 nm)";
        case 0b1101: return "Xeon MP (45 nm)";
        case 0b1110: return "Core i5/i7/Mobile/Xeon (45 nm)";
        case 0b1110: return "Xeon MP (45 nm)";
        case 0b1111: return "Xeon MP (32 nm)";
        case 0b1100: return "Xeon MP (32 nm)";
      }
      break;
  }
  return "Unknown processor signature";
}

const char* intel_extmodel_2_signature_string(tag_processor_signature& sig) {
  switch (sig.family_code) {
    case 0b0110:
      switch (sig.model_number) {
        case 0b1110: return "Xeon MP (45 nm)";
        case 0b1111: return "Xeon MP (32 nm)";
        case 0b1100: return "Core i7/Xeon (32 nm)";
        case 0b0101: return "Core i3/i5/i7 Mobile (32 nm)";
        case 0b1010: return "2nd Generation Core/Xeon E3 Sandy Bridge (32 nm)";
        case 0b1101: return "2nd Generation Core/Xeon E5 Sandy Bridge (32 nm)";
      }
      break;
  }
  return "Unknown processor signature";
}

const char* intel_extmodel_3_signature_string(tag_processor_signature& sig) {
  switch (sig.family_code) {
    case 0b0110:
      switch (sig.model_number) {
        case 0b1010: return "3rd Generation Core/Xeon E3 Sandy Bridge (22 nm)";
      }
      break;
  }
  return "Unknown processor signature";
}
const char* intel_processor_signature_string(tag_processor_signature& sig) {
  switch (sig.extended_model) {
  case 0b00: return intel_extmodel_0_signature_string(sig);
  case 0b01: return intel_extmodel_1_signature_string(sig);
  case 0b10: return intel_extmodel_2_signature_string(sig);
  case 0b11: return intel_extmodel_3_signature_string(sig);
  }
  return "Unknown processor signature (due to unknown extended model)";
}
#endif

void intel_fill_processor_signature(tag_processor_signature& sig) {
  cpuid_with_eax(1);
  sig.full_bit_string  = eax;
  sig.stepping_id      = MASK_RANGE_IN(eax,  3, 0);
  sig.model_number     = MASK_RANGE_EX(eax,  7, 3);
  sig.family_code      = MASK_RANGE_EX(eax, 11, 7);
  sig.processor_type   = MASK_RANGE_EX(eax, 13, 11);
  sig.extended_model   = MASK_RANGE_EX(eax, 19, 15);
  sig.extended_family  = MASK_RANGE_EX(eax, 27, 19);
}


void intel_init_all_features_to_false(cpuid_info& info) {
  for (int i = 0; i < ARRAY_SIZE(intel_feature_bits); ++i) {
    info.features[intel_feature_bits[i].name] = false;
  }
  for (int i = 0; i < ARRAY_SIZE(intel_ext_feature_bits); ++i) {
    info.features[intel_ext_feature_bits[i].name] = false;
  }
  for (int i = 0; i < ARRAY_SIZE(intel_st_ext_feature_bits); ++i) {
    info.features[intel_st_ext_feature_bits[i].name] = false;
  }
}

void intel_detect_processor_topology(cpuid_info& info) {
  info.processor_features.max_logical_processors_per_physical_processor_package
      = MASK_RANGE_IN(ebx, 23, 16);
  if (info.features["x2apic"] && info.max_basic_eax >= 0x0B) {
    cpuid_with_eax_and_ecx(0x0B, 0);
    uint threads_per_core = MASK_RANGE_IN(ebx, 15, 0); // as shipped; BIOS may disable some.

    cpuid_with_eax_and_ecx(0x0B, 1);
    uint logical_cores_per_package = MASK_RANGE_IN(ebx, 15, 0);
    uint physical_cores_per_package = logical_cores_per_package / threads_per_core;

    info.processor_features.logical_processors_per_physical_processor_package =
      logical_cores_per_package;
  } else {
    info.processor_features.logical_processors_per_physical_processor_package =
      info.processor_features.max_logical_processors_per_physical_processor_package;
  }
}


void intel_fill_processor_features(cpuid_info& info) {
  cpuid_with_eax(1);
  for (int i = 0; i < ARRAY_SIZE(intel_feature_bits); ++i) {
    feature_bit f(intel_feature_bits[i]);
    info.features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  intel_detect_processor_topology(info);

  cpuid_with_eax(0x80000001);
  for (int i = 0; i < ARRAY_SIZE(intel_ext_feature_bits); ++i) {
    feature_bit f(intel_ext_feature_bits[i]);
    info.features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  cpuid_with_eax(0x7);
  for (int i = 0; i < ARRAY_SIZE(intel_st_ext_feature_bits); ++i) {
    feature_bit f(intel_st_ext_feature_bits[i]);
    info.features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  if (info.max_basic_eax >= 0x0A) {
    cpuid_with_eax(0x0A);
    info.processor_features.pm_features.version_id = MASK_RANGE_IN(eax, 7, 0);
    info.processor_features.pm_features.gp_counters_per_processor = MASK_RANGE_IN(eax, 15, 8);
    info.processor_features.pm_features.gp_counter_bitwidth = MASK_RANGE_IN(eax, 23, 16);
    info.processor_features.pm_features.gp_counter_events   = MASK_RANGE_IN(eax, 31, 24);

    info.processor_features.pm_features.ff_counter_bitwidth = MASK_RANGE_IN(edx,  4, 0);
    info.processor_features.pm_features.ff_counter_count    = MASK_RANGE_IN(edx, 12, 5);
  }

  if (info.max_basic_eax >= 0x80000006) {
    cpuid_with_eax(0x80000006);
    info.cache_line_size = MASK_RANGE_IN(ecx, 7, 0);
    info.cache_size_bytes = 1024 * MASK_RANGE_IN(ecx, 31, 16);
  }

  if (info.max_basic_eax >= 0x80000007) {
    cpuid_with_eax(0x80000007);
    info.features["invariant-tsc"] = BIT_IS_SET(edx, 8);
  }

  if (info.features["monitor"]) {
    cpuid_with_eax(5);
    info.processor_features.monitor_features.min_line_size = MASK_RANGE_IN(eax, 15, 0);
    info.processor_features.monitor_features.max_line_size = MASK_RANGE_IN(ebx, 15, 0);
  } else {
    info.processor_features.monitor_features.min_line_size = 0;
    info.processor_features.monitor_features.max_line_size = 0;
  }
}


void intel_set_cache_properties(tag_processor_cache_descriptor& cache,
      int size, int ways, int entries_or_linesize, bool sectored = false) {
  cache.size = size;
  cache.set_assoc_ways = ways;
  cache.entries_or_linesize = entries_or_linesize;
  cache.sectored = sectored;
}

void intel_decode_cache_descriptor(cpuid_info& info, unsigned char v) {
  const int KB = 1024;
  const int MB = KB * KB;

  switch (v) {
  case 0x01: return intel_set_cache_properties(info.processor_cache_descriptors.TLBi, 4*KB, 4, 32);
  case 0x02: return intel_set_cache_properties(info.processor_cache_descriptors.TLBi, 4*MB, 0, 2);
  case 0x03: return intel_set_cache_properties(info.processor_cache_descriptors.TLBd, 4*KB, 4, 64);
  case 0x04: return intel_set_cache_properties(info.processor_cache_descriptors.TLBd, 4*MB, 4, 8);
  case 0x05: return intel_set_cache_properties(info.processor_cache_descriptors.TLBd, 4*MB, 4, 32);
  case 0x06: return intel_set_cache_properties(info.processor_cache_descriptors.L1i, 8*KB, 4, 32);
  case 0x08: return intel_set_cache_properties(info.processor_cache_descriptors.L1i, 16*KB, 4, 32);
  case 0x09: return intel_set_cache_properties(info.processor_cache_descriptors.L1i, 32*KB, 4, 64);
  case 0x0A: return intel_set_cache_properties(info.processor_cache_descriptors.L1d, 8*KB, 2, 32);
  case 0x0C: return intel_set_cache_properties(info.processor_cache_descriptors.L1d, 16*KB, 4, 32);
  case 0x0D: return intel_set_cache_properties(info.processor_cache_descriptors.L1d, 16*KB, 4, 64); // also ECC...
  case 0x21: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 256*KB, 8, 64);
  case 0x22: return intel_set_cache_properties(info.processor_cache_descriptors.L3, 512*KB, 4, 64, true);
  case 0x23: return intel_set_cache_properties(info.processor_cache_descriptors.L3, 1*MB, 8, 64, true);
  case 0x25: return intel_set_cache_properties(info.processor_cache_descriptors.L3, 2*MB, 8, 64, true);
  case 0x29: return intel_set_cache_properties(info.processor_cache_descriptors.L3, 4*MB, 8, 64, true);
  case 0x2C: return intel_set_cache_properties(info.processor_cache_descriptors.L1d, 32*KB, 8, 64);
  case 0x30: return intel_set_cache_properties(info.processor_cache_descriptors.L1i, 32*KB, 8, 64);
  case 0x39: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 128*KB, 8, 64, true);
  case 0x3A: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 192*KB, 6, 64, true);
  case 0x3B: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 128*KB, 2, 64, true);
  case 0x3C: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 256*KB, 4, 64, true);
  case 0x3D: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 384*KB, 6, 64, true);
  case 0x3E: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 512*KB, 4, 64, true);
  case 0x40: return; // no L2 (or L3) cache                        _descriptors
  case 0x41: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 128*KB, 4, 32);
  case 0x42: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 256*KB, 4, 32);
  case 0x43: return intel_set_cache_properties(info.processor_cache_descriptors.L2, 512*KB, 4, 32);
    // TODO: http://www.intel.com/Assets/PDF/appnote/241618.pdf page 27
    // TODO: http://www.intel.com/Assets/PDF/appnote/241618.pdf page 28
  }
}

// Precondition: CPUID.4 executed
void intel_add_processor_cache_parameters(cpuid_info& info) {
  tag_processor_cache_parameter_set   params;

  params.reserved_APICS                = MASK_RANGE_IN(eax, 31, 26) + 1;
  params.max_sharing_threads           = MASK_RANGE_IN(eax, 25, 14) + 1;
  params.fully_associative             = BIT_IS_SET(eax, 9);
  params.self_initializing_cache_level = BIT_IS_SET(eax, 8);
  params.cache_level                   = MASK_RANGE_IN(eax, 7, 5);
  params.cache_type                    = MASK_RANGE_IN(eax, 4, 0);
  params.ways                          = MASK_RANGE_IN(ebx, 31, 22) + 1;
  params.physical_line_partitions      = MASK_RANGE_IN(ebx, 21, 12) + 1;
  params.system_coherency_line_size    = MASK_RANGE_IN(ebx, 11, 0) + 1;
  params.sets                          = ecx + 1;

  /// "A value of '0' means that WBINVD/INVD from any thread sharing this cache
  ///  acts on all lower cachess for threads sharing this cache. A value of '1'
  ///  means that WBINVD/INVD is not guaranteed to act upon lower-level caches
  ///  of non-originating threads sharing this cache."
  params.inclusive                     = BIT_IS_SET(edx, 1);

  /// "A value of '0' means that WBINVD/INVD from any thread sharing this cache
  ///  acts on all lower cachess for threads sharing this cache. A value of '1'
  ///  means that WBINVD/INVD is not guaranteed to act upon lower-level caches
  ///  of non-originating threads."
  params.inclusive_behavior            = BIT_IS_SET(edx, 0);

  params.size_in_bytes = params.ways * params.physical_line_partitions
                       * params.system_coherency_line_size * params.sets;

  info.processor_cache_parameters.push_back(params);
}

void intel_fill_processor_caches(cpuid_info& info) {
  info.processor_cache_descriptors.TLBi.entries_or_linesize = 0;
  info.processor_cache_descriptors.TLBd.entries_or_linesize = 0;
  info.processor_cache_descriptors.L1i.entries_or_linesize = 0;
  info.processor_cache_descriptors.L1d.entries_or_linesize = 0;
  info.processor_cache_descriptors.L2.entries_or_linesize = 0;
  info.processor_cache_descriptors.L3.entries_or_linesize = 0;

  cpuid_with_eax(2);

  // TODO: function 4
  uint in_ecx = 0;
  cpuid_with_eax_and_ecx(4, in_ecx);
  do {
    intel_add_processor_cache_parameters(info);
    cpuid_with_eax_and_ecx(4, ++in_ecx);
  } while(MASK_RANGE_IN(eax, 4, 0) != 0);
}

// TODO: test EFLAGS first
// TODO: function 5 for MONITOR/MWAIT
// TODO: function 6 for thermal and power management
// TODO: function 8000_0006
// TODO: function 8000_0007
// TODO: function 8000_0008
// TODO: denormals-are-zero


// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13
bool intel_was_cpuid_input_acceptable(uint eax) {
  return (eax & BIT(31)) == 0;
}
