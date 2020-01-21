//
// Created by pedro on 12/11/19.
//

#ifndef C___SET_PARSER_H
#define C___SET_PARSER_H

#include "../units.h"
#include "../set.h"
#include "chu_liu_edmonds/chu_liu_edmonds.h"


namespace parsers {

    class set_parser {
    public:
        explicit set_parser ( set::set const & s ) ;

//        template < typename M >
//        set::set parse ( M & m ) ;

        set::set parse ( std::function < units::sentence ( units::sentence const & , parsers::chu_liu_edmonds::stc_parser & ) > && parse_fn ) ;
        ~set_parser () = default;
        set_parser (const set_parser & other) = delete;
        set_parser (set_parser && other) = delete;
        set_parser & operator=(const set_parser & other) = delete;
        set_parser & operator=(set_parser && other) = delete;

    private:
        set::set const &    s_ ;
    };


}


#endif //C___SET_PARSER_H
