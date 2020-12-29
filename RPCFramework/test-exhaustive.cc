#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <array>
#include <dlfcn.h>

#include "gtest/gtest.h"

#define GEN_FILE "__test.cc"

namespace {

struct TypeIface {
  const std::string name;
  virtual std::string RandomLiteral() const = 0;
  virtual ~TypeIface() {}
  TypeIface(std::string name) : name(name) {}
};

template <typename ...T> struct CreateType;

template <typename T, typename ...Others>
struct CreateType<T, Others...> {
  static TypeIface *New(int offset) {
    if (offset == 0) return new T();
    return CreateType<Others...>::New(offset - 1);
  }
  static TypeIface *New() {
    return New(rand() % (1 + sizeof...(Others)));
  }
};

template <>
struct CreateType<> {
  static TypeIface *New(int offset) {
    return nullptr;
  }
};

struct IntType : public TypeIface {
  IntType() : TypeIface("int") {}
  std::string RandomLiteral() const override { return std::to_string(rand()); }
};

struct LongType : public TypeIface {
  LongType() : TypeIface("long") {}
  std::string RandomLiteral() const override {
    long l = rand();
    l <<= 32;
    l |= rand();
    std::string r = std::to_string(l);
    r += "L";
    return r;
  }
};

struct StringType : public TypeIface {
  StringType() : TypeIface("std::string") {}
  std::string RandomLiteral() const override {
    std::stringstream s;
    s << "std::string(\"";
    int l = rand() % 24 + 8;
    for (int i = 0; i < l; i++) {
   retry:
      char ch = (rand() % (0x7F - 0x20) + 0x20);
      if (ch == '"' || ch == '\\') goto retry;

      s << ch;
    }
    s << "\")";
    return s.str();
  }
};

using RandomTypeFactory = CreateType<IntType, LongType, StringType>;

class CodeGen {
  static constexpr size_t kMaxParams = 32;
  static constexpr int kNrMethods = 16;

  std::string klass_name;
  int instance_id;
  struct Method {
    TypeIface *r_type;
    std::string r_value;
    std::string name;
    int nr_params;
    TypeIface *p_types[];   // flexible array member

    Method(std::string name) : name(name), nr_params(0) {}
    ~Method() {
        delete r_type;
        for (int i = 0; i < nr_params; i++) {
            delete p_types[i];
        }
    }
  };
  std::array<Method *, kNrMethods> methods;
  size_t nr_methods;
 public:
  CodeGen(std::string name);
  ~CodeGen();
  
  void WriteTo(const char *filename) {
    std::ofstream fout(filename);
    fout << s.rdbuf();
  }
  void GenClass();
  void GenTest();
 private:
  void GenCtor();
  void GenMethods();
  void GenMethod(const Method *);
  std::stringstream s;
};

CodeGen::CodeGen(std::string name)
    : klass_name(name), nr_methods(0)
{
  // Generate all methods!

  // We don't need random function names, let's name them Methond<ID>
  for (int i = 0; i < kNrMethods; i++) {
    auto nr_params = rand() % kMaxParams;

    auto m = new (malloc(sizeof(Method) + nr_params * sizeof(TypeIface *)))
             Method(std::string("Method") + std::to_string(i));
    methods[nr_methods++] = m;
    m->r_type = RandomTypeFactory::New();
    m->nr_params = nr_params;
    for (int j = 0; j < m->nr_params; j++) {
      m->p_types[j] = RandomTypeFactory::New();
    }
    m->r_value = m->r_type->RandomLiteral();
  }
  instance_id = rand() % 32768;
}

CodeGen::~CodeGen() 
{
    for (decltype(nr_methods) i = 0; i < nr_methods; i++) {
        delete methods[i];
    }
}

void CodeGen::GenClass()
{
  s << "struct " << klass_name << " : public rpc::Service<" << klass_name << "> {" << std::endl;
  GenCtor();
  GenMethods();
  s << "};" << std::endl;
}

void CodeGen::GenCtor()
{
  s << "  " << klass_name << "() {" << std::endl;
  std::for_each(methods.begin(), methods.begin() + nr_methods, [this](Method *m) {
    s << "    Export(&" << klass_name << "::" << m->name << ");" << std::endl;
  });
  s << "  }" << std::endl;
}

void CodeGen::GenMethods()
{
  std::for_each(methods.begin(), methods.begin() + nr_methods, [this](Method *m) {
    this->GenMethod(m);
  });
}

void CodeGen::GenMethod(const Method *m)
{
  s << "  " << m->r_type->name << " " << m->name << "(";
  if (m->nr_params == 0) {
    s << ") ";
  } else {
    std::string names[m->nr_params];
    for (int i = 0; i < m->nr_params; i++) {
      s << m->p_types[i]->name << ((i == m->nr_params - 1) ? ") " : ", ");
    }
  }
  s << "{ return " << m->r_value << "; }" << std::endl;
}

void CodeGen::GenTest()
{
  s << "struct Test : public ServiceTestUtil { void Run() {" << std::endl;
  s << "SetUpServer(); srv->AddService(new " << klass_name << "(), " << instance_id << ");" << std::endl;
  s << "SetUpClient(); auto s = new " << klass_name << "(); s->set_instance_id(" << instance_id << ");" << std::endl;

  for (size_t i = 0; i < nr_methods; i++) {
    s << "{";
    s << "  auto r = client->Call(s, &" << klass_name << "::" << methods[i]->name;
    for (int j = 0; j < methods[i]->nr_params; j++) {
      s << ", " << methods[i]->p_types[j]->RandomLiteral();
    }
    s << ");" << std::endl
      << "  client->Flush();" << std::endl;
    s << "  assert(!r->has_error()); assert(r->data() == " << methods[i]->r_value << ");" << std::endl;
    s << "  delete r;" << std::endl;
    s << "}" << std::endl;
  }

  s << "TearDownServer(); TearDownClient();" << std::endl;
  s << "delete s;" << std::endl;    // used by client
  s << "}};" << std::endl;

  s << "extern \"C\" void __invoke() { Test().Run(); }" << std::endl;
}

class RandomTest : public testing::Test {
 public:
  void SetUp() override {
    srand(time(NULL));

    CodeGen gen("TestClass");
    gen.GenClass();
    gen.GenTest();
    gen.WriteTo(GEN_FILE);
  }
};

TEST_F(RandomTest, RandomGeneratedCode)
{
  // Let's build a shared-object instead an executable! This is because they can
  // gracefully terminate the process using exit(0) and avoiding all assertions.

  ASSERT_EQ(
      0,
      system("g++ -g --std=c++11 -include test-rpc-common.h -include cassert "
        "-include rpc.cc " GEN_FILE " -fPIC -pthread -shared -o gen.so"))
      << "Your code cannot compile! Check with " GEN_FILE;

  void *dlh = dlopen("./gen.so", RTLD_NOW);
  ASSERT_NE(dlh, nullptr);

  auto invoke_fun = (void (*)()) dlsym(dlh, "__invoke");
  ASSERT_NE(invoke_fun, nullptr);
  invoke_fun();
  dlclose(dlh);
}

}
