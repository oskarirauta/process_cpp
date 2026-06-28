#include "process.hpp"

process_t::BUFFER::~BUFFER() {

	if ( this -> in != nullptr ) {
		this -> in -> close();
		delete this -> in;
	}
}
