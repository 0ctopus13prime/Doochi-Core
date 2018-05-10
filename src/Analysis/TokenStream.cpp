#include <Analysis/TokenStream.h>

using namespace lucene::core::analysis;
using namespace lucene::core::util;

/**
 *  TokenStream
 */

TokenStream::TokenStream()
  : AttributeSource() {
}

TokenStream::TokenStream(const AttributeSource& input)
  : AttributeSource(input) {
}

TokenStream::TokenStream(AttributeFactory* factory)
  : AttributeSource(factory) {
}

TokenStream::~TokenStream() {
}

void TokenStream::End() {
  EndAttributes();
}

/**
 *  TokenFilter
 */

TokenFilter::TokenFilter(TokenStream& input)
  : TokenStream(input),
    input(input) {
}

TokenFilter::~TokenFilter() {
}

void TokenFilter::End() {
  input.End();
}

void TokenFilter::Reset() {
  input.Reset();
}