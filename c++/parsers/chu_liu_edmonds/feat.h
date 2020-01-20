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
    int const template_size = 18 ;


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

            hpos_dpos,
            hform_dform,
            hform_hpos_dpos,
            hform_hpos_dform,
            hform_dform_dpos,
            hpos_dform_dpos,
            hform_hpos_dform_dpos,

            hpos_bpos_dpos,
            hpos_dpos_hposp1_dposm1,
            hpos_dpos_hposm1_dposm1,
            hpos_dpos_hposp1_dposp1,
            hpos_dpos_hposm1_dposp1,
        } ;

    public:
        explicit tmpl ( ) : i_ { 0 } {
        } ;

        ~tmpl () = default ;
        tmpl (const tmpl & other) = delete ;
        tmpl (tmpl && other) = default ;
        tmpl & operator=(const tmpl & other) = delete ;
        tmpl & operator=(tmpl && other) = default ;

        void gold_features ( set::set const & s ) {

//            #pragma omp parallel for default (shared)
            for ( int i = 0 ; i < s.sentences_.size ( ) ; ++i ) {
                units::sentence const & stc = s.sentences_ [ i ] ;
//                auto[feat,i] = gold_features ( stc ) ;
            }
        }

//        auto gold_features ( units::sentence const & stc ) {
//
//            units::token const & root_head = stc.root ( ) ;
//            std::vector < gold_feat > fts ;
//
//            /* root */
//            for ( int j = 0 ; j < stc.size ( ) ; ++j ) {
//                units::token const & dependant = stc.tokens_ [ j ] ;
//
//                gold_feat & f1 = fts.emplace_back ( feat::direction::right, j, dependant.head_ == 0 ) ;
//                gold_feat & f2 = fts.emplace_back ( feat::direction::left, j, false ) ;
//
//                extract_features ( root_head, dependant , f1 ) ;
//                extract_features ( dependant, root_head , f2 ) ;
//
//            }
//
//            for ( int j = 0 ; j < stc.size ( ) ; ++j ) {
//                units::token const & h = stc.tokens_ [ j ] ;
//                for ( int k = j + 1 ; k < stc.size ( ) ; ++k ) {
//                    units::token const & d = stc.tokens_[ k ] ;
//
//                    gold_feat & f1 = fts.emplace_back ( feat::direction::right , k , d.head_ == h.id_ ) ;
//                    gold_feat & f2 = fts.emplace_back ( feat::direction::left , k , h.head_ == d.id_ ) ;
//
//                    extract_features ( h , d , f1 );
//                    extract_features ( d , h , f2 );
//
//                }
//            }
//
//            return std::make_tuple ( std::move( fts ) , i_ ) ;
//
//        }

        inline void extract_features (  units::sentence const & s,
                                        int const & hi,
                                        int const & di,
                                        std::string const & dir,
                                        int const & dist,
                                        feat & fh ) {

            using std::string_literals::operator""s;
            size_t const ssize = s.size ( ) ;
            std::string const dir_dist_string = dir + "$" + std::to_string ( dist ) ;

            units::token const & h = [ & ] ( ) -> units::token const & {
                if ( hi != -1  ) {
                    return s.tokens_[ hi ] ;
                } else {
                    return s.root ( ) ;
                }
            } ( ) ;
            units::token const & d = s.tokens_ [ di ] ;

            std::string const & dposm1 = [& ] ( ) {
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



            #define JOIN "$"
            /* UNIGRAM FEATURES */
            add_feature ( h_[ hform ] , h.form_+dir_dist_string , fh );
            add_feature ( h_[ hpos ] , h.pos_+dir_dist_string , fh );
            add_feature ( h_[ hform_hpos ] , h.form_+h.pos_+dir_dist_string  , fh );
            add_feature ( h_[ dform ] , d.form_+dir_dist_string , fh );
            add_feature ( h_[ dpos ] , d.pos_+dir_dist_string , fh );
            add_feature ( h_[ dform_dpos ] ,  d.form_+d.pos_+dir_dist_string  , fh );
            /* BIGRAM FEATURES */
            add_feature ( h_[ hpos_dpos ] , h.pos_+d.pos_+dir_dist_string , fh );
            add_feature ( h_[ hform_dform ] , h.form_+d.form_+dir_dist_string , fh );
            add_feature ( h_[ hform_hpos_dpos ] , h.form_+h.pos_+d.pos_+dir_dist_string , fh );
            add_feature ( h_[ hform_hpos_dform ] , h.form_+h.pos_+d.form_+dir_dist_string , fh );
            add_feature ( h_[ hform_dform_dpos ] , h.form_+h.form_+d.pos_+dir_dist_string , fh );
            add_feature ( h_[ hpos_dform_dpos ] , h.pos_+d.form_+d.pos_+dir_dist_string , fh );
            add_feature ( h_[ hform_hpos_dform_dpos ] , h.form_+h.pos_+d.form_+d.pos_+dir_dist_string , fh );
            /* OTHER FEATURES */
//            add_feature ( h_[ hpos_bpos_dpos ] , h.pos_+bpos+d.pos_ , fh );
            add_feature ( h_[ hpos_dpos_hposp1_dposm1 ] , h.pos_+d.pos_+hposp1+dposm1+dir_dist_string , fh );
            add_feature ( h_[ hpos_dpos_hposm1_dposm1 ] , h.pos_+d.pos_+hposm1+dposm1+dir_dist_string , fh );
            add_feature ( h_[ hpos_dpos_hposp1_dposp1 ] , h.pos_+d.pos_+hposp1+dposp1+dir_dist_string , fh );
            add_feature ( h_[ hpos_dpos_hposm1_dposp1 ] , h.pos_+d.pos_+hposm1+dposp1+dir_dist_string  , fh );

            if ( std::abs ( hi - di ) == 1 ) {
                add_feature ( h_[ hpos_bpos_dpos ] , h.pos_+"NULL"+d.pos_ , fh );
            } else {
                auto [ min, max ] = std::minmax ( hi , di ) ;
                for ( int i = min + 1 ; i < max ; ++i ) {
                    /* TODO: Check if tokens can be repeated */
                    add_feature ( h_[ hpos_bpos_dpos ] , h.pos_+s.tokens_ [ i ].pos_ +d.pos_+dir_dist_string , fh );
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

        inline std::string join (std::string const & s1, std::string const & s2 ) {
            return s1 + "$" + s2 ;
        }


        std::array < ht , template_size >       h_          ;
        size_t                                  i_          ;
    } ;









}

#endif //C___FEAT_H

#pragma clang diagnostic pop