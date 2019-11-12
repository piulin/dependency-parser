//
// Created by pedro on 7/11/19.
//

#ifndef C___ARCSTANDARD_H
#define C___ARCSTANDARD_H

#include "../../units.h"
#include <stack>
#include <map>
#include <algorithm>
#include <set>
#include "../../model.h"

namespace parsers::arcstandard {


    struct state_t {
        std::vector < units::token const * >                                        buf     ;
        std::stack < units::token const * >                                         stk     ;
        std::map < units::token const *, std::set < units::token const * > >        arcs    ;
    } ;


    std::ostream & operator << (std::ostream & o, std::set < units::token const * > const & v ) {
        o << "[" ;
        for ( auto const & e : v ) {
            o << e->form_ << ", ";
        }
        o << "]" ;
        return o ;
    }

    std::ostream & operator << ( std::ostream & o, state_t const & st ) {
        if ( !st.stk.empty() ) {
            o << "Stack -> [ ... , " << st.stk.top()->form_ << "] " ;

        } else {
            o << "Stack -> [ ] " ;
        }
        if ( !st.buf.empty() ) {
            o << " ["  << st.buf.back()->form_ << " , ... ] <- Buff\n" ;
        } else {
            o << " [ ] <- Buff\n" ;
        }
//        o << "Heads: \n" ;
//        for ( auto const & [ key, val ] : st.heads ) {
//            o << "  " << key->form_ << ": " << val->form_ << '\n' ;
//        }
        o << "Arcs: \n" ;
        for ( auto const & [ key, val ] : st.arcs ) {
            o << "  " << key->form_ << ": " << val << '\n' ;
        }
        o << '\n' ;
        return o ;
    }





    class stc_parser {

    public:

        explicit stc_parser (units::sentence const & stc ) :
                        stc_ { stc } ,
                        root_ { &stc.root() },
                        parsed_ { stc.size() } {

            if ( !stc.size() ) { throw std::runtime_error ( "FAILURE. Cannot parse an empty sentence." ) ; }

            std::vector < units::token const * >    buf ;
            std::stack < units::token const * >     stk ;
            stk.push( &stc.root() ) ;
            std::for_each( stc.tokens().rbegin(), stc.tokens().rend(), [ &buf ] ( auto const & tok ) {
                buf.push_back(&tok) ;
            } ) ;
            states_.push_back( { std::move(buf), std::move(stk),{} } ) ;

        }

        inline state_t curr_state ( ) const {
            return states_.back() ;
        }

        inline bool terminal_state ( ) const {
            return curr_state().buf.empty() ;

        }

        inline bool can_leftarc ( ) const {
            auto const & st = curr_state() ;
            return !st.stk.empty() && st.stk.top() != root_ ;
        }

        inline bool can_rightarc ( ) const {
            auto const & st = curr_state() ;
            return !st.stk.empty() && !st.buf.empty() ;
        }

        inline bool can_shift ( ) const {
            auto const & st = curr_state() ;
            return st.buf.size() > 1 || st.stk.empty()  ;
        }

        void leftarc ( ) {
//            std::cout << "Left arc ... \n" ;
            auto const & st = curr_state() ;
            state_t ns = st ;
            ns.arcs [ st.buf.back() ].insert( st.stk.top() ) ;
//            ns.heads [ st.stk.top() ] = st.buf.back() ;
            ns.stk.pop() ;
            states_.push_back( std::move( ns ) ) ;

            units::token ntok = *st.stk.top() ;
            ntok.head_ = st.buf.back()->id_ ;
            parsed_ [ ntok.id_ - 1 ] = std::move ( ntok ) ;
        }

        void rightarc ( ) {
//            std::cout << "Right arc ... \n" ;
            auto const & st = curr_state() ;
            state_t ns = st ;
            ns.arcs [ st.stk.top() ].insert( st.buf.back() ) ;
//            ns.heads [ st.buf.back() ] = st.stk.top() ;
            ns.stk.pop() ;
            ns.buf.pop_back() ;
            ns.buf.push_back( st.stk.top() ) ;
            states_.push_back( std::move( ns ) ) ;

            units::token ntok = *st.buf.back() ;
            ntok.head_ = st.stk.top()->id_ ;
            parsed_ [ ntok.id_ - 1 ] = std::move ( ntok ) ;
        }

        void shift ( ) {
            auto const & st = curr_state() ;
            state_t ns = st ;
            ns.stk.push( ns.buf.back() ) ;
            ns.buf.pop_back() ;
            states_.push_back( std::move( ns ) ) ;
        }

        units::sentence parse ( model::model & m ) {
            while ( !terminal_state() ) {
                auto scores = m.eval() ;
                perform_best(scores) ;
            }
            return std::move ( parsed_ ) ;
        }

        void perform_best ( std::array < float, 3 > & scores ) {
            for (short i = 0 ; i < 3 ; ++i ) {
                float * m = std::max_element(scores.begin(), scores.end());
                long max_pos = scores.end() - m ;
                switch (max_pos) {
                    case 3: {
                        if (can_shift()) {
                            shift();
                            return;
                        }
                        break ;
                    }
                    case 2: {
                        if (can_leftarc()) {
                            leftarc();
                            return;
                        }
                        break ;
                    }
                    case 1: {
                        if (can_rightarc()) {
                            rightarc();
                            return;
                        }
                        break ;
                    }
                }
                *m = -1 ;
            }

        }

    private:
        std::vector < state_t >     states_     ;
        units::sentence const &     stc_        ;
        units::token const *        root_       ;
        units::sentence             parsed_     ;

    } ;

}

#endif //C___ARCSTANDARD_H
