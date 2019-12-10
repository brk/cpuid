#include <sstream>
#include <iostream>
#include <cstdio>

#include "cpuid.h"

template<int N, typename T>
std::string format_bitstring(T x) {
  std::string buf(N, '@');
  for (int i = 0; i < N; ++i) {
    buf[(N - 1) - i] = (x & (1<<i)) ? '1' : '0';
  }
  return "0b" + buf;
}

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
  root["reserved_apics"]      = Value(params.reserved_APICS);
  root["max_sharing_threads"] = Value(params.max_sharing_threads);
  root["ways"]                = Value(params.ways);
  root["line_size"]           = Value(params.system_coherency_line_size);
  root["line_partitions"]     = Value(params.physical_line_partitions);
  root["sets"]                = Value(params.sets);
  root["total_size"]          = Value(params.size_in_bytes);
  root["inclusive"]           = Value(params.inclusive);
  root["inclusive_behavior"]  = Value(params.inclusive_behavior);
  return root;
}

Value& operator<<(Value& root,
                  const tag_processor_cache_parameter_set& params) {
  std::stringstream name;
  name << "L" << params.cache_level << cache_type_str(params.cache_type);
  root[name.str()] = Value_from(params);
  return root;
}

Value& operator<<(Value& root, const tag_processor_features& feats) {
  root["logical_processors_per_physical_processor_package"]
             = Value(feats.logical_processors_per_physical_processor_package);
  root["max_logical_processors_per_physical_processor_package"]
             = Value(feats.max_logical_processors_per_physical_processor_package);
  root["monitor_line_size_min"] = Value(feats.monitor_features.min_line_size);
  root["monitor_line_size_max"] = Value(feats.monitor_features.max_line_size);

  Value pm;
  pm["version"]                   = Value(feats.pm_features.version_id);
  pm["gp_counters_per_processor"] = Value(feats.pm_features.gp_counters_per_processor);
  pm["gp_counter_bitwidth"] = Value(feats.pm_features.gp_counter_bitwidth);
  pm["ff_counter_bitwidth"] = Value(feats.pm_features.ff_counter_bitwidth);
  pm["ff_counter_count"]    = Value(feats.pm_features.ff_counter_count);

  root["perfmon"] = pm;
  return root;
}

Value Value_from(const tag_processor_signature& sig) {
  Value root;
  root["full_bit_string"] = Value(format_bitstring<8 * sizeof(void*)>(sig.full_bit_string));
  root["extended_family"] = Value(sig.extended_family);
  root["extended_model"]  = Value(sig.extended_model);
  root["processor_type"]  = Value(sig.processor_type);
  root["family_code"]     = Value(sig.family_code);
  root["model_number"]    = Value(sig.model_number);
  root["stepping_id"]     = Value(sig.stepping_id);
  return root;
}

///////////////////////////////////////////////////////

int main() {
  cpuid_info info;
  cpuid_introspect(info);

  ///////////////////////////////////////////////////////////////////

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

  root["physical_address_bits"] = info.max_physical_address_size;
  root["linear_address_bits"] = info.max_linear_address_size;
  //root["cache_line_size"] = info.cache_line_size;
  //root["cache_size_bytes"] = info.cache_size_bytes;
  root["max_basic_eax"] = info.max_basic_eax;
  root["max_ext_eax"] = info.max_ext_eax;
  root["signature"] = Value_from(info.processor_signature);

  std::cout << root.toStyledString() << std::endl;

  return 0;
}

