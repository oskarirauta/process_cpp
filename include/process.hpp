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
		std::vector<std::string> arg_storage;	// owns the argument strings ...
		std::vector<const char*> args;		// ... that args points into

		PIPE	*pipe = nullptr;
		BUFFER	*buffer = nullptr;
		STREAM	*stream = nullptr;

		// stdout/stderr are drained eagerly into these buffers by collect();
		// reading lazily while status()/the destructor waits would deadlock as
		// soon as a child fills the (~64 KiB) pipe buffer
		mutable std::string out_buf;
		mutable std::string err_buf;
		mutable bool collected = false;

		void execute();
		void run_child();
		void collect() const;			// drain stdout+stderr before waiting
		std::string str_out() const;
		std::string str_err() const;
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

		BUFFER() : in(nullptr) {}

	public:
		~BUFFER();
};

class process_t::STREAM {

	friend class process_t;

	private:
		std::ostream	*in;

		STREAM() : in(nullptr) {}

	public:
		~STREAM();
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

	this -> arg_storage = { std::string(args)... };
	for ( const std::string& arg : this -> arg_storage )
		this -> args.push_back(arg.c_str());
	this -> args.push_back(nullptr);

	try {
		this -> pipe = new PIPE();
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
