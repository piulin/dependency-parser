//
// Created by pedro on 21/01/2020.
//

#include "assessment.h"

using namespace assessment ;

std::ostream & assessment::operator << ( std::ostream & o , assess_t const & ass ) {
    o << "precision: " << ass.precision << '\n'
      << "total: " << ass.total << '\n'
      << "correctly_predicted: " << ass.correctly_predicted << '\n' ;
    return o ;
}


template < typename C >
assess_t assess ( set::set const & gold, set::set const & pred, C && condition ) {
    auto const & gold_stcs = gold.sentences() ;
    auto const & pred_stcs = pred.sentences() ;
    size_t predicted = 0 ;
    size_t total = 0 ;

    if ( gold_stcs.size() != pred_stcs.size() ) {
        throw std::runtime_error ( "Assess failure: gold and pred sets do not "
                                   "present the same amount of sentences." ) ;
    }
    size_t const length = gold_stcs.size() ;

    for ( size_t i = 0 ; i < length ; ++i ) {
        auto const &gstc = gold_stcs[i];
        auto const &pstc = pred_stcs[i];

        size_t const tok_length = gstc.size() ;
        total += tok_length ;
        for ( size_t j = 0 ; j < tok_length ; ++j ) {
            predicted += condition(gstc.tokens()[j], pstc.tokens()[j]);
        }
    }

    return {static_cast< float > ( predicted )*100.0f / static_cast< float >( total ) , total, predicted } ;
}

assess_t assessment::uas ( set::set const & gold, set::set const & pred ) {
    return assess ( gold, pred , [ ] ( units::token const & tgold, units::token const & tpred ) -> int {
        return tgold.head_ == tpred.head_ ;
    } ) ;
}
assess_t assessment::las ( set::set const & gold, set::set const & pred ) {
    return assess ( gold, pred , [ ] ( units::token const & tgold, units::token const & tpred ) -> int {
        return tgold.head_ == tpred.head_ && tgold.rel_ == tpred.rel_ ;
    } ) ;
}