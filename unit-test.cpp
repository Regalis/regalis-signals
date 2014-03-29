#include <iostream>
#include <string>
#include <functional>
#include <assert.h>
#include "Signals.hpp"
using namespace std;
using namespace regalis;

class Monitor {
	protected:
		int string_count = 0;
		int int_int_count = 0;
		string str;
		int int_sum = 0;
	public:
		void slot_string(string s) {
			string_count++;
			str += s;
		}
		
		void slot_int_string(int x, string y) {
			str += y;
			int_sum += x;
		}

		void slot_int_int(int x, int y) {
			int_int_count++;
			int_sum += x + y;
		}

		int get_string_count() {
			return string_count;
		}

		int get_int_int_count() {
			return int_int_count;
		}

		int get_int_sum() {
			return int_sum;
		}

		string get_str() {
			return str;
		}
};

string slot_string_global;
void slot_string(string str) {
	slot_string_global += str;
}

int main() {
	Signal<string> s_string;
	Signal<int, int> s_int_int;
	Signal<int, string> s_int_string;
	Monitor a;
	Monitor b;

	auto slot_s_string = connect(s_string, slot_string);
	assert(slot_s_string.use_count() == 2);
	s_string("test_1");
	assert(slot_string_global == "test_1");

	
	auto slot_s_int_string = connect(s_int_string, a, &Monitor::slot_int_string);
	auto slot_s_int_string2 = connect(s_int_string, b, &Monitor::slot_int_string);
	s_int_string(1, "test_int_string");
	assert(a.get_int_sum() == 1);
	assert(a.get_str() == "test_int_string");
	assert(b.get_int_sum() == 1);
	assert(b.get_str() == "test_int_string");

	s_int_string.disconnect(slot_s_int_string2);
	assert(slot_s_int_string2.use_count() == 1);
	s_int_string(2, "test_int_string");
	assert(a.get_int_sum() == 3);
	assert(a.get_str() == "test_int_stringtest_int_string");
	assert(b.get_int_sum() == 1);
	assert(b.get_str() == "test_int_string");

	s_int_string.disconnect(slot_s_int_string);
	assert(slot_s_int_string.use_count() == 1);
	s_int_string(2, "test_int_string");
	assert(a.get_int_sum() == 3);
	assert(a.get_str() == "test_int_stringtest_int_string");
	
	s_int_string.connect(slot_s_int_string);
	s_int_string(3, "test_int_string_reconnect");
	assert(a.get_int_sum() == 6);
	assert(a.get_str() == "test_int_stringtest_int_stringtest_int_string_reconnect");
	assert(slot_s_int_string.use_count() == 2);

	auto slot_int_int = connect(s_int_int, a, &Monitor::slot_int_int);
	s_int_int(1, 1);
	assert(a.get_int_sum() == 8);
	s_int_int(-8, 0);
	assert(a.get_int_sum() == 0);
	s_int_int(2.5, 10.9999);
	assert(a.get_int_sum() == 12);

	return 0;
}
