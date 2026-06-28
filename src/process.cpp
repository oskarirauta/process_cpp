#include <iostream>
#include <algorithm>
#include <cerrno>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "process.hpp"

static const int READ = 0;
static const int WRITE = 1;

static std::string trim_end(std::string s) {

	while ( !s.empty() && std::string(" \t\n\r\f\v").find_first_of(s.back()) != std::string::npos )
		s.pop_back();
	return s;
}

process_t::process_t(const std::string& cmd, const std::vector<std::string>& args, process_t *input) {

	this -> arg_storage.push_back(cmd);
	for ( const std::string& arg : args )
		this -> arg_storage.push_back(arg);

	for ( const std::string& arg : this -> arg_storage )
		this -> args.push_back(arg.c_str());
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

	this -> status();
	if ( this -> pipe != nullptr ) delete this -> pipe;
	if ( this -> buffer != nullptr ) delete this -> buffer;
	if ( this -> stream != nullptr ) delete this -> stream;
}

// Drain stdout and stderr into out_buf/err_buf. This MUST happen before waiting
// for the child: a child that writes more than the pipe buffer holds blocks on
// write until a reader drains it, so waiting first would deadlock. Both fds are
// polled together so a full stderr cannot stall draining of stdout.
void process_t::collect() const {

	if ( this -> collected )
		return;
	this -> collected = true;

	// close our copy of the child's stdin so a child that reads until EOF finishes
	if ( this -> buffer != nullptr && this -> buffer -> in != nullptr )
		this -> buffer -> in -> close();

	if ( this -> pipe == nullptr )
		return;

	struct pollfd pfd[2];
	pfd[0].fd = this -> pipe -> out[READ];	// -1 when stdout is piped to another process
	pfd[1].fd = this -> pipe -> err[READ];
	pfd[0].events = pfd[1].events = POLLIN;

	char tmp[4096];

	while ( pfd[0].fd != -1 || pfd[1].fd != -1 ) {

		if ( ::poll(pfd, 2, -1) < 0 ) {
			if ( errno == EINTR )
				continue;
			break;
		}

		for ( int i = 0; i < 2; i++ ) {

			if ( pfd[i].fd == -1 || ( pfd[i].revents & ( POLLIN | POLLHUP | POLLERR )) == 0 )
				continue;

			ssize_t n = ::read(pfd[i].fd, tmp, sizeof(tmp));

			if ( n > 0 )
				( i == 0 ? this -> out_buf : this -> err_buf ).append(tmp, (size_t)n);
			else {	// EOF or error: stop watching this fd
				::close(pfd[i].fd);
				pfd[i].fd = -1;
			}
		}
	}
}

std::string process_t::str_out() const {

	this -> collect();
	return trim_end(this -> out_buf);
}

std::string process_t::str_err() const {

	this -> collect();
	return trim_end(this -> err_buf);
}

int process_t::status() {

	this -> collect();	// drain output first so waitpid cannot deadlock

	if ( this -> pid <= 0 )
		return -1;

	else if ( this -> code < 0 ) {

		::waitpid(this -> pid, &this -> code, 0);

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

	return this -> str_out();
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

	// stdout (pipe->out[READ]) and stderr (pipe->err[READ]) are read straight
	// from the pipe fds by collect()
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

	(void)endl;
	if ( this -> stream -> in != nullptr )
		(*this -> stream -> in) << this -> buf << std::endl;

	if ( this -> buffer -> in != nullptr )
		this -> buffer -> in -> close();
	this -> buf = std::string();
	return *this;
}

std::ostream& operator <<(std::ostream& os, const process_t& proc) {

	os << proc.str_out();
	return os;
}

std::ostream& operator <<(std::ostream& os, const process_t *proc) {

	if ( proc != nullptr )
		os << proc -> str_out();
	return os;
}
