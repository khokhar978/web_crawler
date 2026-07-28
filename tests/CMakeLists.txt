add_executable(test_extractor test_html_extractor.cpp ../src/indexer/HTMLTextExtractor.cpp)
target_include_directories(test_extractor PRIVATE ../include)
add_test(NAME test_extractor COMMAND test_extractor)
