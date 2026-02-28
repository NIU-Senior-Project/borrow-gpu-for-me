#include "json.h"

#include <string>

std::string extract_json_field(const std::string& body, const std::string& key) {
    // ... (Keep the existing implementation)
    std::string searchKey = "\"" + key + "\"";
    auto keyPos = body.find(searchKey);
    if (keyPos == std::string::npos) return "";

    auto colonPos = body.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    auto firstQuote = body.find('"', colonPos + 1);
    if (firstQuote == std::string::npos) return "";

    auto secondQuote = body.find('"', firstQuote + 1);
    while (secondQuote != std::string::npos && body[secondQuote - 1] == '\\') {
        secondQuote = body.find('"', secondQuote + 1);
    }

    if (secondQuote == std::string::npos) return "";
    return body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}
