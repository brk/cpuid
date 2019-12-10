
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

void amd_init_all_features_to_false(cpuid_info& info) {
  for (int i = 0; i < ARRAY_SIZE(amd_feature_bits); ++i) {
    info.features[amd_feature_bits[i].name] = false;
  }
  for (int i = 0; i < ARRAY_SIZE(amd_ext_feature_bits); ++i) {
    info.features[amd_ext_feature_bits[i].name] = false;
  }
}

void amd_fill_processor_features(cpuid_info& info) {
  cpuid_with_eax(1);
  for (int i = 0; i < ARRAY_SIZE(amd_feature_bits); ++i) {
    feature_bit f(amd_feature_bits[i]);
    info.features[f.name] = BIT_IS_SET(reg[f.reg], f.offset);
  }

  if (info.features["htt"]) {
    info.processor_features.logical_processors_per_physical_processor_package
        = MASK_RANGE_IN(ebx, 23, 16);
  } else {
    info.processor_features.logical_processors_per_physical_processor_package = 1;
  }

  cpuid_with_eax(0x80000001);
  for (int i = 0; i < ARRAY_SIZE(amd_ext_feature_bits); ++i) {
    feature_bit f(amd_ext_feature_bits[i]);
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


tag_processor_cache_parameter_set amd_L1_cache_parameters(int reg) {
    tag_processor_cache_parameter_set cache = { 0 };
    cache.size_in_bytes              = 1024 * MASK_RANGE_IN(reg, 31, 24);
    cache.system_coherency_line_size = MASK_RANGE_IN(reg, 7, 0);
    cache.sets                       = MASK_RANGE_IN(reg, 23, 16);
    cache.ways                       = MASK_RANGE_IN(reg, 15, 8);
    cache.cache_level                = 1;
    return cache;
}

int amd_l2_l3_cache_assoc(int bits) {
  switch (bits) {
    case 0x6: return 8;
    case 0x8: return 16;
    case 0xA: return 32;
    case 0xB: return 48;
    case 0xC: return 64;
    case 0xD: return 96;
    case 0xE: return 128;
  }
  return bits;
}

tag_processor_cache_parameter_set amd_L2_cache_parameters(int reg) {
    tag_processor_cache_parameter_set cache = { 0 };
    cache.system_coherency_line_size = MASK_RANGE_IN(reg, 7, 0);
    cache.sets = amd_l2_l3_cache_assoc(MASK_RANGE_IN(reg, 15, 12));
    cache.ways                       = MASK_RANGE_IN(reg, 11, 8);
    cache.cache_type = cache_type_tag('u');
    return cache;
}


void amd_fill_processor_caches(cpuid_info& info) {
  uint max_eax = info.max_ext_eax;
  if (  max_eax >= 0x80000005) {
    cpuid_with_eax(0x80000005);
    tag_processor_cache_parameter_set L1i = amd_L1_cache_parameters(edx);
    L1i.cache_type = cache_type_tag('i');
    info.processor_cache_parameters.push_back(L1i);

    tag_processor_cache_parameter_set L1d = amd_L1_cache_parameters(ecx);
    L1d.cache_type = cache_type_tag('d');
    info.processor_cache_parameters.push_back(L1d);
  }

  if (  max_eax >= 0x80000006) {
    cpuid_with_eax(0x80000006);
    tag_processor_cache_parameter_set L2 = amd_L2_cache_parameters(ecx);
    L2.size_in_bytes = 1024 * MASK_RANGE_IN(ecx, 31, 16);
    L2.cache_level = 2;
    info.processor_cache_parameters.push_back(L2);

    tag_processor_cache_parameter_set L3 = amd_L2_cache_parameters(edx);
    L3.size_in_bytes = 512 * 1024 * MASK_RANGE_IN(edx, 31, 18);
    L3.cache_level = 3;
    info.processor_cache_parameters.push_back(L3);
  }
}


