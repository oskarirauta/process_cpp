#include <unistd.h>
#include "process.hpp"

static const int READ = 0;
static const int WRITE = 1;

process_t::PIPE::PIPE(process_t *out) {

	if ( out == nullptr ) {

		if ( ::pipe(this -> in) == -1 || ::pipe(this -> out) == -1 || ::pipe(this -> err) == -1 )
			throws << "failed to create pipe" << std::endl;

	} else {

		// route our stdout into the source process's stdin instead of a pipe of
		// our own; guard the lookup so a process without an input stream cannot
		// dereference a null filebuffer
		std::ostream* in_stream = out -> stream != nullptr ? out -> stream -> in : nullptr;
		filebuffer* fb = in_stream != nullptr ? dynamic_cast<filebuffer*>(in_stream -> rdbuf()) : nullptr;

		if ( fb == nullptr )
			throws << "failed to set up pipe, source process has no usable input stream" << std::endl;

		this -> out[READ] = -1;
		this -> out[WRITE] = fb -> fd();

		if ( ::pipe(this -> in) == -1 || ::pipe(this -> err) == -1 )
			throws << "failed to create pipe" << std::endl;
	}
}
