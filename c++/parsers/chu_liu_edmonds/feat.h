#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
//
// Created by pedro on 4/12/19.
//

#ifndef C___FEAT_H
#define C___FEAT_H


#include <vector>
#include <algorithm>
#include "../../units.h"
#include "../../set.h"
#include <omp.h>

namespace parsers::chu_liu_edmonds::features {

    static std::string const dir_right = "r" ;
    static std::string const dir_left = "l" ;
    int const template_size = 23 ;


    /* optimizations for the near future:
     * Average the number of elements of the sparse vector.
     * If it's big enough, try to insert sorted by using std::upper_bound,
     * so to get a value std::binary_search can be used.
     * Also could be intersting to use std::sort.
     */
    template < typename I >
    class bool_sparse {
    public:
        bool_sparse () = default ;

        ~bool_sparse ( ) = default ;
        bool_sparse ( const bool_sparse & other ) = delete ;
        bool_sparse ( bool_sparse && other) = default ;
        bool_sparse & operator=( const bool_sparse & other) = delete ;
        bool_sparse & operator=( bool_sparse && other) = default ;

        inline void add ( I const & pos ) {

                pos_.push_back ( pos ) ;

        }

        [[nodiscard]] inline bool get ( I const & pos ) const {
            return std::find ( pos_.begin (), pos_.end (), pos ) != pos_.end () ;
        }

        std::vector < I > pos_ ;
    } ;



    class feat {
    public:

        feat ( )  { }

        ~feat () = default;
        feat (const feat & other) = delete ;
        feat (feat && other) = default ;
        feat & operator=(const feat & other) = delete ;
        feat & operator=(feat && other) = default ;

        bool_sparse < size_t >      p_      ;

    } ;

    class gold_feat : public feat
    {
    public:
        gold_feat ( bool const & v ) :
                feat { } , correct_arc { v } { }

        ~gold_feat () = default;
        gold_feat ( const gold_feat & other) = delete;
        gold_feat ( gold_feat && other) = default;
        gold_feat & operator=( const gold_feat & other) = delete ;
        gold_feat & operator=( gold_feat && other) = default;


        bool correct_arc ;
    };





    class tmpl {

        using ht = std::unordered_map < std::string, size_t > ;

        enum prp : int {
            hform,
            hpos,
            hform_hpos,
            dform,
            dpos,
            dform_dpos,

            hpos_dpos, //
            hform_dform, //
            hform_hpos_dpos,
            hform_hpos_dform, //
            hform_dform_dpos, //
            hpos_dform_dpos, //
            hform_hpos_dform_dpos, //

            hpos_bpos_dpos,
            hpos_dpos_hposp1_dposm1,
            hpos_dpos_hposm1_dposm1,
            hpos_dpos_hposp1_dposp1,
            hpos_dpos_hposm1_dposp1,

            hform_dform_hpos,

            hpos_hposp1_dpos,
            hpos_dposm1_dpos,
            hpos_dpos_dposp1,

            hposm1_hposp1_dposm1_dpos,



            hpos_dpos_hposp2_dposm2,
            hpos_dpos_hposm2_dposm2,
            hpos_dpos_hposp2_dposp2,
            hpos_dpos_hposm2_dposp2,
        } ;

    public:
        explicit tmpl ( ) : i_ { 0 } {
        } ;

        ~tmpl () = default ;
        tmpl (const tmpl & other) = delete ;
        tmpl (tmpl && other) = default ;
        tmpl & operator=(const tmpl & other) = delete ;
        tmpl & operator=(tmpl && other) = default ;

        template < typename T >
        inline void extract_features (  units::sentence const & s,
                                        int const & hi,
                                        int const & di,
                                        std::string const & dir,
                                        int const & dist,
                                        feat & fh,
                                        void (tmpl::*extraction_function)(ht & h, T const & key, feat & f) ) {

            using std::string_literals::operator""s;
            size_t const ssize = s.size ( ) ;
            std::string const dir_dist_string = "$"s + dir +  ( ( dist > 3 ) ? "3+"s : std::to_string ( dist ) ) ;


            units::token const & h = [ & ] ( ) -> units::token const & {
                if ( hi != -1  ) {
                    return s.tokens_[ hi ] ;
                } else {
                    return s.root ( ) ;
                }
            } ( ) ;
            units::token const & d = s.tokens_ [ di ] ;

            std::string const & dposm1 = [ & ] ( ) {
                if ( di > 0 ) {
                    return s.tokens_ [ di - 1 ].pos_ ;
                } else {
                    return "NULL"s ;
                }
            } ( ) ;

            std::string const & dposp1 = [& ] ( ) {
                if ( di + 1 < ssize ) {
                    return s.tokens_ [ di + 1 ].pos_ ;
                } else {
                    return "NULL"s ;
                }
            } ( ) ;

            std::string const & hposp1 = [& ] ( ) {
                if ( hi + 1 < ssize && hi != -1 ) {
                    return s.tokens_ [ hi + 1 ].pos_ ;
                } else {
                    return "NULL"s ;
                }
            } ( ) ;

            std::string const & hposm1 = [& ] ( ) {
                if ( hi > 0 ) {
                    return s.tokens_ [ hi - 1 ].pos_ ;
                } else {
                    return "NULL"s ;
                }
            } ( ) ;


//            std::string const & dposm2 = [& ] ( ) {
//                if ( di > 1 ) {
//                    return s.tokens_ [ di - 2 ].pos_ ;
//                } else {
//                    return "NULL"s ;
//                }
//            } ( ) ;
//
//            std::string const & dposp2 = [& ] ( ) {
//                if ( di + 2 < ssize ) {
//                    return s.tokens_ [ di + 2 ].pos_ ;
//                } else {
//                    return "NULL"s ;
//                }
//            } ( ) ;
//
//            std::string const & hposp2 = [& ] ( ) {
//                if ( hi + 2 < ssize && hi != -1 ) {
//                    return s.tokens_ [ hi + 2 ].pos_ ;
//                } else {
//                    return "NULL"s ;
//                }
//            } ( ) ;
//
//            std::string const & hposm2 = [& ] ( ) {
//                if ( hi > 1 ) {
//                    return s.tokens_ [ hi - 2 ].pos_ ;
//                } else {
//                    return "NULL"s ;
//                }
//            } ( ) ;


            #define JOIN "$"
            /* UNIGRAM FEATURES */
            (this->*extraction_function) ( h_[ hform ] , h.special_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hpos ] , h.pos_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hform_hpos ] , h.special_+h.pos_+dir_dist_string  , fh );
            (this->*extraction_function)  ( h_[ dform ] , d.special_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ dpos ] , d.pos_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ dform_dpos ] ,  d.special_+d.pos_+dir_dist_string  , fh );
            /* BIGRAM FEATURES */
            (this->*extraction_function)  ( h_[ hpos_dpos ] , h.pos_+d.pos_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hform_dform ] , h.special_+d.special_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hform_hpos_dpos ] , h.special_+h.pos_+d.pos_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hform_hpos_dform ] , h.special_+h.pos_+d.special_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hform_dform_dpos ] , h.special_+d.special_+d.pos_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hpos_dform_dpos ] , h.pos_+d.special_+d.pos_+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hform_hpos_dform_dpos ] , h.special_+h.pos_+d.lemma_+d.pos_+dir_dist_string , fh );


//            (this->*extraction_function)  ( h_[ hform_dform_hpos ] , h.special_+d.special_+h.pos_+dir_dist_string , fh );
            /* OTHER FEATURES */
//            add_feature ( h_[ hpos_bpos_dpos ] , h.pos_+bpos+d.pos_ , fh );
            (this->*extraction_function)  ( h_[ hpos_dpos_hposp1_dposm1 ] , h.pos_+d.pos_+hposp1+dposm1+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hpos_dpos_hposm1_dposm1 ] , h.pos_+d.pos_+hposm1+dposm1+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hpos_dpos_hposp1_dposp1 ] , h.pos_+d.pos_+hposp1+dposp1+dir_dist_string , fh );
            (this->*extraction_function)  ( h_[ hpos_dpos_hposm1_dposp1 ] , h.pos_+d.pos_+hposm1+dposp1+dir_dist_string  , fh );

            (this->*extraction_function)  ( h_[ hpos_hposp1_dpos ] , h.pos_+hposp1+d.pos_+dir_dist_string  , fh );
            (this->*extraction_function)  ( h_[ hpos_dposm1_dpos ] , h.pos_+dposm1+d.pos_+dir_dist_string  , fh );
            (this->*extraction_function)  ( h_[ hpos_dpos_dposp1 ] , h.pos_+d.pos_+dposp1+dir_dist_string  , fh );


            (this->*extraction_function)  ( h_[ hposm1_hposp1_dposm1_dpos ] , hposm1+hposp1+dposm1+d.pos_+dir_dist_string  , fh );

//            (this->*extraction_function)  ( h_[ hpos_dpos_hposp2_dposm2 ] , h.pos_+d.pos_+hposp2+dposm2+dir_dist_string , fh );
//            (this->*extraction_function)  ( h_[ hpos_dpos_hposm2_dposm2 ] , h.pos_+d.pos_+hposm2+dposm2+dir_dist_string , fh );
//            (this->*extraction_function)  ( h_[ hpos_dpos_hposp2_dposp2 ] , h.pos_+d.pos_+hposp2+dposp2+dir_dist_string , fh );
//            (this->*extraction_function)  ( h_[ hpos_dpos_hposm2_dposp2 ] , h.pos_+d.pos_+hposm2+dposp2+dir_dist_string  , fh );

            if ( std::abs ( hi - di ) == 1 ) {
                (this->*extraction_function)  ( h_[ hpos_bpos_dpos ] , h.pos_+"NULL"+d.pos_ , fh );
            } else {
                auto [ min, max ] = std::minmax ( hi , di ) ;
                for ( int i = min + 1 ; i < max ; ++i ) {
                    /* TODO: Check if tokens can be repeated */
//                    std::string innerpos = ( i < 4 )? std::to_string ( i )  : std::string( "$4+" ) ;
//                    (this->*extraction_function)  ( h_[ hpos_bpos_dpos ] , h.pos_+innerpos
//                    +d.pos_+dir_dist_string , fh );

                    (this->*extraction_function)  ( h_[ hpos_bpos_dpos ] , h.pos_+s.tokens_ [ i ].pos_
                    +d.pos_+dir_dist_string , fh );
                }
            }


        }

        template < typename T >
        inline void add_feature ( ht & h, T const & key, feat & f ) {
//            #pragma omp critical
            {
                auto it = h.find ( key );
                if ( it != h.end ( ) ) {
                    f.p_.add ( it->second );
                } else {
                    h[ key ] = i_;
                    f.p_.add ( i_ );
                    i_++;
                }
            }
        }

        template < typename T >
        inline void get_feature ( ht & h, T const & key, feat & f ) {
            auto it = h.find ( key );
            if ( it != h.end ( ) ) {
                f.p_.add ( it->second );
            }
        }

        inline std::string join (std::string const & s1, std::string const & s2 ) {
            return s1 + "$" + s2 ;
        }


        std::array < ht , template_size >       h_          ;
        size_t                                  i_          ;
    } ;









}

#endif //C___FEAT_H

#pragma clang diagnostic pop