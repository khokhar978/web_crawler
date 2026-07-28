#include <iostream>
#include <cassert>
#include "indexer/HTMLTextExtractor.h"

void testBasicExtraction() {
    std::string html = "<html><body><h1>Hello World</h1><p>This is a test.</p></body></html>";
    std::string expected = "Hello World This is a test.";
    std::string result = HTMLTextExtractor::extractText(html);
    if (result != expected) {
        std::cerr << "Basic extraction failed!\nExpected: '" << expected << "'\nGot: '" << result << "'\n";
        assert(false);
    }
}

void testScriptAndStyleStripping() {
    std::string html = "<div>Text before</div><script>var x = 1; console.log(x);</script><span>Text middle</span><style>body { color: red; }</style><p>Text after</p>";
    std::string expected = "Text before Text middle Text after";
    std::string result = HTMLTextExtractor::extractText(html);
    if (result != expected) {
        std::cerr << "Script/Style stripping failed!\nExpected: '" << expected << "'\nGot: '" << result << "'\n";
        assert(false);
    }
}

void testWhitespaceCollapsing() {
    std::string html = "<p>   Multiple   \n\n  spaces \t should   be   collapsed.  </p>";
    std::string expected = "Multiple spaces should be collapsed.";
    std::string result = HTMLTextExtractor::extractText(html);
    if (result != expected) {
        std::cerr << "Whitespace collapsing failed!\nExpected: '" << expected << "'\nGot: '" << result << "'\n";
        assert(false);
    }
}

int main() {
    std::cout << "Running HTMLTextExtractor tests...\n";
    
    testBasicExtraction();
    std::cout << "[PASS] Basic Extraction\n";
    
    testScriptAndStyleStripping();
    std::cout << "[PASS] Script/Style Stripping\n";
    
    testWhitespaceCollapsing();
    std::cout << "[PASS] Whitespace Collapsing\n";
    
    std::cout << "All tests passed successfully!\n";
    return 0;
}
