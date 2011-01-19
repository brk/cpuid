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
  { ECX,  7, "eist" },
  { ECX,  9, "ssse3" },
  { ECX, 12, "fma" },
  { ECX, 13, "cx16" },
  { ECX, 15, "pdcm" },
  { ECX, 19, "sse41" },
  { ECX, 20, "sse42" },
  { ECX, 21, "x2apic" },
  { ECX, 22, "movbe" },
  { ECX, 23, "popcnt" },
  { ECX, 24, "tsc-deadline" },
  { ECX, 25, "aes" },
  { ECX, 28, "avx" }
};

feature_bit intel_ext_feature_bits[] = { // EAX = 0x80000001
  { EDX, 29, "x86_64" },
  { ECX,  0, "lahf" }
};

#if 0
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
      }
      break;
  }
  return "Unknown processor signature";
}

const char* intel_processor_signature_string(tag_processor_signature& sig) {
  switch (sig.extended_model) {
  case 0: return intel_extmodel_0_signature_string(sig);
  case 1: return intel_extmodel_1_signature_string(sig);
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
}


void intel_fill_processor_features(cpuid_info& info) {
  cpuid_with_eax(1);
  for (int i = 0; i < ARRAY_SIZE(intel_feature_bits); ++i) {
    feature_bit f(intel_feature_bits[i]);
    info.features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  info.processor_features.logical_processors_per_physical_processor_package
      = MASK_RANGE_IN(ebx, 23, 16);

  cpuid_with_eax(0x80000001);
  for (int i = 0; i < ARRAY_SIZE(intel_ext_feature_bits); ++i) {
    feature_bit f(intel_ext_feature_bits[i]);
    info.features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
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


#if 0
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
#endif


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
// TODO: function 5
// TODO: function 8000_0006
// TODO: function 8000_0008
// TODO: denormals-are-zero


// http://www.intel.com/Assets/PDF/appnote/241618.pdf page 13
bool intel_was_cpuid_input_acceptable(uint eax) {
  return (eax & BIT(31)) == 0;
}
