//

#include "perceptron.h"
#include <cmath>
#include <set>
#include "chu_liu_edmonds.h"
#include "../../assessment.h"
#include "../set_parser.h"

using namespace parsers::chu_liu_edmonds::model ;


perceptron::perceptron ( size_t const & chunk, std::vector < std::string > rels ) :
    t_ { } ,
    rels_ { std::move ( rels ) } ,
    wl_ ( rels.size () ),
    chunk_ { chunk } {
    /* legacy code because of realloc*/
    w_.size_= chunk  ;
    w_.f_ = ( int * ) malloc ( chunk_*sizeof ( int ) ) ;
    std::fill ( w_.f_ , w_.f_ + w_.size_ , 0 ) ;
    u_.size_ = chunk ;
    u_.f_ = ( int * ) malloc ( chunk_*sizeof ( int ) ) ;
    std::fill ( u_.f_ , u_.f_ + u_.size_ , 0 ) ;

    for ( w & e : wl_ ) {
        e.size_= chunk  ;
        e.f_ = ( int * ) malloc ( chunk_*sizeof ( int ) ) ;
        std::fill ( e.f_ , e.f_ + e.size_ , 0 ) ;
    }
}

perceptron::perceptron ( std::string const & filename ) {
    std::ifstream f ( filename , std::ofstream::binary ) ;
    u_.f_ = nullptr ;
    f >> w_.size_ ;
    f >> chunk_ ;
    int rel_size ;
    f >> rel_size ;
    rels_.resize ( rel_size ) ;
    wl_.resize ( rel_size ) ;
    for ( int j = 0 ; j < rel_size ; ++j ) {
        f >> rels_[ j ] ;
    }
    f.ignore ( 1 ) ;
    w_.f_ = ( int * ) malloc ( w_.size_*sizeof ( int ) ) ;

    if ( w_.f_ == nullptr ) {
        throw std::runtime_error ( "Not enough memory to allocate weight vector of the perceptron :(" ) ;
    }


    f.read ( reinterpret_cast< char * > ( w_.f_ ) , w_.size_*sizeof ( int ) ) ;

    for ( int k = 0 ; k < rel_size ; ++k ) {
        int size = 0 ;
        f >> size ;
//        f.ignore ( 1 ) ;
        wl_ [ k ].size_ = size ;
        wl_ [ k ].f_ = ( int * ) malloc ( size*sizeof ( int ) ) ;
        if ( wl_ [ k ].f_ == nullptr ) {
            throw std::runtime_error ( "Not enough memory to allocate weight vector of the perceptron :(" ) ;
        }
        f.read ( reinterpret_cast< char * > ( wl_ [ k ].f_ ) , size*sizeof ( int ) ) ;
    }

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
    if ( u_.f_ ) {
        free ( u_.f_ ) ;
    }

    for ( w & e : wl_ ) {
        free ( e.f_ ) ;
    }

}

void perceptron::train ( set::set const & s, size_t const & epochs, std::optional < set::set > dev_set ) {
    size_t q = 0 ;
    for ( int j = 0 ; j < epochs ; ++j ) {
        std::cout << "\nEpoch: " << j << '\n' ;
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
                update ( hds , stc , [ ] ( auto & e ) { e-- ; } , [ &q ] ( auto & e ) { e -= q ; } ) ;
                update ( gold_hds , stc , [ ] ( auto & e ) { e++; }, [ &q ] ( auto & e ) { e += q ; } ) ;
            }
            total += gold_hds.size ( ) ;
            total_correct += correct ;

            arcs ( stc , gold_hds ) ;

        }
        std::cout << "\nUAS (iterative): " << std::fixed << std::setw (5) << static_cast< float > ( total_correct )*100.0f/static_cast< float >( total ) <<
                  " Correct: " << total_correct << " Out of: " << total << '\n' ;

        if ( dev_set != std::nullopt ) {
            test_dev_set ( dev_set.value () , q ) ;
        }
    }

    std::cout << "Training done. Averaging...\n" ;
    average ( q ) ;


}

inline void perceptron::arcs ( units::sentence const & stc, std::vector < int > const & gold_heads ) {
    std::vector < int > pred_labels ( gold_heads.size ( ) ) ;
    std::vector < features::feat > fs ( gold_heads.size ( ) ) ;
    for ( int i = 0 ; i < gold_heads.size () ; ++i ) {
        int const & head = gold_heads[ i ] - 1 ;
        t_.extract_features < std::string > ( stc, head , i,
                ( i > head + 1 ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                std::abs (head + 1 - i ) ,  fs [ i ],
                &features::tmpl::add_feature ) ;

        for ( w & e : wl_ ) {
            enlarge ( e, t_.i_ ) ;
        }

        pred_labels[ i ] = eval_label ( fs[ i ] ) ;
    }
    if ( !equal ( gold_heads , pred_labels ) ) {
        for ( int i = 0 ; i < pred_labels.size () ; ++i ) {
            if ( pred_labels[ i ] != stc.tokens_ [ i ].irel_ ) {
                w & wrong = wl_[ pred_labels[ i ] ] ;
                w & good = wl_ [ stc.tokens_ [ i ].irel_ ] ;
                features::feat & f = fs [ i ] ;
                for ( auto const & e : f.p_.pos_ ) {
                    wrong.f_ [ e ] -- ;
                    good.f_ [ e ] ++ ;
                }
            }
        }
    }
}

void perceptron::predict_labels ( units::sentence & stc ) {
    std::vector < int > stc_heads = to_heads ( stc ) ;
    for ( int i = 0 ; i < stc_heads.size () ; ++i ) {
        int const & head = stc_heads[ i ] - 1 ;
        features::feat f ;
        t_.extract_features < std::string > ( stc, head , i,
                                              ( i > head + 1 ) ? parsers::chu_liu_edmonds::features::dir_left : parsers::chu_liu_edmonds::features::dir_right ,
                                              std::abs (head + 1 - i ) ,  f ,
                                              &features::tmpl::get_feature ) ;
        int const ilabel = eval_label ( f ) ;
        stc.tokens_[ i ].rel_ = rels_[ ilabel ] ;
        stc.tokens_[ i ].irel_ = ilabel ;
    }
}

int perceptron::eval_label ( features::feat const & f ) {
    int maxv = std::numeric_limits<int>::min () ;
    int maxpc = -1 ;

//    #pragma omp parallel for reduction (max:maxv)
    for ( int j = 0 ; j < wl_.size () ; ++j ) {
        w const & cw = wl_[ j ] ;
        int prod = dot_product ( f , cw ) ;
        if ( prod > maxv ) {
            maxv = prod ;
            maxpc = j ;
        }
    }
    return maxpc ;
}


inline bool perceptron::equal ( std::vector < int > const & gold, std::vector < int > const & heads ) const {
    for ( int i = 0 ; i < gold.size () ; ++i ) {
        if ( gold [ i ] != heads [ i ] ) {
            return false ;
        }
    }
    return true ;
}

void perceptron::test_dev_set ( set::set const & ds, size_t const & q ) {
    auto waverage = cpy_average ( q ) ;
    parsers::set_parser grapph_parser { ds } ;
    int * averageptr = waverage.get ( ) ;
    std::swap ( averageptr , w_.f_ ) ;
    auto gparsed = grapph_parser.parse (
            [ this ] ( units::sentence const & s , parsers::chu_liu_edmonds::stc_parser & e ) {
                return e.parse ( *this );
            } );
    assessment::assess_t as =  assessment::uas ( ds , gparsed );
    std::cout << "UAS (dev_set):   " << std::fixed << std::setw (5) << as.precision <<
    " Correct: " << as.correctly_predicted << " Out of: " << as.total << '\n' ;
    std::swap ( averageptr , w_.f_ ) ;
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

void perceptron::enlarge ( w & target, size_t const & idx ) {
    if ( idx >= target.size_ ) {

        size_t const diff = idx + 1 - target.size_;
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
#pragma omp parallel for default (shared)
    for ( int j = 0 ; j < m.rows () ; ++j ) {
        /* TODO: optimize: create and delete too many times. */
        features::feat f ;
        t_.extract_features < std::string > ( stc, -1 , j , parsers::chu_liu_edmonds::features::dir_right , j + 1 ,  f,
                                              &features::tmpl::get_feature ) ;

        pm[ j*m.cols ( ) ] = dot_product ( f, w_ ) ;
    }
#pragma omp parallel for default (shared)
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
                upd = dot_product ( f, w_ ) ;
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

        enlarge ( w_, t_.i_ ) ;
        enlarge ( u_, t_.i_ ) ;

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
    f << std::to_string ( rels_.size () ) << '\n';
    for ( auto const & label : rels_ ) {
        f << label << '\n' ;
    }
    f.write ( reinterpret_cast< char * > ( w_.f_ ) , w_.size_*sizeof ( int ) );
    for ( w const & e : wl_ ) {
        f << e.size_ ;
        f.write ( reinterpret_cast< char * > ( e.f_ ) , e.size_*sizeof ( int ) );
    }
    for ( auto const & ht : t_.h_ ) {
        f << ht.size () << '\n';
        for ( auto &[key , val] : ht ) {
            f << key << '\n' ;
            f.write ( reinterpret_cast < char const * > ( &val ) , sizeof ( size_t ) );
        }
    }

}


int perceptron::dot_product ( features::feat const & f, w const & fvec ) {
    int product = 0 ;
    for ( auto const & e : f.p_.pos_ ) {
        product += fvec.f_ [ e ] ;
    }
    return product ;
}

void perceptron::average ( size_t const & q ) {

    #pragma omp parallel for default ( shared )
    for ( size_t i = 0 ; i < w_.size_ ; ++i ) {
        w_.f_[ i ] -= static_cast< int > ( static_cast<float> (u_.f_[ i ]) /static_cast<float>(q) ) ;
    }
}

std::unique_ptr < int [] > perceptron::cpy_average ( size_t const & q ) {
    std::unique_ptr < int[] > p = std::make_unique < int[] > ( w_.size_ ) ;
    #pragma omp parallel for default ( shared )
    for ( size_t i = 0 ; i < w_.size_ ; ++i ) {
        p[ i ] = w_.f_[ i ] - static_cast< int > ( static_cast<float> (u_.f_[ i ])/static_cast<float>(q) ) ;
    }
    return p ;
}

