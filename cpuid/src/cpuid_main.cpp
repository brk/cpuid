#include <sstream>
#include <iostream>
#include <cstdio>

#include "cpuid.h"

#include "json/json.h"

using Json::Value;

Value Value_from(bool b) { return Value(b); }
Value Value_from(int b) { return Value(b); }
Value Value_from(double b) { return Value(b); }
Value Value_from(const std::string& b) { return Value(b); }
Value Value_from(uint b) { return Value(b); }
Value Value_from(uint64 b) { return Value(double(b)); }
Value Value_from(int64 b) { return Value(double(b)); }

template <typename T>
Value Value_from(const std::map<std::string, T>& amap) {
  Value root;
  typedef std::map<std::string, T> map_type;
  for (typename map_type::const_iterator it = amap.begin(); it != amap.end(); ++it) {
    root[(*it).first] = Value_from((*it).second);
  }
  return root;
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
  name << "L" << params.cache_level << cache_type_str(params.cache_type);
  root[name.str()] = Value_from(params);
}

Value& operator<<(Value& root, const tag_processor_features& feats) {
  root["threads"] = Value(feats.threads);
  root["monitor_line_size_min"] = Value(feats.monitor_features.min_line_size);
  root["monitor_line_size_max"] = Value(feats.monitor_features.max_line_size);
  return root;
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

///////////////////////////////////////////////////////

int main() {
  cpuid_info info;
  cpuid_introspect(info);

  ///////////////////////////////////////////////////////////////////

  std::cout << "max basic %eax cpuid input is " << info.max_basic_eax << std::endl;
  printf("max ext %%eax input is 0x%x\n", info.max_ext_eax);

  //std::cout << "max linear address size: " << << std::endl;
  //std::cout << "max physcl address size: " << << std::endl;

  Value root;
  root["cpuid_version"] = Value(CPUID_VERSION_STRING);

  root["vendor_id"] = info.vendor_id;
  root["model_name"] = info.brand_string;
  for (int i = 0; i < info.processor_cache_parameters.size(); ++i) {
    root["caches"] << info.processor_cache_parameters[i];
  }
  root["features"] = Value_from(info.features);
  root << info.processor_features;

  if (info.features["tsc"]) {
    root["rdtsc_serialized_overhead_cycles"] = Value_from(info.rdtsc_serialized_overhead_cycles);
    root["rdtsc_unserialized_overhead_cycles"] = Value_from(info.rdtsc_unserialized_overhead_cycles);
  }
  
  std::cout << root.toStyledString() << std::endl;
  
  return 0;
}
