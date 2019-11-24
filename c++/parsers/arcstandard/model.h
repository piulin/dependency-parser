//
// Created by pedro on 7/11/19.
//

#ifndef C___MODEL_H
#define C___MODEL_H

#include <array>
#include <random>

namespace  parsers::arcstandard::model {

    class model {
    public:
        virtual std::array < float , 3 >  eval ( ) = 0 ;
    } ;

    class rd_model : public model {
    public:

        explicit rd_model () ;

        std::array<float, 3>  eval ( ) ;
    };

    class tr_model : public model {

    public:
        explicit tr_model ( std::vector < std::array < float , 3 > > transitions ) ;

        std::array<float, 3> eval ( ) ;

    private:
        std::vector < std::array < float , 3 > > trans_ ;
        size_t ctr_ ;
    };




}
#endif //C___MODEL_H
