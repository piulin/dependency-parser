//
// Created by pedro on 21/01/2020.
//

#include "set_parser.h"
#include "chu_liu_edmonds/chu_liu_edmonds.h"

using namespace parsers ;

set_parser::set_parser ( set::set const & s ) : s_ { s }  { }


set::set set_parser::parse ( std::function < units::sentence ( units::sentence const & , parsers::chu_liu_edmonds::stc_parser & ) > && parse_fn ) {
    std::vector < units::sentence > stcs { s_.sentences().size() } ;
    #pragma omp parallel for default (shared) // NOLINT(openmp-use-default-none)
    for ( size_t i = 0 ; i < s_.sentences().size() ; i++ ) {
        units::sentence const & stc = s_.sentences() [ i ] ;
        parsers::chu_liu_edmonds::stc_parser p { stc } ;
        stcs [ i ] = parse_fn ( stc, p ) ;
    }
    return set::set { stcs } ;
}
