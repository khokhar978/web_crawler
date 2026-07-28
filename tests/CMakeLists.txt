add_executable(test_extractor test_html_extractor.cpp ../src/indexer/HTMLTextExtractor.cpp)
target_include_directories(test_extractor PRIVATE ../include)
add_test(NAME test_extractor COMMAND test_extractor)

add_executable(test_tokenizer test_tokenizer.cpp ../src/indexer/Tokenizer.cpp)
target_include_directories(test_tokenizer PRIVATE ../include)
add_test(NAME test_tokenizer COMMAND test_tokenizer)
