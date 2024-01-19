#include <iostream>
#include "process.hpp"

void example1() {

	process_t proc("ls");

	std::cout << "\noutput of ls command:\n" << proc << std::endl;
	std::cout << "exit code: " << proc.status() << std::endl;
}

void example2() {

	process_t proc("/bin/sh", { "-c", "./test.sh" });

	std::cout << "\noutput of test.sh:\n" << proc[STREAM_OUT] << std::endl;
	std::cout << "\ntest.sh stderr: " << proc[STREAM_ERR] << std::endl;
	std::cout << "exit code: " << proc[STREAM_STATUS] << std::endl;
}

void example3() {

	std::string s;
	#if __cplusplus > 201803L
	s = " (c++20 method)";
	process_t *proc = new process_t("ls", "-l");
	#else
	process_t *proc = new process_t("ls", { "-l" });
	#endif

	std::cout << "\noutput of 'ls -l'" << s << ":\n" << (*proc) << std::endl;
	std::cout << "exit code: " << proc -> status() << std::endl;
	delete proc;
}

void example4() {

	process_t proc("sort", { "-r" });
	proc >> "a\nb\nc" >> std::endl;

	std::string out = proc;
	std::cout << "\noutput of 'sort -r' when a + b + c where sent to it's stdin:\n" <<
			out << std::endl;
	std::cout << "exit code: " << proc.status() << std::endl;
}

void example5() {

	process_t *sort = new process_t("sort", { "-r" });
	process_t *cat = new process_t("cat", {}, sort);
	(*cat) >> "a\nb\nc" >> std::endl;

	std::cout << "\noutput of sort with input from cat:\n" << sort << std::endl;
	std::cout << "exit code for sort: " << sort -> status() << std::endl;
	std::cout << "exit code for cat: " << cat -> status() << std::endl;
	delete cat;
	delete sort;
}

void example6() {

	process_t *echo = new process_t("echo", { "\"hello world\"" });
	std::string out = *echo;
	int code = *echo;
	std::cout << "\noutput of echo command:\n" << out << std::endl;
	std::cout << "exit code: " << code << std::endl;
}

void example7() {

	process_t proc("/bin/sh", { "-c", "./test.sh" });
	std::string out = proc[STREAM_OUT];
	std::string err = proc[STREAM_ERR];
	int code = proc[STREAM_STATUS];

	std::cout << "\noutput of test.sh:\n" << out << std::endl;
	std::cout << "\ntest.sh stderr: " << err << std::endl;
	std::cout << "exit code: " << code << std::endl;
}


int main(int argc, char **argv) {

	std::cout << "process test" << std::endl;

	example1();
	example2();
	example3();
	example4();
	example5();
	example6();
	example7();

	std::cout << "\nExiting.." << std::endl;
	return 0;
}
