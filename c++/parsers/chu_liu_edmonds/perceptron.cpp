//
// Created by pedro on 5/12/19.
//

#include "perceptron.h"
#include <cmath>
#include "chu_liu_edmonds.h"

using namespace parsers::chu_liu_edmonds::model ;


perceptron::perceptron ( size_t const & chunk ) :
     chunk_ { chunk }, t_ { } {
    /* legacy code because of realloc*/
    w_.size_= chunk  ;
    w_.f_ = ( int * ) malloc ( chunk_*sizeof ( int ) ) ;
    std::fill ( w_.f_ , w_.f_ + w_.size_ , 0 ) ;

}

perceptron::~perceptron ( ) {
    free( w_.f_ ) ;
}

void perceptron::train ( set::set const & s ) {
    for ( int j = 0 ; j < 50 ; ++j ) {
        std::cout << "\nEphoc: " << j << '\n' ;
        int total_correct = 0 ;
        int total = 0 ;
        for ( int i = 0 ; i < s.sentences_.size ( ) ; ++i ) {

            if ( (i % 100) == 0 ) {
                std::cout << " *" << std::flush ;
            }

            units::sentence const & stc = s.sentences_[ i ];
            std::vector < int > gold_hds = to_heads ( stc );
            stc_parser prs { stc };
            std::vector < int > hds = prs.heads ( *this );

            int const correct = count_correct ( gold_hds , hds ) ;
            if ( correct != gold_hds.size () ) {
                update ( hds , stc , [ &gold_hds ] ( auto & e, auto const & i, auto const & h ) { if ( gold_hds [ i ] != h ) e--; } ) ;
                update ( gold_hds , stc , [&gold_hds ] ( auto & e, auto const & i, auto const & h) {  if ( gold_hds [ i ] != h ) e++; } ) ;
            }
            total_correct += correct ;
            total += gold_hds.size ( ) ;
        }
        std::cout << "\nUAS: " << std::fixed << std::setw (5) << static_cast< float > ( total_correct )*100.0f/static_cast< float >( total ) <<
                  " Correct: " << total_correct << " Out of: " << total << '\n' ;


    }


}

inline bool perceptron::equal ( std::vector < int > const & gold, std::vector < int > const & heads ) const {
    for ( int i = 0 ; i < gold.size () ; ++i ) {
        if ( gold [ i ] != heads [ i ] ) {
            return false ;
        }
    }
    return true ;
}
inline int perceptron::count_correct ( std::vector < int > const & gold, std::vector < int > const & heads ) const {
    int correct = 0 ;
    for ( int i = 0 ; i < gold.size () ; ++i ) {
        if ( gold [ i ] == heads [ i ] ) {
            correct ++ ;
        }
    }
    return correct ;
}



void perceptron::enlarge ( size_t const & s ) {
    size_t const diff = s + 1 - w_.size_  ;
    int no_chunks = std::ceil ( static_cast<double> (diff)/static_cast<double> (chunk_) ) ;
    int const new_size = w_.size_ + no_chunks*chunk_ ;
    int * r = ( int * ) std::realloc ( w_.f_ , new_size*sizeof ( int ) ) ;
    if ( r == nullptr ) {
        throw std::runtime_error ( "[Failure]: Realloc error. :(" ) ;
    }
    w_.f_ = r ;
    std::fill ( w_.f_ + w_.size_ , w_.f_ + new_size , 0 ) ;
    w_.size_ = new_size ;

}

void perceptron::eval ( units::sentence const & stc , parsers::chu_liu_edmonds::matrix < int > & m ) {
    int * pm = m.ptr ( );
    units::token const & root = stc.root ( ) ;
    for ( int j = 0 ; j < m.rows () ; ++j ) {
        units::token const & d = stc.tokens_ [ j ] ;
        /* TODO: optimize: create and delete too many times. */
        features::feat f ;
        t_.extract_features ( stc, -1 , j , parsers::chu_liu_edmonds::features::dir_right , j + 1 ,  f ) ;
        if ( t_.i_ >= w_.size_ ) {
            enlarge ( t_.i_ ) ;
        }

        pm[ j*m.cols ( ) ] = dot_product ( f ) ;
    }

    //std::cout << "COST : " << m ;

    for ( int i = 0 ; i < m.rows ( ) ; ++i ) {
        for ( int j = 1 ; j < m.cols ( ) ; ++j ) {
            if ( i + 1 == j ) {
                pm[ i*m.cols ( ) + j ] = std::numeric_limits< int >::min ( ) ;
            } else {
                auto & upd = pm[ i*m.cols ( ) + j ] ;
                units::token const & h = stc.tokens_ [ j - 1 ] ;
                units::token const & d = stc.tokens_ [ i ] ;

//                auto dir = features::feat::direction::right ;
//                if ( j > i + 1  ) {
//                    dir = features::feat::direction::left ;
//                }

                features::feat f ;
                t_.extract_features ( stc , j - 1, i ,
                        ( j > i + 1  ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                        std::abs (i+1-j),  f ) ;
                if ( t_.i_ >= w_.size_ ) {
                    enlarge ( t_.i_ ) ;
                }

                upd = dot_product ( f ) ;
                //std::cout << "COST : " << m ;
            }
        }
    }
            //std::cout << "COST : " << m ;

}

int perceptron::dot_product ( features::feat const & f ) {
    int product = 0 ;
    for ( auto const & e : f.p_.pos_ ) {
        product += w_.f_ [ e ] ;
    }
    //product += w_.dr_ * f.dr_ ; /* i'm not sure about this */
    //product += w_.ds_ * f.ds_ ;
    return product ;
}

std::vector < int > perceptron::to_heads ( units::sentence const & stc ) {
    std::vector < int > heads ( stc.size ( ) ) ;
    for ( int i = 0 ; i < stc.size ( ) ; ++i ) {
        int const & h = stc.tokens_ [ i ].head_ ;
        heads [ i ] = h ;
    }
    return heads ;
}


template < typename BOP >
void perceptron::update ( std::vector < int > const & heads, units::sentence const & stc, BOP && bop ) {
    for ( int i = 0 ; i < stc.size () ; ++i ) {
        units::token const & d = stc.tokens_[ i ] ;
        int const hindex = heads[ i ] ;
        units::token const & h = [ & ] ( ) -> units::token const & {
            if ( hindex ) {
                return stc.tokens_[ hindex - 1 ] ;
            } else {
                return stc.root ( ) ;
            }
        } ( ) ;

//        auto dir = features::feat::direction::right ;
//        if ( h.id_ > d.id_  ) {
//            dir = features::feat::direction::left ;
//        }

        features::feat f ;
        t_.extract_features ( stc , hindex - 1 , i ,
                ( h.id_ > d.id_ ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                std::abs (h.id_-d.id_), f ) ;

        for ( auto const & p : f.p_.pos_ ) {
            bop( w_.f_ [ p ], i, hindex ) ;
        }

//        std::cout << "w : < dr: " << std::setw (4) << w_.dr_ << ", ds: " << std::setw (4) << w_.ds_ << " > [" ;
//        for ( int i = 0 ; i < t_.i_ ; ++i ) {
//            std::cout << std::setw (2) << w_.f_ [ i ] << " " ;
//        }
//        std::cout << "]\n" ;


    }
}

void perceptron::dump ( std::string const & filename ) {

    std::ofstream f ( filename , std::ofstream::binary ) ;
    f << std::to_string ( w_.size_ ) << '\n' ;
    f << std::to_string ( chunk_ ) << '\n';
    f.write ( reinterpret_cast< char * > ( w_.f_ ) ,w_.size_*sizeof( int ) ) ;
    for ( auto const & ht : t_.h_ ) {
        for ( auto & [ key, val ] : ht ) {
            f << key << '\n' ;
            f.write ( reinterpret_cast < char const * > ( &val ) , sizeof( size_t ) ) ;
        }
    }
    
}


