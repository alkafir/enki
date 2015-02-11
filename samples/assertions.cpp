#include <cstring>
#include <thread>
#include "../src/enki.h"

using namespace enki;

class AssertTestCase : public TestCase<AssertTestCase> {
  public:
    AssertTestCase() {
      add(test_assert, "Assert::assert()");
      add(test_assert_exception, "Assert::assert_exception()");
      add(test_assert_array_equals_pass, "Assert::assert_array_equals() pass");
      add(test_assert_array_equals_fail, "Assert::assert_array_equals() fail");
      add(test_assert_array_subdomain_pass, "Assert::assert_array_subdomain() pass");
      add(test_assert_array_subdomain_fail, "Assert::assert_array_subdomain() fail");
      add(test_wait_1s, "Timing test, 666ms ");
    }

    void test_assert() {
      Assert::assert(true == !false);
    }

    void test_assert_exception() {
      auto f = [] { throw std::exception(); };

      Assert::assert_exception(f);
    }

    void test_assert_array_equals_pass() {
      int a[] = {1, 2, 3, 4, 5};
      int b[] = {1, 2, 3, 4, 5};

      Assert::assert_array_equals<int>(a, 5, b, 5);
    }

    void test_assert_array_equals_fail() {
      int a[] = {1, 2, 3, 4, 5};
      int b[] = {1, 2, 3, 4, 6};

      Assert::assert_array_equals<int>(a, 5, b, 5);
    }

    void test_assert_array_subdomain_pass() {
      char arr[] = "abcdefghijklmnopqrstuvwxyz";

      Assert::assert_array_subdomain(arr, strlen(arr), 'a', 'z');
    }

    void test_assert_array_subdomain_fail() {
      char arr[] = "abcdefghijklmnopqrstuvwxy1";

      Assert::assert_array_subdomain(arr, strlen(arr), 'a', 'z');
    }

    void test_wait_1s() {
      using std::chrono::duration;
      using std::chrono::milliseconds;
      
      milliseconds ms(666);
      duration<float> d(ms);

      std::this_thread::sleep_for(d);
    }
};

int main(int argc, char** argv) {
  AssertTestCase tcase;

  int ret = tcase.run()? 0: 1;

  ConsoleResultExporter<AssertTestCase> exp(true);
  exp.export_results(tcase);

  // TextFileResultExporter<AssertTestCase> exp2(__BASE_FILE__ ".log", true);
  // exp2.export_results(tcase);
  

  // XMLFileResultExporter<AssertTestCase> exp3(__BASE_FILE__ ".xml", true);
  // exp3.export_results(tcase);

  return ret;
}
