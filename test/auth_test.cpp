#include "auth.h"

#include <cstdlib>

#include <gtest/gtest.h>

namespace {
void set_token(const char* v) {
    if (v) setenv("AUTH_TOKEN", v, 1);
    else   unsetenv("AUTH_TOKEN");
}
}  // namespace

TEST(AuthTest, ConstantTimeEquals) {
    EXPECT_TRUE(constant_time_equals("secret", "secret"));
    EXPECT_FALSE(constant_time_equals("secret", "secreT"));
    EXPECT_FALSE(constant_time_equals("secret", "secret-longer"));
    EXPECT_TRUE(constant_time_equals("", ""));
}

TEST(AuthTest, DisabledWhenTokenUnset) {
    set_token(nullptr);
    EXPECT_FALSE(auth_enabled());
    // 認證停用時任何請求都放行，且不產生 Authorization 標頭
    EXPECT_TRUE(is_request_authorized("GET / HTTP/1.1\r\nHost: x\r\n"));
    EXPECT_EQ(auth_header_line(), "");
}

TEST(AuthTest, EnforcesBearerWhenTokenSet) {
    set_token("s3cr3t");
    EXPECT_TRUE(auth_enabled());
    EXPECT_EQ(auth_header_line(), "Authorization: Bearer s3cr3t\r\n");

    EXPECT_TRUE(is_request_authorized("POST /execute HTTP/1.1\r\nAuthorization: Bearer s3cr3t\r\n"));
    // 標頭名與 scheme 大小寫不敏感
    EXPECT_TRUE(is_request_authorized("POST /x HTTP/1.1\r\nauthorization: bearer s3cr3t\r\n"));
    // 錯誤或缺少 token 一律拒絕
    EXPECT_FALSE(is_request_authorized("POST /x HTTP/1.1\r\nAuthorization: Bearer wrong\r\n"));
    EXPECT_FALSE(is_request_authorized("POST /x HTTP/1.1\r\nHost: x\r\n"));
    EXPECT_FALSE(is_request_authorized("POST /x HTTP/1.1\r\nAuthorization: s3cr3t\r\n"));

    set_token(nullptr);  // 還原，避免影響其他測試
}
