#include "process.hpp"

process_t::STREAM::~STREAM() {

	if ( this -> in != nullptr )
		delete this -> in;
}
