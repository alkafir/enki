/*
 *  Enki: C++ 11 test suite
 *  Copyright (C) 2015  Alfredo Mungo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ****************************************************************************
 *
 *  File:           enki.h
 *
 *  Author:         Alfredo Mungo
 *  Creation date:  2015-02-05
 */
#ifndef _ENKI_TESTCASE_H
#define _ENKI_TESTCASE_H

#define ENKI_VERSION_MAJOR 0
#define ENKI_VERSION_MINOR 1
#define ENKI_VERSION_PATCH 1

#include <iostream>
#include <fstream>
#include <list>
#include <exception>
#include <functional>
#include <chrono>

#if !defined(ENKI_STYLE_NOCOLORS) && !defined(__WIN32)
  #define ENKI_STYLE_PASSED "\33[32m;" 
  #define ENKI_STYLE_FAILED "\33[31m;"
  #define ENKI_STYLE_DEFAULT "\33[0m;"
  #define ENKI_STR_PASSED "PASSED"
  #define ENKI_STR_FAILED "FAILED"
#else
  #define ENKI_STYLE_PASSED
  #define ENKI_STYLE_FAILED
  #define ENKI_STYLE_DEFAULT
  #define ENKI_STR_PASSED "passed"
  #define ENKI_STR_FAILED "FAILED"
#endif /* ENKI_STYLE_NOCOLORS */

/*
 * Main testsuite namespace.
 */
namespace enki {
  template<typename T> class ResultExporter;

  /*
   * Exception thrown to indicate that a test has failed.
   */
  class TestFailedException : public std::exception {
    public:
      TestFailedException() noexcept: exception() {}
      virtual const char* what() const noexcept { return "Test failed"; }
  };

  /*
   * Exception thrown to indicate that a test has passed.
   */
  class TestPassedException : public std::exception {
    public:
      TestPassedException() noexcept: exception() {}
      virtual const char* what() const noexcept { return "Test passed"; }
  };

  /*
   * This class provides facilities for assertions.
   *
   * All the methods inside this class do not return any value and
   * fail the test if the asserted condition is not met.
   */
  class Assert {
    public:
      /*
       * Asserts that a condition is true.
       *
       * condition: The condition
       */
      static void assert(bool condition) { if(!condition) throw TestFailedException(); }

      /*
       * Asserts that no exception is thrown.
       *
       * func: The function to test
       */
      static void assert_exception(typename std::function<void(void)> func) { try { func(); } catch(...) { throw TestFailedException(); } }

      /*
       * Asserts that two arrays are equivalent. Two arrays are
       * said to be equivalent when they have the same elements
       * in the same order.
       *
       * a: The first array
       * b: The second array
       * len_a: The length of a
       * len_b: The length of b
       *
       * T: The domain type of both arrays
       */
      template<typename T> static void assert_array_equals(const T* a, size_t len_a, const T* b, size_t len_b) {
        assert(len_a == len_b);
        
        for(size_t t = 0; t < len_a; t++)
          if(a[t] != b[t])
            throw TestFailedException();
      }

      /*
       * Asserts that all the elements in the given array are in the subdomain [min, max] from the domain T.
       *
       * arr: The array
       * len_a: The length of the array
       * min: The minimum value (inclusive)
       * max: The maximum value (inclusive)
       *
       * T: The domain type
       */
      template<typename T> static void assert_array_subdomain(const T* arr, size_t len_a, const T& min, const T& max) {
        for(size_t t = 0; t < len_a; t++)
          if(arr[t] < min || arr[t] > max)
            throw TestFailedException();
      }
  };

  /*
   * Test case class. Subclasses of this class hold the test code and data.
   *
   * This class itself is responsible for running the setup/cleanup functions,
   * test running and success/failure identification, test data storage.
   *
   * T: The type of the child class
   */
  template<typename T> class TestCase {
    public:
      typedef void (T::*TestFunc)(); /* Test function type */

      /* Test data structure */
      typedef struct _TestData {
        TestFunc func; /* Test function */
        const char* name; /* Test name */
        bool passed; /* Test result */
        double time; /* Test duration in seconds */
      } TestData;

      /*
       * Setup function. Override this function to setup an environment for the test case.
       */
      virtual void setup() {}

      /*
       * Cleanup function. Override this function to cleanup the environment after the test
       * case completes.
       */
      virtual void cleanup() {}

      /*
       * Schedule a test for running.
       *
       * test: The test function
       * name: The test name
       */
      void add(TestFunc test, const char* name) {
        data.push_back({
          test, /* Test function */
          name, /* Test name */
          false, /* Test passed? */
          0.0 /* Test duration */
        });
      }

      /*
       * Runs the tests and stores the results.
       *
       * Return value: true if all the tests passed, false if not
       */
      bool run() {
        bool err = false; /* Did any test fail? */
        setup();

        for(typename std::list<TestData>::iterator it = data.begin(); it != data.end(); it++) {
          try {
            register TestFunc func = (*it).func;
            register T* cls = static_cast<T*>(this);

            {
              using namespace std::chrono;
              time_point<high_resolution_clock> t1, t2;
              
              t1 = high_resolution_clock::now();
              (cls->*func)();
              t2 = high_resolution_clock::now();

              (*it).time = duration_cast<duration<float>>(t2 - t1).count();
            }

            (*it).passed = true;
          } catch(enki::TestFailedException e) {
            (*it).passed = false;
            err = true;
          } catch(enki::TestPassedException e) {
            (*it).passed = true;
          }
        }

        cleanup();

        return err;
      }

      /*
       * Successfully passes the running test.
       */
      void pass() const { throw TestPassedException(); }

      /*
       * Fails the running test.
       */
      void fail() const { throw TestFailedException(); }

      /*
       * Returns the test data.
       *
       * Return value: The test data
       */
      std::list<TestData>& get_data() { return data; }
    private:
      std::list<TestData> data; /* Test data */
  };
  
  /*
   * Result exporter class. Subclasses of this class are responsible for
   * exporting the data to a defined medium into a defined format.
   *
   * T: The test case class type to export
   */
  template<typename T> class ResultExporter {
    public:
      /*
       * Initializes a new instance of this class.
       *
       * export_time: True to also export time information
       */
      ResultExporter(bool export_time): export_test_durations(export_time) {}

      /*
       * The general contract for this method is to export all the data of the
       * given test case.
       *
       * The default implementation exports each result through the
       * export_result() function.
       *
       * tcase: The testcase to export the data of
       */
      virtual void export_results(TestCase<T>& tcase) {
        typedef typename std::list<typename TestCase<T>::TestData>::iterator qiterator;

        for(qiterator it = tcase.get_data().begin(); it != tcase.get_data().end(); it++)
          export_result(*it);
      }

      /*
       * The general contract for this method is to export the information for
       * a single test.
       *
       * data: The test data structure to export
       * */
      virtual void export_result(typename TestCase<T>::TestData& data) = 0;

    private:
      bool export_test_durations; /* True to export test duration data */

    protected:
      /*
       * Returns the value of the export_test_durations property.
       *
       * Return value: True if the test duration data has to be exported
       */
      bool is_duration_exported() const { return export_test_durations; }
  };

  template<typename T> class StreamResultExporter: public ResultExporter<T> {
    public:
      /*
       * Initializes a new instance of this class.
       *
       * ostream: The stream to export the data to
       * export_time: True to also export the test duration data
       */
      StreamResultExporter(std::ostream& ostream, bool export_time = false): ResultExporter<T>(export_time), os(ostream) {}

      virtual void export_result(typename TestCase<T>::TestData& data) = 0;

    protected:
      /*
       * Gets the output stream.
       *
       * Return value: The output stream
       */
      inline std::ostream& get_output_stream() { return os; }

    private:
      std::ostream& os; /* The output stream */
  };

  /*
   * Text stream result exporter.
   *
   * This class exports the test data to a text stream in pure text format.
   *
   * T: The type of test case to export
   */
  template<typename T> class TextStreamResultExporter: public StreamResultExporter<T> {
    public:
      /*
       * Initializes a new instance of this class.
       *
       * ostream: The stream to export the data to
       * export_time: True to also export the test duration data
       */
      TextStreamResultExporter(std::ostream& ostream, bool export_time = false): StreamResultExporter<T>(ostream, export_time) {}

      /*
       * Exports a test result.
       *
       * Each test result is exported in the format:
       * [RESULT] duration_data test_name
       *
       * Where RESULT is the word "passed" or "failed" (case can vary),
       * duration_data is the duration information, and test_name
       * is the test name.
       *
       * data: The test data structure to export
       */
      virtual void export_result(typename TestCase<T>::TestData& data) {
        auto& os = this->get_output_stream();

        os << "[" << (data.passed? ENKI_STYLE_PASSED ENKI_STR_PASSED ENKI_STYLE_DEFAULT: ENKI_STYLE_FAILED ENKI_STR_FAILED ENKI_STYLE_DEFAULT) << "] ";

        if(this->is_duration_exported()) {
          os.width(8);
          os << data.time << "s ";
        }

        os << data.name << std::endl;
      }
  };
 
  /*
   * This class exports the test results to stdout.
   *
   * T: The type of test case to export
   */
  template<typename T> class ConsoleResultExporter: public TextStreamResultExporter<T> {
    public:
      /*
       * Initializes a new instance of this class.
       *
       * export_time: True to also export duration information
       */
      ConsoleResultExporter(bool export_time = false): TextStreamResultExporter<T>(std::cout, export_time) {}
  };

  /*
   * This class exports the test results to a text file.
   *
   * T: The type of test case to export
   */
  template<typename T> class TextFileResultExporter: public TextStreamResultExporter<T> {
    public:
      /*
       * Initializes a new instance of this class.
       *
       * fname: The file name
       * export_time: True to also export duration information
       */
      TextFileResultExporter(const char* fname, bool export_time = false): ofstream(fname), TextStreamResultExporter<T>(ofstream, export_time) {}

    private:
      std::ofstream ofstream; /* The file output stream */
  };

  /*
   * XML stream result exporter.
   *
   * This class exports the test data to a text stream in XML format.
   *
   * T: The type of test case to export
   */
  template<typename T> class XMLStreamResultExporter: public StreamResultExporter<T> {
    public:
      /*
       * Initializes a new instance of this class.
       *
       * ostream: The stream to export the data to
       * export_time: True to also export the test duration data
       */
      XMLStreamResultExporter(std::ostream& ostream, bool export_time = false): StreamResultExporter<T>(ostream, export_time) {
        /* Export XML header */
        this->get_output_stream() << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<test-results>\n";
      }

      virtual ~XMLStreamResultExporter() {
        /* Export XML footer */
        this->get_output_stream() << "</test-results>" << std::endl;
      }

      /*
       * See ResultExporter::export_result()
       */
      virtual void export_result(typename TestCase<T>::TestData& data) {
        auto& os = this->get_output_stream();

        os << "\t\t<test result=\"" << (data.passed? "passed": "failed") << "\"";

        if(this->is_duration_exported())
          os << " duration=\"" << data.time << "\"";

        os << " name=\"" << data.name << "\"/>" << std::endl;
      }

      /*
       * Exports the results in XML format.
       *
       * tcase: The testcase to export the data of
       */
      virtual void export_results(TestCase<T>& tcase) {
        typedef typename std::list<typename TestCase<T>::TestData>::iterator qiterator;
        auto& os = this->get_output_stream();

        /* Testcase header */
        os << "\t<test-case>\n";

        /* Data */
        for(qiterator it = tcase.get_data().begin(); it != tcase.get_data().end(); it++)
          this->export_result(*it);

        /* Testcase footer */
        os << "\t</test-case>\n";
      }
  };

  /*
   * XML file result exporter.
   *
   * This class exports the test data to a text file in XML format.
   *
   * T: The type of test case to export
   */
  template<typename T> class XMLFileResultExporter: public ResultExporter<T> {
    public:
    /*
     * Initializes a new instance of this class.
     *
     * fname: The file name
     * export_time: True to also export duration information
     */
    XMLFileResultExporter(const char* fname, bool export_time = false): ofstream(fname), ResultExporter<T>(export_time) {
      exp = new XMLStreamResultExporter<T>(ofstream, export_time);
    }

    virtual ~XMLFileResultExporter() {
      delete exp;
    }

    /*
     * See ResultExporter::export_results()
     */
    virtual void export_results(TestCase<T> tcase) {
      exp->export_results(tcase);
    }

    /*
     * The general contract for this method is to export the information for
     * a single test.
     *
     * data: The test data structure to export
     * */
    virtual void export_result(typename TestCase<T>::TestData& data) {
      exp->export_result(data);
    }

    private:
      std::ofstream ofstream; /* The file output stream */
      XMLStreamResultExporter<T>* exp; /* The XML stream exporter */
  };
}

#endif /* _ENKI_TESTCASE_H */
