#pragma once

#include <string>

std::string extract_json_field(const std::string& body, const std::string& key);

// Reverses escape_json(): turns JSON backslash escapes back into raw bytes.
std::string unescape_json(const std::string& s);

// Returns the value of query parameter `key` from a request path/query string,
// e.g. query_param("/status?node=x&id=job_1", "id") == "job_1". "" if absent.
std::string query_param(const std::string& path, const std::string& key);

// True only for "job_" + [A-Za-z0-9_] ids. Rejects path separators / dots so a
// job id can be safely interpolated into a /tmp/<id>.out filename.
bool is_valid_job_id(const std::string& id);
