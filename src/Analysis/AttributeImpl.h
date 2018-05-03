#ifndef LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_IMPL_H
#define LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_IMPL_H

#include <string>
#include <Analysis/Attribute.h>
#include <Util/Bytes.h>
#include <Util/Attribute.h>

using namespace lucene::core::util;

namespace lucene { namespace core { namespace analysis { namespace tokenattributes {

class BytesTermAttributeImpl: public AttributeImpl, public BytesTermAttribute {
  private:
    BytesRef bytes;

  public:
    BytesTermAttributeImpl();
    BytesTermAttributeImpl(const BytesTermAttributeImpl& other);
    BytesRef& GetBytesRef() override;
    void SetBytesRef(BytesRef& bytes) override;
    void Clear() override;
    void ReflectWith(AttributeReflector& reflector) override;
    bool operator==(BytesTermAttributeImpl& other);
};

class CharTermAttributeImpl: public AttributeImpl, public CharTermAttribute, public TermToBytesRefAttribute {
  private:
    static unsigned int MIN_BUFFER_SIZE;
    char* term_buffer;
    unsigned int term_length;

  private:
    void GrowTermBuffer(int new_size);

  public:
    CharTermAttributeImpl();
    CharTermAttributeImpl(const CharTermAttributeImpl& other);
    virtual ~CharTermAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    BytesRef& GetBytesRef() override;
    void CopyBuffer(char* buffer, const int offset, const int length) override;
    char* Buffer() override;
    char* ResizeBuffer(const int new_size) override;
    int Length() override;
    std::string SubSequence(const int start, const int end) override;
    CharTermAttributeImpl& SetLength(const int length) override;
    CharTermAttributeImpl& SetEmpty() override;
    CharTermAttribute& Append(const std::string& csq) override;
    CharTermAttribute& Append(const std::string& csq, const unsigned int start, const unsigned int end) override;
    CharTermAttribute& Append(const char c) override;
    CharTermAttribute& Append(const std::stringbuf& buf) override;
    CharTermAttribute& Append(const CharTermAttribute& term_att) override;
    char operator[](const int idx) override;
    bool operator==(CharTermAttributeImpl& other);
};

class FlagsAttributeImpl: public AttributeImpl, public FlagsAttribute {
  private:
    int flags;

  public:
    FlagsAttributeImpl();
    FlagsAttributeImpl(const FlagsAttributeImpl& other);
    virtual ~FlagsAttributeImpl();
    int GetFlags() override;
    void SetFlags(const int flags) override;
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(const FlagsAttributeImpl& other);
};

class KeywordAttributeImpl: public AttributeImpl, public KeywordAttribute {
  private:
    bool keyword;

  public:
    KeywordAttributeImpl();
    KeywordAttributeImpl(const KeywordAttributeImpl& other);
    virtual ~KeywordAttributeImpl();
    bool IsKeyword() override;
    void SetKeyword(const bool is_keyword) override;
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(const KeywordAttributeImpl& other);
};

class OffsetAttributeImpl: public AttributeImpl, public OffsetAttribute {
  private:
    unsigned int start_offset;
    unsigned int end_offset;

  public:
    OffsetAttributeImpl();
    OffsetAttributeImpl(const OffsetAttributeImpl& other);
    virtual ~OffsetAttributeImpl();
    int StartOffset() override;
    void SetOffset(const int start_offset, const int end_offset) override;
    int EndOffset() override;
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(const OffsetAttributeImpl& other);
};

class PackedTokenAttributeImpl {
  // TODO Implement it
};

class PayloadAttributeImpl: public AttributeImpl, public PayloadAttribute {
  private:
    BytesRef payload;

  public:
    PayloadAttributeImpl();
    PayloadAttributeImpl(const PayloadAttributeImpl& other);
    virtual ~PayloadAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(PayloadAttributeImpl& other);
    BytesRef& GetPayload() override;
    void SetPayload(BytesRef& payload) override;
};

class PositionIncrementAttributeImpl: public AttributeImpl, public PositionIncrementAttribute {
  private:
    unsigned int position_increment;

  public:
    PositionIncrementAttributeImpl();
    PositionIncrementAttributeImpl(const PositionIncrementAttributeImpl& other);
    virtual ~PositionIncrementAttributeImpl();
    void SetPositionIncrement(int position_increment) override;
    unsigned int GetPositionIncrement() override;
    void ReflectWith(AttributeReflector& reflector) override;
    void End() override;
    void Clear() override;
    bool operator==(PositionIncrementAttributeImpl& other);
};

class PositionLengthAttributeImpl: public AttributeImpl, public PositionLengthAttribute {
  private:
    unsigned int position_length;

  public:
    PositionLengthAttributeImpl();
    PositionLengthAttributeImpl(const PositionLengthAttributeImpl& other);
    virtual ~PositionLengthAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(PositionLengthAttributeImpl& other);
    void SetPositionLength(unsigned int position_length) override;
    unsigned int GetPositionLength() override;
};

class TermFrequencyAttributeImpl: public AttributeImpl, public TermFrequencyAttribute {
  private:
    unsigned int term_frequency;

  public:
    TermFrequencyAttributeImpl();
    TermFrequencyAttributeImpl(const TermFrequencyAttributeImpl& other);
    virtual ~TermFrequencyAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(TermFrequencyAttributeImpl& other);
    void SetTermFrequency(unsigned int term_frequency) override;
    unsigned int GetTermFrequency() override;
};

class TypeAttributeImpl: public AttributeImpl, public TypeAttribute {
  private:
    std::string type;

  public:
    TypeAttributeImpl();
    TypeAttributeImpl(const TypeAttributeImpl& other);
    virtual ~TypeAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void Clear() override;
    bool operator==(TypeAttributeImpl& other);
    std::string& Type() override;
    void SetType(std::string& type) override;
};

}}}} // End of namespace

#endif
