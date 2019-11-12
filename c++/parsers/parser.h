//
// Created by pedro on 12/11/19.
//

#ifndef C___PARSER_H
#define C___PARSER_H

#include "../units.h"
#include "../set.h"
#include "../model.h"
#include "arcstandard/arcstandard.h"
#include "arcstandard/oracle.h"


namespace parsers {


    class parser {
    public:
        explicit parser (set::set const & s ) : s_ { s } { }

        set::set parse ( model::model & m ) {
            std::vector < units::sentence > stcs { s_.sentences().size() } ;

            //#pragma omp parallel for default (shared)
            for ( size_t i = 0 ; i < s_.sentences().size() ; i++ ) {
                auto const & stc = s_.sentences() [ i ] ;
                arcstandard::stc_parser p { stc } ;
                stcs [ i ] = p.parse( m ) ;
            }
            return set::set {  stcs } ;
        }

        set::set oracle_parse ( ) {
            std::vector < units::sentence > stcs { s_.sentences().size() } ;
            #pragma omp parallel for default (shared)
            for ( size_t i = 0 ; i < s_.sentences().size() ; i++ ) {
                auto const & stc = s_.sentences() [ i ] ;
                arcstandard::stc_parser p { stc } ;
                auto so = arcstandard::smart_oracle { stc, p } ;
                arcstandard::oracle_model m { so } ;
                stcs [ i ] = p.parse( m ) ;
            }
            return set::set { stcs } ;
        }

        ~parser () = default;
        parser (const parser & other) = delete;
        parser (parser && other) = delete;
        parser & operator=(const parser & other) = delete;
        parser & operator=(parser && other) = delete;

    private:
        set::set const & s_ ;
    };


}


#endif //C___PARSER_H
