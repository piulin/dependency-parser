//
// Created by pedro on 22/11/19.
//

#ifndef C___CHIU_MODEL_H
#define C___CHIU_MODEL_H


#include <string>
#include <fstream>
#include <iostream>
#include "feat.h"
#include "matrix.h"




namespace parsers::chu_liu_edmonds::model {
    template < typename T >
    class model {
    public:
        virtual void eval ( units::sentence const & stc, matrix < T > & m ) = 0 ;
    } ;


    template < typename T >
    class file_model : public model < T > {
    public:
        explicit file_model ( std::string const & filename ) : f_ { filename } {
            if (!f_.is_open()){
                throw std::runtime_error ( "Couldn't open file" + filename ) ;
            }
        }
        void eval ( units::sentence const & stc, matrix < T > & m ) override  {
            T fi ;
            T * pm = m.ptr ( );
            for ( int i = 0 ; i < m.rows ( ) ; ++i ) {
                for ( int j = 0 ; j < m.cols ( ) ; ++j ) {
                    if ( i + 1 == j ) {
                        pm[ i*m.cols ( ) + j ] = -1;
                    } else {
                        f_ >> fi ;
                        pm[ i*m.cols ( ) + j ] = fi ;
                    }
                }
            }
        }
    private:
        std::ifstream f_ ;
    };

    template < typename T >
    class rd_model : public model < T > {
    public:
        rd_model ( ) {
//            int s = 1575126670 ;
            int s = time(nullptr) ;
            std::cout << "seed: " << s <<'\n' ;
            srand(s);
        }
        void eval ( units::sentence const & stc, matrix < T > & m ) override  {
            T * pm = m.ptr ( );
            for ( int i = 0 ; i < m.rows ( ) ; ++i ) {
                for ( int j = 0 ; j < m.cols ( ) ; ++j ) {
                    if ( i + 1 == j ) {
                        pm[ i*m.cols ( ) + j ] = -1;
                    } else {
                        pm[ i*m.cols ( ) + j ] = static_cast< T >( ( static_cast<float>( rand() )/ static_cast<float>( RAND_MAX ) ) * max_ ) ;
                    }
                }
            }
        }

    private:
        T max_ = static_cast< T > ( 200 )  ;
    } ;





}

#endif //C___CHIU_MODEL_H
