//
// Created by pedro on 22/11/19.
//

#ifndef C___CHIU_MODEL_H
#define C___CHIU_MODEL_H


#include <string>
#include <fstream>
#include <iostream>

namespace parsers::chu_liu_edmonds::model {

    class model {
    public:
        virtual short eval ( ) = 0 ;
    } ;


    class file_model : public model {
    public:
        file_model ( std::string const & filename ) : f_ { filename } {
            if (!f_.is_open()){
                throw std::runtime_error ( "Couldn't open file" + filename ) ;
            }
        }
        short eval ( ) override  {
            short i ;
            f_ >> i ;
            return i ;
        }
    private:
        std::ifstream f_ ;
    };

    class rd_model : public model {
    public:
        rd_model ( ) {
            int s = 1574603579 ;
//            int s = time(nullptr) ;
            std::cout << "seed: " << s <<'\n' ;
            srand(s);
        }
        short eval ( ) override  {
            return static_cast<short>( ( static_cast<float>( rand() )/ static_cast<float>( RAND_MAX ) ) * max_ ) ;
        }

    private:
        short max_ = 20000  ;
    };






}

#endif //C___CHIU_MODEL_H
