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
        int *       f_      ;
        size_t      size_   ;
    } ;

    class scryer : public model < int > {
    public:
        explicit scryer ( std::string const & filename ) ;
    protected:

        scryer ( );
        scryer (const scryer & other) = delete ;
        scryer (scryer && other) = delete ;
        scryer & operator=(const scryer & other) = delete ;
        scryer & operator=(scryer && other) = delete ;

        int dot_product ( const features::feat & f );

        w                                           w_              ;
        parsers::chu_liu_edmonds::features::tmpl    t_              ;
    public:
        virtual void eval ( units::sentence const & stc , matrix < int > & m ) override;
    } ;


    class perceptron : public scryer {


    public:

        explicit perceptron ( size_t const & chunk )  ;
        explicit perceptron ( std::string const & filename ) ;
        ~perceptron ( ) ;
        perceptron (const perceptron & other) = delete ;
        perceptron (perceptron && other) = delete ;
        perceptron & operator=(const perceptron & other) = delete ;
        perceptron & operator=(perceptron && other) = delete ;
        virtual void eval ( units::sentence const & stc , matrix < int > & m ) override;
        void train ( set::set const & s, size_t const & ephocs ) ;
        void dump ( std::string const & filename ) ;


    private:
        size_t                                      q_              ;
        w                                           u_              ;
        size_t                                      chunk_          ;

        std::vector < int > to_heads ( const units::sentence & stc );

        bool equal ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        template < typename BOP >
        void update ( const std::vector < int > & heads , const units::sentence & stc , BOP && bop );

        int count_correct ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        void enlarge ( w & target );

    } ;




}






#endif //C___PERCEPTRON_H
