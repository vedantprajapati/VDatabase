// -*- c++ -*-
#ifndef RPCXX_SAMPLE_H
#define RPCXX_SAMPLE_H

#include <iostream>
#include <typeinfo>
#include <cstdlib>
#include "rpc.h"
#include <iostream>

namespace rpc {

// Protocol is used for encode and decode a type to/from the network.
//
// You may use network byte order, but it's optional. We won't test your code
// on two different architectures.

// TASK1: add more specializations to Protocol template class to support more
// types.
template <typename T>
 struct Protocol {
  static constexpr size_t TYPE_SIZE = sizeof(T);
  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const T &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, T &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};
/*
template <> struct Protocol<int> {
  static constexpr size_t TYPE_SIZE = sizeof(int);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const int &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, int &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<bool> {
  static constexpr size_t TYPE_SIZE = sizeof(bool);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const bool &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, bool &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<float> {
  static constexpr size_t TYPE_SIZE = sizeof(float);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const float &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, float &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<double> {
  static constexpr size_t TYPE_SIZE = sizeof(double);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const double &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, double &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<char> {
  static constexpr size_t TYPE_SIZE = sizeof(char);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const char &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, char &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<unsigned char> {
  static constexpr size_t TYPE_SIZE = sizeof(unsigned char);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const unsigned char &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, unsigned char &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<unsigned int> {
  static constexpr size_t TYPE_SIZE = sizeof(unsigned int);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const unsigned int &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, unsigned int &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<short> {
  static constexpr size_t TYPE_SIZE = sizeof(short);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const short &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, short &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<long> {
  static constexpr size_t TYPE_SIZE = sizeof(long);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const long &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, long &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<long long> {
  static constexpr size_t TYPE_SIZE = sizeof(long long);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const long long &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, long long &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<unsigned long long> {
  static constexpr size_t TYPE_SIZE = sizeof(unsigned long long);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const unsigned long long &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, unsigned long long &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<unsigned short> {
  static constexpr size_t TYPE_SIZE = sizeof(unsigned short);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const unsigned short &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, unsigned short &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};

template <> struct Protocol<unsigned long> {
  static constexpr size_t TYPE_SIZE = sizeof(unsigned long);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const unsigned long &x) {
    if (*out_len < TYPE_SIZE) return false;
    memcpy(out_bytes, &x, TYPE_SIZE);
    *out_len = TYPE_SIZE;
    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, unsigned long &x) {
    if (*in_len < TYPE_SIZE) return false;
    memcpy(&x, in_bytes, TYPE_SIZE);
    *in_len = TYPE_SIZE;
    return true;
  }
};*/

// ok so here im using pascal strings (basically what we did in task 1 and task 3 and @605 on piazza)
//sending the length of the string first as a uint32_t, and then the string
template <> struct Protocol<std::string> {
  static constexpr int SIZE_LEN = sizeof(uint32_t);

  static bool Encode(uint8_t *out_bytes, uint32_t *out_len, const std::string &x) {
    uint8_t stringlength = x.length();

    if(*out_len < (uint8_t)(stringlength + 1))
      return false;
    memcpy(out_bytes, &stringlength, 1);
    memcpy(out_bytes + 1, x.c_str(), stringlength);
    *out_len = stringlength + 1;

    return true;
  }
  static bool Decode(uint8_t *in_bytes, uint32_t *in_len, bool *ok, std::string &x) {
    if(*in_len < 1)
      return false;
    else{
      uint8_t string_length;
      memcpy(&string_length, in_bytes, 1);

      if(((uint8_t)(*in_len - 1))< string_length)
        return false;
      else{
        x.assign((char*)(in_bytes + 1), string_length);
        *in_len = string_length + 1;
        return true;
      }
      
    }
    return false;
  }
};

// TASK2: Client-side

class IntParam : public BaseParams {
  int p;
 public:
  IntParam(int p) : p(p) {}

  bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    return Protocol<int>::Encode(out_bytes, out_len, p);
  }
};

class BoolParam : public BaseParams {
  bool p;
 public:
  BoolParam(bool p) : p(p) {}

  bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    return Protocol<bool>::Encode(out_bytes, out_len, p);
  }
};



class UIntParam : public BaseParams {
  unsigned int p;
 public:
  UIntParam(unsigned int p) : p(p) {}

  bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    return Protocol<unsigned int>::Encode(out_bytes, out_len, p);
  }
};
template<typename ... >
class Param : public BaseParams{};

template<typename T>
class Param<T> : public BaseParams{
	T p;
	public:
	Param(T p) : p(p){}

	bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    return Protocol<T>::Encode(out_bytes, out_len, p);
}
};

template <> class Param<void>: public BaseParams{
  public:
	bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    *out_len = 0;
    return true;
  }
};

template <> class Param<std::string> : public BaseParams {
  std::string p;
 public:
  Param(std::string p) : p(p) {}

  bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    return Protocol<std::string>::Encode(out_bytes, out_len, p);
  }
};
//twoparams
template<typename A, typename B> class Param<A, B>:public BaseParams {
  A ArgOne;
  B ArgTwo;
  public:
  Param(A ArgOne, B ArgTwo) : ArgOne(ArgOne),ArgTwo(ArgTwo){}
	bool Encode(uint8_t *out_bytes, uint32_t *out_len) const override {
    uint32_t buffsize = *out_len;
    if (Protocol<A>::Encode(out_bytes, out_len, ArgOne)==false){
      return false;
    }
    uint32_t outlenOne = *out_len;
    buffsize -= outlenOne;
    out_bytes += outlenOne;
    *out_len = buffsize;
    if (Protocol<B>::Encode(out_bytes, out_len, ArgTwo)==false){
      return false;
    }
    outlenOne += *out_len;
    *out_len = outlenOne;
    return true;
  }
};

// TASK2: Server-side
template <typename Svc>
class IntIntProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    int x;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<int>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
	std::cout << "gets to int int procedure";
    // Now we cast the function pointer func_ptr to its original type.
    //
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = int (Svc::*)(int);
    auto p = func_ptr.To<FunctionPointerType>();
    int result = (((Svc *) instance)->*p)(x);
    if (!Protocol<int>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    return true;
  }
};
template <typename Svc>
class VoidVoidProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    
    *in_len = 0;//This function takes no parameters, doesnt consume any bytes
    std::cout << "i got here!" << "\n";
    // Now we cast the function pointer func_ptr to its original type.
    //
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = void (Svc::*)();
    auto p = func_ptr.To<FunctionPointerType>();
    (((Svc *) instance)->*p)();
    *out_len = 0;
    return true;
  }
};
template <typename Svc>
class BoolVoidProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {

    // Now we cast the function pointer func_ptr to its original type.
    //
    // This incomplete solution only works for this type of member functions.
    *in_len = 0;
    std::cout << "gets to bool void proc";
    using FunctionPointerType = bool (Svc::*)();
    auto p = func_ptr.To<FunctionPointerType>();
    bool result = (((Svc *) instance)->*p)();
    if (!Protocol<bool>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    *out_len = sizeof(bool);
    return true;
  }
};
/*
template <typename Svc>
class BoolProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    bool x;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<bool>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    // Now we cast the function pointer func_ptr to its original type.
    //
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = bool (Svc::*)(bool);
    auto p = func_ptr.To<FunctionPointerType>();
    bool result = (((Svc *) instance)->*p)(x);
    if (!Protocol<bool>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    return true;
  }
};&*/
template <typename Svc>
class StrStrProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
	  std::cout << "now at str str procedure";
    std::string x;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<std::string>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    //*in_len = sizeof(std::string);
    // Now we cast the function pointer func_ptr to its original type.
    //

    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = std::string (Svc::*)(std::string);
    auto p = func_ptr.To<FunctionPointerType>();
    std::string result = (((Svc *) instance)->*p)(x);
    if (!Protocol<std::string>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    //*out_len = sizeof(std::string);
    return true;
  }
};

// TASK2: Server-side
template <typename Svc>
class StrIntProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    unsigned int x;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<unsigned int>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    // Now we cast the function pointer func_ptr to its original type.
    //
	  std::cout <<"gets strint procedure";
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = std::string (Svc::*)(unsigned int);
    auto p = func_ptr.To<FunctionPointerType>();
    auto result = (((Svc *) instance)->*p)(x);
    if (!Protocol<std::string>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    //*out_len = sizeof(std::string);
    return true;
  }
};

template <typename Svc>
class StrStrIntProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    std::string myString;
    int x;
    uint32_t buffsize = *in_len;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<std::string>::Decode(in_bytes, in_len, ok, myString) || !*ok) {
      return false;
    }
    uint32_t inlenOne = *in_len;
    buffsize -= inlenOne;
    in_bytes += inlenOne;
    *in_len = buffsize;
    if (!Protocol<int>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    inlenOne += *in_len;
    *in_len = inlenOne;
    // Now we cast the function pointer func_ptr to its original type.
    //
	std::cout <<"gets strstrint @@@@ procedure";
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = std::string (Svc::*)(std::string, unsigned int);
    auto p = func_ptr.To<FunctionPointerType>();
    auto result = (((Svc *) instance)->*p)(myString, x);
    if (!Protocol<std::string>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    //*out_len = sizeof(std::string);
    return true;
  }
};

template <typename Svc>
class ULongIntUIntProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    int x;
    unsigned int y;
    uint32_t buffsize = *in_len;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<int>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    uint32_t inlenOne = *in_len;
    buffsize -= inlenOne;
    in_bytes += inlenOne;
    *in_len = buffsize;
    if (!Protocol<unsigned int>::Decode(in_bytes, in_len, ok, y) || !*ok) {
      return false;
    }
    inlenOne += *in_len;
    *in_len = inlenOne;
    // Now we cast the function pointer func_ptr to its original type.
    //
	std::cout <<"gets strstrint @@@@ procedure";
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = unsigned long (Svc::*)(int, unsigned int);
    auto p = func_ptr.To<FunctionPointerType>();
    auto result = (((Svc *) instance)->*p)(x, y);
    if (!Protocol<unsigned long>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    //*out_len = sizeof(std::string);
    return true;
  }
};


template <typename Svc>
class VoidStrStrProcedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    std::string x;
    std::string y;
    uint32_t buffsize = *in_len;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<std::string>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    uint32_t inlenOne = *in_len;
    buffsize -= inlenOne;
    in_bytes += inlenOne;
    *in_len = buffsize;
    if (!Protocol<std::string>::Decode(in_bytes, in_len, ok, y) || !*ok) {
      return false;
    }
    inlenOne += *in_len;
    *in_len = inlenOne;
    // Now we cast the function pointer func_ptr to its original type.
    //
	std::cout <<"gets voidstrstr @@@@ procedure";
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = void (Svc::*)(std::string, std::string);
    auto p = func_ptr.To<FunctionPointerType>();
    (((Svc *) instance)->*p)(x, y);
    // if (!Protocol<void>::Encode(out_bytes, out_len, result)) {
    //   // out_len should always be large enough so this branch shouldn't be
    //   // taken. However just in case, we return an fatal error by setting *ok
    //   // to false.
    //   *ok = false;
    //   return false;
    // }
    *out_len = 0;
    return true;
  }
};

template <typename Svc, typename T>
class Procedure : public BaseProcedure {
  bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                        uint8_t *out_bytes, uint32_t *out_len,
                        bool *ok) override final {
    T x;
    // This function is similar to Decode. We need to return false if buffer
    // isn't large enough, or fatal error happens during parsing.
    if (!Protocol<T>::Decode(in_bytes, in_len, ok, x) || !*ok) {
      return false;
    }
    // Now we cast the function pointer func_ptr to its original type.
    //
    // This incomplete solution only works for this type of member functions.
    using FunctionPointerType = T (Svc::*)(T);
    auto p = func_ptr.To<FunctionPointerType>();
    T result = (((Svc *) instance)->*p)(x);

    if (!Protocol<T>::Encode(out_bytes, out_len, result)) {
      // out_len should always be large enough so this branch shouldn't be
      // taken. However just in case, we return an fatal error by setting *ok
      // to false.
      *ok = false;
      return false;
    }
    return true;
  }
};

// TASK2: Client-side
/*
class IntResult : public BaseResult {
  int r;
 public:
  bool HandleResponse(uint8_t *in_bytes, uint32_t *in_len, bool *ok) override final {
    return Protocol<int>::Decode(in_bytes, in_len, ok, r);
  }
  int &data() { return r; }
};

class BoolResult: public BaseResult{
bool r;
 public:
  bool HandleResponse(uint8_t *in_bytes, uint32_t *in_len, bool *ok) override final {
	std::cout <<" bool result";
    return Protocol<bool>::Decode(in_bytes, in_len, ok, r);
  }
  bool &data() { return r; }

};

class StrResult: public BaseResult{
std::string r;
 public:
  bool HandleResponse(uint8_t *in_bytes, uint32_t *in_len, bool *ok) override final {
    return Protocol<std::string>::Decode(in_bytes, in_len, ok, r);
  }
  std::string &data() { return r; }

};*/

template<typename T>
class Result : public BaseResult{
  T r;
public:
bool HandleResponse(uint8_t *in_bytes, uint32_t *in_len, bool *ok) override final {
    return Protocol<T>::Decode(in_bytes, in_len, ok, r);
  }
  T &data() { return r; }
};

template<> class Result<void>:public BaseResult{
  public:
  bool HandleResponse(uint8_t *in_bytes, uint32_t *in_len, bool *ok) override final {
    *in_len = 0;
    *ok = true;
    return true;
  }
};

// TASK2: Client-side
class Client : public BaseClient {
 public:
  template <typename Svc>
  Result<int> *Call(Svc *svc, int (Svc::*func)(int), int x) {
    // Lookup instance and function IDs.
    int instance_id = svc->instance_id();
    int func_id = svc->LookupExportFunction(MemberFunctionPtr::From(func));

    // This incomplete solution only works for this type of member functions.
    // So the result must be an integer.
    auto result = new Result<int>();

    // We also send the paramters of the functions. For this incomplete
    // solution, it must be one integer.
    if (!Send(instance_id, func_id, new IntParam(x), result)) {
      // Fail to send, then delete the result and return nullptr.
      delete result;
      return nullptr;
    }
    return result;
  }
  /* add this */

  template <typename Svc>
  Result<void> *Call(Svc *svc, void (Svc::*func)()) {
    // Lookup instance and function IDs.
    int instance_id = svc->instance_id();
    int func_id = svc->LookupExportFunction(MemberFunctionPtr::From(func));

    // This incomplete solution only works for this type of member functions.
    // So the result must be an integer.
    auto result = new Result<void>();
    std::cout << "this is callvoid"<< "\n";

    // We also send the paramters of the functions. For this incomplete
    // solution, it must be one integer.
    if (!Send(instance_id, func_id, new Param<void>(), result)) {
      // Fail to send, then delete the result and return nullptr.
      delete result;
      return nullptr;
    }
    return result;
  }
  /* add this */
  template <typename Svc>
  Result<bool> *Call(Svc *svc, bool (Svc::*func)()) {
    // Lookup instance and function IDs.
    int instance_id = svc->instance_id();
    int func_id = svc->LookupExportFunction(MemberFunctionPtr::From(func));

    // This incomplete solution only works for this type of member functions.
    // So the result must be an integer.
    auto result = new Result<bool>();
    std::cout << "this is callbool"<< "\n";

    // We also send the paramters of the functions. For this incomplete
    // solution, it must be one integer.
    if (!Send(instance_id, func_id, new Param<void>(), result)) {
      // Fail to send, then delete the result and return nullptr.
      delete result;
      return nullptr;
    }
    return result;
  }

 template <typename Svc>
  Result<std::string> *Call(Svc *svc, std::string (Svc::*func)(std::string), std::string x) {
    std::cout << "this is call string to string"<< "\n";
    // Lookup instance and function IDs.
    int instance_id = svc->instance_id();
    int func_id = svc->LookupExportFunction(MemberFunctionPtr::From(func));

    // This incomplete solution only works for this type of member functions.
    // So the result must be an integer.
    auto result = new Result<std::string>();


    // We also send the paramters of the functions. For this incomplete
    // solution, it must be one integer.
    if (!Send(instance_id, func_id, new Param<std::string>(x), result)) {
      // Fail to send, then delete the result and return nullptr.
      delete result;
      return nullptr;
    }
    return result;
  }

template <typename Svc>
  Result<std::string> *Call(Svc *svc, std::string (Svc::*func)(unsigned int),unsigned int x) {
    // Lookup instance and function IDs.
    int instance_id = svc->instance_id();
    int func_id = svc->LookupExportFunction(MemberFunctionPtr::From(func));

    // This incomplete solution only works for this type of member functions.
    // So the result must be an integer.
    auto result = new Result<std::string>();
    std::cout << "this is call int to string"<< "\n";

    // We also send the paramters of the functions. For this incomplete
    // solution, it must be one integer.
    if (!Send(instance_id, func_id, new IntParam(x), result)) {
      // Fail to send, then delete the result and return nullptr.
      delete result;
      return nullptr;
    }
    return result;
  }
template <typename Svc, typename A, typename B, typename C>
  Result<A> *Call(Svc *svc, A (Svc::*func)(B,C),B ArgOne,C ArgTwo) {
    // Lookup instance and function IDs.
    int instance_id = svc->instance_id();
    int func_id = svc->LookupExportFunction(MemberFunctionPtr::From(func));

    // This incomplete solution only works for this type of member functions.
    // So the result must be an integer.
    auto result = new Result<A>();
    std::cout << "variadic 2 variable input"<< "\n";

    // We also send the paramters of the functions. For this incomplete
    // solution, it must be one integer.
    if (!Send(instance_id, func_id, new Param<B,C>(ArgOne, ArgTwo), result)) {
      // Fail to send, then delete the result and return nullptr.
      delete result;
      return nullptr;
    }
    return result;
  }

  template<typename Svc, typename RT, typename ... FA> 
  Result<RT> * Call(Svc *svc, RT (Svc::*f)(FA...), ...) {
    std::cout << "WARNING: Calling " 
          << typeid(decltype(f)).name()
          << " is not supported\n";
    return nullptr;
  }
  /* end here */
 // end of class Client
};

// TASK2: Server-side
template <typename Svc>
class Service : public BaseService {
 protected:
  void Export(int (Svc::*func)(int)) {
    ExportRaw(MemberFunctionPtr::From(func), new IntIntProcedure<Svc>());
  }
  void Export(void (Svc::*func)()) {
	std::cout<<"void to void";
    ExportRaw(MemberFunctionPtr::From(func), new VoidVoidProcedure<Svc>());
  }
  void Export(bool (Svc::*func)()) {
	std::cout <<"void to bool";
    ExportRaw(MemberFunctionPtr::From(func), new BoolVoidProcedure<Svc>());
  }
void Export(std::string (Svc::*func)(std::string)) {
	std::cout <<"string string";
    ExportRaw(MemberFunctionPtr::From(func), new StrStrProcedure<Svc>());
  }
void Export(std::string (Svc::*func)(unsigned int)) {
	std::cout << "string int" ;
    ExportRaw(MemberFunctionPtr::From(func), new StrIntProcedure<Svc>());
  }
void Export(std::string (Svc::*func)(std::string,int)) {
  std::cout << "string int" ;
  ExportRaw(MemberFunctionPtr::From(func), new StrStrIntProcedure<Svc>());
}
void Export(unsigned long (Svc::*func)(int,unsigned int)) {
  std::cout << "ulong, int uint" ;
  ExportRaw(MemberFunctionPtr::From(func), new ULongIntUIntProcedure<Svc>());
}
void Export(void (Svc::*func)(std::string, std::string)) {
  std::cout << "ulong, int uint" ;
  ExportRaw(MemberFunctionPtr::From(func), new VoidStrStrProcedure<Svc>());
}


  /* add this */
  template<typename MemberFunction>
  void Export(MemberFunction f) {
    ExportRaw(MemberFunctionPtr::From(f), new Procedure<Svc, MemberFunction>());
    std::cout << "WARNING: Exporting " 
              << typeid(MemberFunction).name()
              << " is not supported\n";
  }
  // void Export(MemberFunction a, Memberfunction b){
  //   ExportRaw(MemberFunctionPtr::From(a,b), new Procedure<Svc, MemberFunction>());
  //   std::cout << "WARNING: Exporting " 
  //             << typeid(MemberFunction).name()
  //             << " is not supported\n";
  // }
  /* end here */
 // end of class Service
};

}

#endif /* RPCXX_SAMPLE_H */

/*

*/
