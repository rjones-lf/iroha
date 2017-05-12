// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SAMPLE_H_
#define FLATBUFFERS_GENERATED_SAMPLE_H_

#include "flatbuffers/flatbuffers.h"

struct Sample;

struct Sample FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_STRVALUE = 4,
    VT_INTVALUE = 6,
    VT_DOUBLEVALUE = 8
  };
  const flatbuffers::String *strvalue() const {
    return GetPointer<const flatbuffers::String *>(VT_STRVALUE);
  }
  int32_t intvalue() const {
    return GetField<int32_t>(VT_INTVALUE, 0);
  }
  double doublevalue() const {
    return GetField<double>(VT_DOUBLEVALUE, 0.0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_STRVALUE) &&
           verifier.Verify(strvalue()) &&
           VerifyField<int32_t>(verifier, VT_INTVALUE) &&
           VerifyField<double>(verifier, VT_DOUBLEVALUE) &&
           verifier.EndTable();
  }
};

struct SampleBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_strvalue(flatbuffers::Offset<flatbuffers::String> strvalue) {
    fbb_.AddOffset(Sample::VT_STRVALUE, strvalue);
  }
  void add_intvalue(int32_t intvalue) {
    fbb_.AddElement<int32_t>(Sample::VT_INTVALUE, intvalue, 0);
  }
  void add_doublevalue(double doublevalue) {
    fbb_.AddElement<double>(Sample::VT_DOUBLEVALUE, doublevalue, 0.0);
  }
  SampleBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SampleBuilder &operator=(const SampleBuilder &);
  flatbuffers::Offset<Sample> Finish() {
    const auto end = fbb_.EndTable(start_, 3);
    auto o = flatbuffers::Offset<Sample>(end);
    fbb_.Required(o, Sample::VT_STRVALUE);
    return o;
  }
};

inline flatbuffers::Offset<Sample> CreateSample(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> strvalue = 0,
    int32_t intvalue = 0,
    double doublevalue = 0.0) {
  SampleBuilder builder_(_fbb);
  builder_.add_doublevalue(doublevalue);
  builder_.add_intvalue(intvalue);
  builder_.add_strvalue(strvalue);
  return builder_.Finish();
}

inline flatbuffers::Offset<Sample> CreateSampleDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *strvalue = nullptr,
    int32_t intvalue = 0,
    double doublevalue = 0.0) {
  return CreateSample(
      _fbb,
      strvalue ? _fbb.CreateString(strvalue) : 0,
      intvalue,
      doublevalue);
}

#endif  // FLATBUFFERS_GENERATED_SAMPLE_H_
