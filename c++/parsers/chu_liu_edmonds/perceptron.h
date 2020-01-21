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

    class perceptron ;

    class scryer : public model < int > {
    public:
        scryer ( perceptron & p );
    protected:
        scryer (const scryer & other) = delete ;

        scryer (scryer && other) = delete ;

        scryer & operator=( const scryer & other) = delete ;
        scryer & operator=(scryer && other) = delete ;

        perceptron                                &  p_              ;
        w                                         &  w_              ;
        parsers::chu_liu_edmonds::features::tmpl  &  t_              ;
    public:
        virtual void eval ( units::sentence const & stc , matrix < int > & m ) override;
    } ;


    class perceptron : public model < int >  {


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


        size_t                                      q_              ;
        w                                           u_              ;
        size_t                                      chunk_          ;
        w                                           w_              ;
        parsers::chu_liu_edmonds::features::tmpl    t_              ;
        int dot_product ( const features::feat & f );

    protected:

        std::vector < int > to_heads ( const units::sentence & stc );

        bool equal ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        template < typename BOP >
        void update ( const std::vector < int > & heads , const units::sentence & stc , BOP && bop );

        int count_correct ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        void enlarge ( w & target );

        template < typename FEATOP >
        void impl_eval ( const units::sentence & stc , matrix < int > & m , FEATOP && featop );
    } ;




}






#endif //C___PERCEPTRON_H
