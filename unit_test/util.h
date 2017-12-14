//
// Created by h46incon on 2017/12/14.
//

#ifndef UNIT_TEST_IJST_UTIL_H
#define UNIT_TEST_IJST_UTIL_H

#include <gtest/gtest.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <ijst/ijst.h>
#include <ijst/types_std.h>

#define UTEST_MOVE_TO_STRING_AND_CHECK(st, pushAllField, doc)				\
{																			\
	string json;															\
	int toStrRet = st._.SerializeToString(pushAllField, json);				\
	ASSERT_EQ(toStrRet, 0);													\
	doc.Parse(json.c_str(), json.length());									\
	ASSERT_FALSE(doc.HasParseError());										\
	rapidjson::Value _jVal;													\
	int serRet = st._.MoveSerializeInInnerAlloc(pushAllField, _jVal);		\
	ASSERT_EQ(serRet, 0);													\
	ASSERT_EQ((rapidjson::Value&)doc, _jVal);								\
	ASSERT_EQ(st._.GetBuffer().MemberCount(), 0u);							\
}

#endif //UNIT_TEST_IJST_UTIL_H
