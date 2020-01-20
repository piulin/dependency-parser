//
// Created by pedro on 5/12/19.
//

#ifndef C___PERCEPTRON_H
#define C___PERCEPTRON_H

#include <string>
#include "model.h"
#include "feat.h"
#include "../../set.h"



namespace parsers::chu_liu_edmonds::model {

    struct w {
        int * f_ ;
        int dr_ = 0 ;
        int ds_ = 0 ;
        size_t  size_ ;
    } ;



    class perceptron : public model < int > {


    public:

        explicit perceptron ( size_t const & chunk )  ;
        ~perceptron ( ) ;
        perceptron (const perceptron & other) = delete ;
        perceptron (perceptron && other) = delete ;
        perceptron & operator=(const perceptron & other) = delete ;

        perceptron & operator=(perceptron && other) = delete ;
        void eval ( units::sentence const & stc , matrix < int > & m ) override;

        void train ( set::set const & s ) ;


        void dump ( std::string const & filename ) ;


    private:
        w                                           w_              ;
        size_t const                                chunk_          ;

        parsers::chu_liu_edmonds::features::tmpl    t_              ;

        void enlarge ( const size_t & s ) ;

        int dot_product ( const features::feat & f );

        std::vector < int > to_heads ( const units::sentence & stc );

        bool equal ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        template < typename BOP >
        void update ( const std::vector < int > & heads , const units::sentence & stc , BOP && bop );

        int count_correct ( const std::vector < int > & gold , const std::vector < int > & heads ) const;
    } ;
}






#endif //C___PERCEPTRON_H
