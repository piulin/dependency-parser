//

#include "perceptron.h"
#include <cmath>
#include "chu_liu_edmonds.h"

using namespace parsers::chu_liu_edmonds::model ;


perceptron::perceptron ( size_t const & chunk ) : t_ {} ,
     chunk_ { chunk } {
    /* legacy code because of realloc*/
    w_.size_= chunk  ;
    w_.f_ = ( int * ) malloc ( chunk_*sizeof ( int ) ) ;
    std::fill ( w_.f_ , w_.f_ + w_.size_ , 0 ) ;
    u_.size_ = chunk ;
    u_.f_ = ( int * ) malloc ( chunk_*sizeof ( int ) ) ;
    std::fill ( u_.f_ , u_.f_ + u_.size_ , 0 ) ;

}

perceptron::perceptron ( std::string const & filename ) {
    std::ifstream f ( filename , std::ofstream::binary );
    f >> w_.size_ ;
    f >> chunk_ ;
    f.ignore ( 1 ) ;
    w_.f_ = ( int * ) malloc ( w_.size_*sizeof ( int ) ) ;
    if ( w_.f_ == nullptr ) {
        throw std::runtime_error ( "Not enough memory to allocate weight vector of the  perceptron :(" ) ;
    }

    f.read ( reinterpret_cast< char * > ( w_.f_ ) , w_.size_*sizeof ( int ) ) ;

    for ( auto & ht : t_.h_ ) {
        size_t s = 0 ;
        f >> s ;
        for ( int i = 0 ; i < s ; ++i ) {
            std::string key ;
            size_t val ;
            f >> key ;
            f.ignore ( 1 ) ;
            f.read ( reinterpret_cast < char * > ( &val ) , sizeof ( size_t ) ) ;
            ht[ std::move ( key ) ] =  val ;
        }
    }
}

perceptron::~perceptron ( ) {
    free ( w_.f_ ) ;
    free ( u_.f_ ) ;
}

void perceptron::train ( set::set const & s, size_t const & ephocs ) {
    size_t q = 0 ;
    for ( int j = 0 ; j < ephocs ; ++j ) {
        std::cout << "\nEphoc: " << j << '\n' ;
        int total_correct = 0 ;
        int total = 0 ;

        for ( int i = 0 ; i < s.sentences_.size ( ) ; ++i ) {
            q ++ ;
            if ( (i % 100) == 0 ) {
                std::cout << " *" << std::flush ;
            }

            units::sentence const & stc = s.sentences_[ i ];
            std::vector < int > gold_hds = to_heads ( stc );
            stc_parser prs { stc };
            std::vector < int > hds = prs.heads ( *this );

            int const correct = count_correct ( gold_hds , hds ) ;
            if ( correct != gold_hds.size () ) {
//                update ( hds , stc , [ &gold_hds ] ( auto & e, auto const & i, auto const & h ) { if ( gold_hds [ i ] != h ) e--; } ) ;
                update ( hds , stc , [ ] ( auto & e ) {
                    e--;
                    } , [ &q ] ( auto & e ) {
                    e -= q ;
                } ) ;
                update ( gold_hds , stc , [ ] ( auto & e ) {
                    e++;
                    }, [ &q ] ( auto & e ) {
                    e += q ;
                } ) ;
            }
            total_correct += correct ;
            total += gold_hds.size ( ) ;
        }
        std::cout << "\nUAS: " << std::fixed << std::setw (5) << static_cast< float > ( total_correct )*100.0f/static_cast< float >( total ) <<
                  " Correct: " << total_correct << " Out of: " << total << '\n' ;


    }

    std::cout << "Training done. Averaging...\n" ;
    average ( q ) ;


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

void perceptron::enlarge ( w & target ) {
    if ( t_.i_ >= target.size_ ) {

        size_t const diff = t_.i_ + 1 - target.size_;
        int no_chunks = std::ceil ( static_cast<double> (diff)/static_cast<double> (chunk_) );
        int const new_size = target.size_ + no_chunks*chunk_;
        int * r = ( int * ) std::realloc ( target.f_ , new_size*sizeof ( int ) );
        if ( r == nullptr ) {
            throw std::runtime_error ( "[Failure]: Realloc error. :(" );
        }
        target.f_ = r;
        std::fill ( target.f_ + target.size_ , target.f_ + new_size , 0 );
        target.size_ = new_size;
    }

}

void perceptron::eval ( units::sentence const & stc , parsers::chu_liu_edmonds::matrix < int > & m ) {
    int * pm = m.ptr ( );
    for ( int j = 0 ; j < m.rows () ; ++j ) {
        /* TODO: optimize: create and delete too many times. */
        features::feat f ;
        t_.extract_features < std::string > ( stc, -1 , j , parsers::chu_liu_edmonds::features::dir_right , j + 1 ,  f,
                &features::tmpl::add_feature ) ;
        enlarge ( w_ ) ;
        enlarge ( u_ ) ;

        pm[ j*m.cols ( ) ] = dot_product ( f ) ;
    }

    for ( int i = 0 ; i < m.rows ( ) ; ++i ) {
        for ( int j = 1 ; j < m.cols ( ) ; ++j ) {
            if ( i + 1 == j ) {
                pm[ i*m.cols ( ) + j ] = std::numeric_limits< int >::min ( ) ;
            } else {
                auto & upd = pm[ i*m.cols ( ) + j ] ;

                features::feat f ;
                t_.extract_features < std::string >  ( stc , j - 1, i ,
                        ( j > i + 1  ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                        std::abs (i+1-j),  f , &features::tmpl::add_feature ) ;
                enlarge ( w_ ) ;
                enlarge ( u_ ) ;

                upd = dot_product ( f ) ;
            }
        }
    }

}


std::vector < int > perceptron::to_heads ( units::sentence const & stc ) {
    std::vector < int > heads ( stc.size ( ) ) ;
    for ( int i = 0 ; i < stc.size ( ) ; ++i ) {
        int const & h = stc.tokens_ [ i ].head_ ;
        heads [ i ] = h ;
    }
    return heads ;
}

template < typename BOPW, typename BOPU >
void perceptron::update ( std::vector < int > const & heads, units::sentence const & stc, BOPW && bopw, BOPU && bopu ) {
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

        features::feat f ;
        t_.extract_features < std::string > ( stc , hindex - 1 , i ,
                ( h.id_ > d.id_ ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                std::abs (h.id_-d.id_), f, &features::tmpl::add_feature ) ;

        for ( auto const & p : f.p_.pos_ ) {
            bopw( w_.f_ [ p ] ) ;
            bopu( u_.f_ [ p ] ) ;
        }

    }
}

void perceptron::dump ( std::string const & filename ) {

    std::ofstream f ( filename , std::ofstream::binary );
    f << std::to_string ( w_.size_ ) << '\n';
    f << std::to_string ( chunk_ ) << '\n';
    f.write ( reinterpret_cast< char * > ( w_.f_ ) , w_.size_*sizeof ( int ) );
    for ( auto const & ht : t_.h_ ) {
        f << ht.size () << '\n';
        for ( auto &[key , val] : ht ) {
            f << key << '\n' ;
            f.write ( reinterpret_cast < char const * > ( &val ) , sizeof ( size_t ) );
        }
    }

}


int perceptron::dot_product ( features::feat const & f ) {
    int product = 0 ;
    for ( auto const & e : f.p_.pos_ ) {
        product += w_.f_ [ e ] ;
    }
    return product ;
}

void perceptron::average ( size_t const & q ) {

    #pragma omp parallel for default ( shared )
    for ( size_t i = 0 ; i < w_.size_ ; ++i ) {
        w_.f_[ i ] -= static_cast< int > ( static_cast<float> (u_.f_[ i ]) /static_cast<float>(q) ) ;
    }
}


void scryer::eval ( units::sentence const & stc , parsers::chu_liu_edmonds::matrix < int > & m ) {
    int * pm = m.ptr ( );
    for ( int j = 0 ; j < m.rows () ; ++j ) {
        /* TODO: optimize: create and delete too many times. */
        features::feat f ;
        t_.extract_features < std::string > ( stc, -1 , j , parsers::chu_liu_edmonds::features::dir_right , j + 1 ,  f,
                                              &features::tmpl::get_feature ) ;
        pm[ j*m.cols ( ) ] = p_.dot_product ( f ) ;
    }
    for ( int i = 0 ; i < m.rows ( ) ; ++i ) {
        for ( int j = 1 ; j < m.cols ( ) ; ++j ) {
            if ( i + 1 == j ) {
                pm[ i*m.cols ( ) + j ] = std::numeric_limits< int >::min ( ) ;
            } else {
                auto & upd = pm[ i*m.cols ( ) + j ] ;
                features::feat f ;
                t_.extract_features < std::string >  ( stc , j - 1, i ,
                                                       ( j > i + 1  ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                                                       std::abs (i+1-j),  f , &features::tmpl::get_feature ) ;
                upd = p_.dot_product ( f ) ;
            }
        }
    }
}


scryer::scryer ( perceptron & p ) : p_ { p }, w_ {p_.w_}, t_ {p_.t_ } {

}
