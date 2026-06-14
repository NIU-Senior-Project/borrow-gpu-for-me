#include "json.h"
#include "request.h"

#include <gtest/gtest.h>

// escape_json (送出端) 與 unescape_json (接收端) 必須互為反函式，
// 否則腳本經過 Manager 轉發後內容會損壞。
TEST(JsonEscapeTest, RoundTripPreservesScript) {
    const std::string original =
        "#!/bin/bash\n"
        "echo \"hello\tworld\"\n"
        "path='C:\\tmp\\run'\r\n"
        "python -c 'print(1)'\n";

    EXPECT_EQ(unescape_json(escape_json(original)), original);
}

TEST(JsonEscapeTest, UnescapesTabAndBackslash) {
    // escape_json 產生的 \t 與 \\ 都要被還原成原始位元組
    EXPECT_EQ(unescape_json("a\\tb"), "a\tb");
    EXPECT_EQ(unescape_json("a\\\\b"), "a\\b");
    EXPECT_EQ(unescape_json("line1\\nline2"), "line1\nline2");
}

TEST(QueryParamTest, ExtractsIdRegardlessOfPosition) {
    EXPECT_EQ(query_param("/status?id=job_1", "id"), "job_1");
    EXPECT_EQ(query_param("/status?node=10.0.0.1&id=job_1", "id"), "job_1");
    EXPECT_EQ(query_param("/status?id=job_1&extra=1", "id"), "job_1");
}

TEST(QueryParamTest, KeyBoundaryAndMissing) {
    // 不可把 "uuid" 當成 "id"
    EXPECT_EQ(query_param("/status?uuid=abc", "id"), "");
    EXPECT_EQ(query_param("/status", "id"), "");
    EXPECT_EQ(query_param("/status?foo=bar", "id"), "");
}

TEST(JobIdTest, AcceptsWellFormedIds) {
    EXPECT_TRUE(is_valid_job_id("job_1700000000000"));
    EXPECT_TRUE(is_valid_job_id("job_deadbeefcafe1234"));
}

TEST(JobIdTest, RejectsTraversalAndMalformed) {
    // 路徑穿越／注入嘗試一律拒絕
    EXPECT_FALSE(is_valid_job_id("../../etc/passwd"));
    EXPECT_FALSE(is_valid_job_id("job_../../etc/passwd"));
    EXPECT_FALSE(is_valid_job_id("job_a/b"));
    EXPECT_FALSE(is_valid_job_id("job_a.out"));
    EXPECT_FALSE(is_valid_job_id("job_"));     // 只有前綴
    EXPECT_FALSE(is_valid_job_id(""));
    EXPECT_FALSE(is_valid_job_id("nope_123")); // 缺前綴
}
