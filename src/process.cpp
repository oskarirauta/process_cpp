#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "process.hpp"

static const int READ = 0;
static const int WRITE = 1;

process_t::process_t(const std::string& cmd, const std::vector<std::string>& args, process_t *input) {

	this -> args = {{ cmd.c_str() }};

	std::transform(args.begin(), args.end(), std::back_inserter(this -> args),
		[](const std::string& s) { return s.c_str(); });
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
	} catch ( const std::runtime_error& e ) {
		throws << e.what() << std::endl;
	}
}

process_t::~process_t() {

	if ( this -> pipe != nullptr ) delete this -> pipe;
	if ( this -> buffer != nullptr ) delete this -> buffer;
	if ( this -> stream != nullptr ) delete this -> stream;
}

int process_t::status() {

	if ( this -> pid <= 0 )
		return -1;

	else if ( this -> code < 0 ) {

		::waitpid(pid, &this -> code, 0);

		if ( WIFEXITED(this -> code))
			this -> code = WEXITSTATUS(this -> code);
		else return -1;
	}
	return this -> code;
}

process_t::OUTPUT process_t::operator [](const STREAM_TYPE& type) {

	return OUTPUT(this, type);
}

process_t::operator int() {

	return this -> status();
}

process_t::operator std::string() {

	return this -> stream == nullptr ? "" : this -> stream -> str_out();
}

process_t::OUTPUT process_t::out() {

	return OUTPUT(this, STREAM_OUT);
}

process_t::OUTPUT process_t::err() {

	return OUTPUT(this, STREAM_ERR);
}

void process_t::execute() {

	pid_t parent = ::getpid();

	if ( this -> pid = ::fork(); this -> pid < 0 )
		throws << "failed to fork" << std::endl;

	else if ( this -> pid == 0 ) run_child();

	if ( parent != ::getpid())
		exit(EXIT_FAILURE);

	::close(this -> pipe -> in[READ]);
	::close(this -> pipe -> out[WRITE]);
	::close(this -> pipe -> err[WRITE]);

	this -> buffer -> in = new filebuffer(this -> pipe -> in[WRITE], std::ios_base::out, 1);
	this -> stream -> in = new std::ostream(this -> buffer -> in);

	if ( this -> pipe -> out[READ] != -1 ) {
		this -> buffer -> out = new filebuffer(this -> pipe -> out[READ], std::ios_base::in, 1);
		this -> stream -> out = new std::istream(this -> buffer -> out);
	}

	this -> buffer -> err = new filebuffer(this -> pipe -> err[READ], std::ios_base::in, 1);
	this -> stream -> err = new std::istream(this -> buffer -> err);
}

void process_t::run_child() {

	if ( ::dup2(this -> pipe -> in[READ], STDIN_FILENO) == -1 ||
		::dup2(this -> pipe -> out[WRITE], STDOUT_FILENO) == -1 ||
		::dup2(this -> pipe -> err[WRITE], STDERR_FILENO) == -1 ) {
		std::perror("failed to duplicate file descriptors");
		return;
	}

	::close(this -> pipe -> in[READ]);
	::close(this -> pipe -> in[WRITE]);

	if ( this -> pipe -> out[READ] != -1 )
		::close(this -> pipe -> out[READ]);
	::close(this -> pipe -> out[WRITE]);

	::close(this -> pipe -> err[READ]);
	::close(this -> pipe -> err[WRITE]);

	if ( ::execvp(this -> args[0], const_cast<char* const*>(this -> args.data())) == -1 ) {
		std::perror("failed to execute");
		return;
	}
}

process_t& process_t::operator >>(endl_type &endl) {

	if ( this -> stream -> in != nullptr )
		(*this -> stream -> in) << this -> buf << std::endl;

	this -> buffer -> in -> close();
	this -> buf = std::string();
	return *this;
}

std::ostream& operator <<(std::ostream& os, const process_t& proc) {

	if ( proc.stream != nullptr && proc.stream -> out != nullptr )
		os << proc.stream -> str_out();
	return os;
}

std::ostream& operator <<(std::ostream& os, const process_t *proc) {

	if ( proc != nullptr && proc -> stream != nullptr && proc -> stream -> out != nullptr )
		os << proc -> stream -> str_out();
	return os;
}
