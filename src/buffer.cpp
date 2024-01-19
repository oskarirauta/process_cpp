#include "process.hpp"

process_t::BUFFER::~BUFFER() {

	if ( this -> in != nullptr )
		this -> in -> close();

	if ( this -> in != nullptr ) delete this -> in;
	if ( this -> out != nullptr ) delete this -> out;
	if ( this -> err != nullptr ) delete this -> err;
}
