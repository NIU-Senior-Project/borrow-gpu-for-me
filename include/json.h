#pragma once

#include <string>

std::string extract_json_field(const std::string& body, const std::string& key);

// Reverses escape_json(): turns JSON backslash escapes back into raw bytes.
std::string unescape_json(const std::string& s);
