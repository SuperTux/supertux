#include <string>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <utility>


template <size_t STACK_BUFFER_SIZE=64>
std::string format( const char * fmt, ... )
{
    std::string result;
    char stack_buffer[STACK_BUFFER_SIZE];
    char *heap_buffer = 0;
    char *choosen_buffer = stack_buffer;

    va_list vl_original;
    va_list vl_working;
    va_list vl_working_2;

    va_start( vl_original, fmt );
    va_copy( vl_working, vl_original );
    va_copy( vl_working_2, vl_original );

    size_t chars_that_would_be_printed =
        vsnprintf ( stack_buffer, STACK_BUFFER_SIZE, fmt, vl_working );

    if( chars_that_would_be_printed >= STACK_BUFFER_SIZE )
    {
        unsigned heap_buffer_size = chars_that_would_be_printed+1;
        heap_buffer = new char[heap_buffer_size];
        vsnprintf ( heap_buffer, heap_buffer_size, fmt,  vl_working_2 );
        choosen_buffer = heap_buffer;
    }

    va_end( vl_working );
    va_end( vl_working_2 );
    va_end( vl_original );
    result.assign( choosen_buffer );
    if( heap_buffer )
        delete heap_buffer;
    return result;
}