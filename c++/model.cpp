//
// Created by pedro on 12/11/19.
//

#include "model.h"


model::tr_model::tr_model ( std::vector < std::array < float , 3 > > transitions ) :
trans_ { std::move(transitions) },
ctr_ { 0 }
{   }

std::array<float, 3> model::tr_model::eval() {
return  trans_ [ ctr_++ ] ;
}

model::rd_model::rd_model () { srand( time(nullptr) ) ; }

std::array<float, 3>  model::rd_model::eval ( ) {
return {static_cast<float>( rand() )/ static_cast<float>( RAND_MAX ),
static_cast<float>( rand() )/ static_cast<float>( RAND_MAX ),
static_cast<float>( rand() )/ static_cast<float>( RAND_MAX ) } ;

}



