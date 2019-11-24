//
// Created by pedro on 12/11/19.
//

#ifndef C___PARSER_H
#define C___PARSER_H

#include "../units.h"
#include "arcstandard/model.h"


namespace parsers {

    class parser {
    public:
        virtual units::sentence parse ( parsers::arcstandard::model::model & m ) = 0 ;
    } ;

}

#endif //C___PARSER_H
