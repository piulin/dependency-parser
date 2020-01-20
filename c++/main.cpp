
#include "options.h"
#include "engine.h"


int main ( int const argc, char ** argv ) {

    options::parse ( argc , argv ) ;
    engine::run ( ) ;

}