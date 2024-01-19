#include <unistd.h>
#include "process.hpp"

static const int READ = 0;
static const int WRITE = 1;

process_t::PIPE::PIPE(process_t *out) {

	if ( out == nullptr ) {

		if ( ::pipe(this -> in) == -1 || ::pipe(this -> out) == -1 || ::pipe(this -> err) == -1 )
			throws << "failed to create pipe" << std::endl;

	} else if ( out != nullptr ) {

		auto buffer = dynamic_cast<filebuffer*>(out -> stream -> in -> rdbuf());
		this -> out[READ] = -1;
		this -> out[WRITE] = buffer -> fd();

		if ( ::pipe(this -> in) == -1 || ::pipe(this -> err) == -1 )
			throws << "failed to create pipe" << std::endl;
	}
}
