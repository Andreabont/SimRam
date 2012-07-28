#include <iostream>
#include "lib.h"

bool error_registers(int n, int regnumber)
{

    if( n > ( regnumber - 1 ) )
    {
        std::cerr << "<!> Overflow. Register '" << n << "' does not exist." << std::endl;
        return true;
    }

    return false;

}
