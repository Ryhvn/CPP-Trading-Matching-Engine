# CMake generated Testfile for 
# Source directory: /Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine
# Build directory: /Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_CsvParser "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug/test_CsvParser")
set_tests_properties(test_CsvParser PROPERTIES  WORKING_DIRECTORY "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug" _BACKTRACE_TRIPLES "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;97;add_test;/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;0;")
add_test(test_CsvWriter "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug/test_CsvWriter")
set_tests_properties(test_CsvWriter PROPERTIES  WORKING_DIRECTORY "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug" _BACKTRACE_TRIPLES "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;97;add_test;/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;0;")
add_test(test_MatchingEngine "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug/test_MatchingEngine")
set_tests_properties(test_MatchingEngine PROPERTIES  WORKING_DIRECTORY "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug" _BACKTRACE_TRIPLES "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;97;add_test;/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;0;")
add_test(test_OrderBook "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug/test_OrderBook")
set_tests_properties(test_OrderBook PROPERTIES  WORKING_DIRECTORY "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug" _BACKTRACE_TRIPLES "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;97;add_test;/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;0;")
add_test(test_Performance "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug/test_Performance")
set_tests_properties(test_Performance PROPERTIES  WORKING_DIRECTORY "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/cmake-build-debug" _BACKTRACE_TRIPLES "/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;97;add_test;/Users/kemache/Documents/GitHub/CPP-Trading-Matching-Engine/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
