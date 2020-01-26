//
// Created by pedro on 7/11/19.
//

#ifndef C___ASSESSMENT_H
#define C___ASSESSMENT_H

#include "set.h"

namespace assessment {

    struct assess_t {
        float precision ;
        size_t total ;
        size_t correctly_predicted ;
    } ;

    std::ostream & operator << ( std::ostream & o , assess_t const & ass ) ;

    assess_t uas ( set::set const & gold, set::set const & pred ) ;
    assess_t las ( set::set const & gold, set::set const & pred ) ;





}

#endif //C___ASSESSMENT_H
