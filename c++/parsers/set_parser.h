//
// Created by pedro on 12/11/19.
//

#ifndef C___SET_PARSER_H
#define C___SET_PARSER_H

#include "../units.h"
#include "../set.h"
#include "arcstandard/model.h"
#include "arcstandard/arcstandard.h"
#include "arcstandard/oracle.h"


namespace parsers {

    template < class T >
    class set_parser {
    public:
        explicit set_parser ( set::set const & s ) : s_ { s }  { }

        template < typename M >
        set::set parse ( M & m ) {
            std::vector < units::sentence > stcs { s_.sentences().size() } ;

//            #pragma omp parallel for default (shared)
            for ( size_t i = 0 ; i < s_.sentences().size() ; i++ ) {
                auto const & stc = s_.sentences() [ i ] ;
                T p { stc } ;
                stcs [ i ] = p.parse( m ) ;
            }
            return set::set {  stcs } ;
        }

        set::set parse ( std::function < units::sentence ( units::sentence const & , T & ) > && parse_fn ) {
            std::vector < units::sentence > stcs { s_.sentences().size() } ;
            #pragma omp parallel for default (shared) // NOLINT(openmp-use-default-none)
            for ( size_t i = 0 ; i < s_.sentences().size() ; i++ ) {
                units::sentence const & stc = s_.sentences() [ i ] ;
                T p { stc } ;
                stcs [ i ] = parse_fn ( stc, p ) ;
            }
            return set::set { stcs } ;
        }

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
