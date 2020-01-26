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


    class perceptron : public model < int >  {

    public:

        explicit perceptron ( size_t const & chunk, std::vector < std::string > )  ;
        explicit perceptron ( std::string const & filename ) ;
        ~perceptron ( ) ;
        perceptron (const perceptron & other) = delete ;
        perceptron (perceptron && other) = delete ;
        perceptron & operator=(const perceptron & other) = delete ;
        perceptron & operator=(perceptron && other) = delete ;
        virtual void eval ( units::sentence const & stc , matrix < int > & m ) override;
        void train ( const set::set & s , const size_t & ephocs , std::optional < set::set > dev_set = std::nullopt );

        void dump ( std::string const & filename ) ;
        w                                           u_              ;
        size_t                                      chunk_          ;
        w                                           w_              ;
        std::vector < w >                           wl_             ;
        std::vector < std::string >                 rels_           ;
        parsers::chu_liu_edmonds::features::tmpl    t_              ;



        int dot_product ( const features::feat & f , const w & fvec );

        int eval_label ( const features::feat & f );

        void predict_labels ( units::sentence & stc );

    protected:

        std::vector < int > to_heads ( const units::sentence & stc );

        bool equal ( const std::vector < int > & gold , const std::vector < int > & heads ) const;


        template < typename BOPW, typename BOPU >
        void update ( const std::vector < int > & heads , const units::sentence & stc , BOPW && bopw, BOPU && bopu );

        int count_correct ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        void average ( const size_t & q );

        std::unique_ptr < int[] > cpy_average ( const size_t & q );

        void test_dev_set ( const set::set & ds , const size_t & q );

        void enlarge ( w & target , const size_t & idx );


        void arcs ( const units::sentence & stc , const std::vector < int > & gold_heads );
    } ;




}






#endif //C___PERCEPTRON_H
