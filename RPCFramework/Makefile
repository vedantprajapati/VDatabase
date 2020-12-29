CXX = g++
LDFLAGS = -pthread
LD = g++
CXXFLAGS = -std=c++11 -Igoogletest/googletest/include -Igoogletest/googletest/

GTEST_SRCS = \
	googletest/googletest/src/gtest-all.cc \
	googletest/googletest/src/gtest_main.cc \

PROGS = test-simple test-basic test-proto test-complex test-exhaustive
SRCS_test-basic = rpc.cc test-basic.cc $(GTEST_SRCS)
SRCS_test-proto = rpc.cc test-proto.cc $(GTEST_SRCS)
SRCS_test-simple = rpc.cc test-simple.cc $(GTEST_SRCS)
SRCS_test-complex = rpc.cc test-complex.cc $(GTEST_SRCS)
SRCS_test-exhaustive = test-exhaustive.cc $(GTEST_SRCS)
LDFLAGS_test-exhaustive = -ldl

CXXFLAGS_Release = -O3 -Wall
CXXFLAGS_Debug = -g -Wall

BUILD=Release

include cc.mk
