#pragma once

#include <string>

std::string extract_json_field(const std::string& body, const std::string& key);

// Reverses escape_json(): turns JSON backslash escapes back into raw bytes.
std::string unescape_json(const std::string& s);

// Returns the value of query parameter `key` from a request path/query string,
// e.g. query_param("/status?node=x&id=job_1", "id") == "job_1". "" if absent.
std::string query_param(const std::string& path, const std::string& key);
