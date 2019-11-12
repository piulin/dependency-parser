//
// Created by pedro on 9/11/19.
//

#ifndef C___ORACLE_H
#define C___ORACLE_H

#include "../../units.h"
#include <vector>
#include <set>
#include "arcstandard.h"

namespace parsers::arcstandard {
    enum class transition {
        LEFT_ARC ,
        RIGTH_ARC,
        SHIFT
    } ;

    class smart_oracle {
    public:
        explicit smart_oracle ( units::sentence const & stc, stc_parser const & parser )
                : p_ { parser } {
            gold_arcs_ [ &stc.root ( ) ] = { } ;
            for ( auto const & tok : stc.tokens ( ) ) {
                gold_arcs_ [ &tok ] = { } ;
            }

            for ( auto const & tok : stc.tokens() ) {
                int const & head = tok.head_ ;
                if ( head == 0 /* that is, root */ ) {
                    gold_arcs_ [ &stc.root ( ) ].insert( &tok ) ;
                } else {
                    gold_arcs_ [ &stc.tokens ( ) [ head - 1 ] ].insert( &tok );
                }
            }
        }

        ~smart_oracle () = default;
        smart_oracle (const smart_oracle & other) = delete ;
        smart_oracle (smart_oracle && other) = delete ;
        smart_oracle & operator=(const smart_oracle & other) = delete ;
        smart_oracle & operator=(smart_oracle && other) = delete ;

        inline bool terminal_state ( ) const {
            return p_.curr_state().buf.empty() ;
        }

        inline bool should_leftarc ( ) const {
            if ( p_.can_leftarc() ) {
                auto const & state = p_.curr_state();
                auto const & arcs = gold_arcs_.at(state.buf.back());
                return std::find(arcs.begin(), arcs.end(), state.stk.top()) != arcs.end();
            }
            return false ;

        }

        inline bool should_rightarc ( ) const {
            if ( p_.can_rightarc ( ) ) {
                auto const & state = p_.curr_state ( ) ;
                auto const & arcs = gold_arcs_.at( state.stk.top ( ) ) ;
                auto e = std::find ( arcs.begin(),arcs.end(), state.buf.back() ) ;
                auto a = has_all_children( state.buf.back() ) ;
                return e != arcs.end() && a ;
            }
            return false ;
        }

        inline bool has_all_children ( units::token const * tk ) const {
            auto const & arc = p_.curr_state().arcs ;
            auto pos = arc.find ( tk ) ;
            return ( pos != arc.end() && gold_arcs_.at( tk ) == arc.at( tk ) ) ||
                   ( pos == arc.end() && gold_arcs_.at ( tk ).empty() ) ;
        }

        transition next_transition (  ) const {
            if ( should_leftarc() ) {
                return transition::LEFT_ARC ;
            } else if ( should_rightarc() ) {
                return transition::RIGTH_ARC ;
            } else {
                return transition::SHIFT ;
            }
        }

    private:
        stc_parser const &                                                          p_          ;
        std::map < units::token const *, std::set < units::token const * > >        gold_arcs_  ;
    } ;

    class stc_oracle {

    public:

        explicit stc_oracle ( units::sentence const & stc ) :
                p_ { stc } {
            gold_arcs_ [ &stc.root() ] = { } ;
            for ( auto const & tok : stc.tokens ( ) ) {
                gold_arcs_ [ &tok ] = { } ;
            }

            for ( auto const & tok : stc.tokens() ) {
                int const & head = tok.head_ ;
                if ( head == 0 /* that is, root */ ) {
                    gold_arcs_ [ &stc.root ( ) ].insert( &tok ) ;
                } else {
                    gold_arcs_ [ &stc.tokens ( ) [ head - 1 ] ].insert( &tok );
                }
            }
        }
        stc_oracle ( units::sentence const & stc, std::vector < std::array < float, 3 > > & fill ) :
                stc_oracle {stc} {
            trans_= std::move (fill) ;
        }

        inline bool terminal_state ( ) const {
            return p_.curr_state().buf.empty() ;
        }

        inline bool should_leftarc ( ) const {
            if ( p_.can_leftarc() ) {
                auto const & state = p_.curr_state();
                auto const & arcs = gold_arcs_.at(state.buf.back());
                return std::find(arcs.begin(), arcs.end(), state.stk.top()) != arcs.end();
            }
            return false ;

        }

        inline bool should_rightarc ( ) const {
            if ( p_.can_rightarc ( ) ) {
                auto const & state = p_.curr_state() ;
                auto const & arcs = gold_arcs_.at( state.stk.top() ) ;

                return std::find ( arcs.begin(),arcs.end(), state.buf.back() ) != arcs.end() &&
                       has_all_children( state.buf.back() ) ;
            }
            return false ;
        }

        inline bool has_all_children ( units::token const * tk ) const {
            auto const & arc = p_.curr_state().arcs ;
            auto pos = arc.find ( tk ) ;
            return ( pos != arc.end() && gold_arcs_.at( tk ) == arc.at( tk ) ) ||
                   ( pos == arc.end() && gold_arcs_.at ( tk ).empty() ) ;
        }

        std::vector < std::array < float, 3 > > transitions (  ) {

            while ( !terminal_state() ) {
                if ( should_leftarc() ) {
                    p_.leftarc() ;
                    trans_.push_back( { 0, 1, 0 } ) ;
                } else if ( should_rightarc() ) {
                    p_.rightarc() ;
                    trans_.push_back( { 0, 0, 1 } ) ;
                } else {
                    p_.shift() ;
                    trans_.push_back( { 1, 0, 0 } ) ;
                }
            }
            return std::move( trans_ ) ;
        }

    private:
        std::vector < std::array < float , 3 > >                                    trans_      ;
        stc_parser                                                                  p_          ;
        std::map < units::token const *, std::set < units::token const * > >        gold_arcs_  ;

    };

    class oracle
    {
    public:


        explicit oracle ( set::set const & s ) : s_ { s } { }

        ~oracle () = default ;
        oracle (const oracle & other) = delete ;
        oracle (oracle && other) = delete ;
        oracle & operator=(const oracle & other) = delete ;
        oracle & operator=(oracle && other) = delete ;

        std::vector < std::array < float , 3 > > transitions (  ) {
            if ( s_.sentences().empty() ) { throw std::runtime_error("FAILURE. Empty set of sentences.") ; }
            std::vector < std::array < float , 3 > > tr ;
            for ( auto const & stc : s_.sentences ( ) ){
                stc_oracle o  { stc, tr } ;
                tr = o.transitions ( ) ;
            }
            return tr ;
        }

    private:
        set::set const & s_ ;
    };


    class oracle_model : public model::model {

    public:

        explicit oracle_model (  parsers::arcstandard::smart_oracle const & m ) : or_ { m } { }
        std::array<float, 3> eval ( ) override {
            auto const tr = or_.next_transition ( ) ;
            if ( tr == transition::SHIFT ) {
                return { 1, 0, 0 } ;
            } else if ( tr == transition::LEFT_ARC ) {
                return { 0, 1, 0 } ;
            } else {
                return { 0, 0, 1 } ;
            }
        }
    private:
        parsers::arcstandard::smart_oracle const & or_ ;
    } ;



}

#endif //C___ORACLE_H
