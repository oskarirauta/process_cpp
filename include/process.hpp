#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <ext/stdio_filebuf.h>
#include "throws.hpp"

enum STREAM_TYPE { STREAM_OUT = 0, STREAM_ERR = 1, STREAM_STATUS };

class process_t {

	public:
		using endl_type = std::ostream& (std::ostream&);

		struct OUTPUT;

		int status();
		OUTPUT out();
		OUTPUT err();

		template <typename T>
		constexpr process_t& operator >>(T x);
		process_t& operator >>(endl_type &endl);

		OUTPUT operator [](const STREAM_TYPE& type);
		operator int();
		operator std::string();

		friend std::ostream& operator <<(std::ostream& os, const process_t& proc);
		friend std::ostream& operator <<(std::ostream& os, const process_t *proc);
		friend std::ostream& operator <<(std::ostream& os, process_t::OUTPUT output);

		#if __cplusplus > 201803L
		process_t(const std::convertible_to<std::string> auto& ...args);
		process_t(const std::convertible_to<std::string> auto& ...args, process_t *input);
		#endif

		process_t(const std::string& cmd, const std::vector<std::string>& args = {}, process_t *input = nullptr);
		~process_t();

	private:

		using filebuffer = __gnu_cxx::stdio_filebuf<char>;
		std::string buf;

		struct PIPE;
		struct BUFFER;
		struct STREAM;

		pid_t pid = -1;
		int code = -1;
		std::vector<const char*> args;

		PIPE	*pipe = nullptr;
		BUFFER	*buffer = nullptr;
		STREAM	*stream = nullptr;

		void execute();
		void run_child();
};

class process_t::PIPE {

	friend class process_t;

	private:
		int		in[2];
		int		out[2];
		int		err[2];

		PIPE(process_t* out = nullptr);
};

class process_t::BUFFER {

	friend class process_t;

	private:
		filebuffer	*in;
		filebuffer	*out;
		filebuffer	*err;

		BUFFER() : in(nullptr), out(nullptr), err(nullptr) {}

	public:
		~BUFFER();
};

class process_t::STREAM {

	friend class process_t;

	private:
		std::ostream	*in;
		std::istream	*out;
		std::istream	*err;

		std::string	str_out();
		std::string	str_err();

		STREAM() : in(nullptr), out(nullptr), err(nullptr) {}

	public:
		~STREAM();

	friend std::ostream& operator <<(std::ostream& os, const process_t& proc);
	friend std::ostream& operator <<(std::ostream& os, const process_t *proc);
	friend std::ostream& operator <<(std::ostream& os, process_t::OUTPUT output);

};

class process_t::OUTPUT {

	friend class process_t;

	private:
		process_t *parent;
		STREAM_TYPE type;

		OUTPUT(process_t *parent, const STREAM_TYPE& type) : parent(parent), type(type) {}

	public:
		operator std::string();
		operator int();
		~OUTPUT();

	friend std::ostream& operator <<(std::ostream& os, process_t::OUTPUT output);
};

template <typename T>
constexpr process_t& process_t::operator >>(T x) {
	if ( this -> stream -> in != nullptr )
		(*this -> stream -> in) << x;
	return *this;
}

#if __cplusplus > 201803L

process_t::process_t(const std::convertible_to<std::string> auto& ...args) {

	for ( std::string arg : { std::string(args)... }) {
		this -> args.push_back(arg.c_str());
	}
	this -> args.push_back(nullptr);

	try {
		this -> pipe = new PIPE(); //(input);
	} catch ( const std::runtime_error& e ) {
		throws << e.what() << std::endl;
	}

	this -> buffer = new BUFFER;
	this -> stream = new STREAM;

	try {
		this -> execute();
	} catch ( std::runtime_error& e ) {
		throws << e.what() << std::endl;
	}
}

process_t::process_t(const std::convertible_to<std::string> auto& ...args, process_t *input) {

	for ( std::string arg : { std::string(args)... }) {
		this -> args.push_back(arg.c_str());
	}
	this -> args.push_back(nullptr);

	try {
		this -> pipe = new PIPE(input);
	} catch ( const std::runtime_error& e ) {
		throws << e.what() << std::endl;
	}

	this -> buffer = new BUFFER;
	this -> stream = new STREAM;

	try {
		this -> execute();
	} catch ( std::runtime_error& e ) {
		throws << e.what() << std::endl;
	}
}

#endif

std::ostream& operator <<(std::ostream& os, const process_t& proc);
std::ostream& operator <<(std::ostream& os, const process_t *proc);
std::ostream& operator <<(std::ostream& os, process_t::OUTPUT output);
