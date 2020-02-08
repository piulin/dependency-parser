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

    /**
     * Use this class to train and predict a model suitable to fill the adjacency matrix used in chu-liu-ednomds decoder.
     * The decoder can be changed. (Not template, not added.)
     */
    class perceptron : public model < int >  {

    public:

        /**
         * Instantiate a perceptron with the weight vector being incrementally updated by <chunk> size.
         * @param chunk Chunks used to store the weight vector.
         * @param rel Target labels to be predicted with the arcs.
         */
        explicit perceptron ( size_t const & chunk, std::vector < std::string > rel )  ;

        /**
         * Load an already trained perceptron from file. It'll ready to predict.
         * @param filename
         */
        explicit perceptron ( std::string const & filename ) ;
        ~perceptron ( ) ;

        perceptron (const perceptron & other) = delete ;
        perceptron (perceptron && other) = delete ;
        perceptron & operator=(const perceptron & other) = delete ;
        perceptron & operator=(perceptron && other) = delete ;


        /**
         * This method is called by a decoder (e.g. Chu-Liu-Edmonds). The goal is to fill the adjacency matrix used later to find
         * the MSP.
         * @param stc Sentence to be parsed.
         * @param m Adjacency matrix.
         */
        virtual void eval ( units::sentence const & stc , matrix < int > & m ) override;

        /**
         * Iteratively train the perceptron given a set of gold sentences.
         * @param s set of sentences with gold arcs.
         * @param epochs number of epochs.
         * @param dev_set If desired, a dev set is assess during at the end of each epoch.
         */
        void train ( const set::set & s , const size_t & epochs , std::optional < set::set > dev_set = std::nullopt );

        /**
         * Dump the current state of the perceptron into a file. This file can later be used to instantiate a new perceptron
         * by calling the corresponding constructor.
         * @param filename File to be written.
         */
        void dump ( std::string const & filename ) const ;

        w                                           u_              ;
        size_t                                      chunk_          ;
        w                                           w_              ;
        std::vector < w >                           wl_             ;
        std::vector < std::string >                 rels_           ;
        parsers::chu_liu_edmonds::features::tmpl    t_              ;


        void predict_labels ( units::sentence & stc );

    protected:

        /**
         * Evaluates a feature given a weight vector.
         * @param f Feature to be evaluated.
         * @param fvec weight vector.
         * @return evaluation.
         */
        int dot_product ( const features::feat & f , const w & fvec ) const ;

        /**
         * Get the predicted label from a given feature.
         * @param f Feature at stake.
         * @return
         */
        int eval_label ( const features::feat & f ) const ;

        /**
         * Get a vector with the heads of each token in a sentence.
         * @param stc Sentence at stake.
         * @return Heads for each token.
         */
        std::vector < int > to_heads ( const units::sentence & stc ) const ;

        /**
         * Evaluates if to vectors are equal.
         * @param gold
         * @param heads
         * @return
         */
        bool equal ( const std::vector < int > & gold , const std::vector < int > & heads ) const;


        /**
         * Update the weight vectors of the heads given the lambda functions bopw and bopu.
         * @tparam BOPW Lambda Function
         * @tparam BOPU Lambda Function
         * @param heads dependencies at stake
         * @param stc sentence at stake..
         * @param bopw Update function for w.
         * @param bopu Update function for u (averaged p.).
         */
        template < typename BOPW, typename BOPU >
        void update ( const std::vector < int > & heads , const units::sentence & stc , BOPW && bopw, BOPU && bopu );

        /**
         * Count how many dependencies have been correctly predicted.
         * @param gold
         * @param heads
         * @return
         */
        int count_correct ( const std::vector < int > & gold , const std::vector < int > & heads ) const;

        void average ( const size_t & q ) ;

        std::unique_ptr < int[] > cpy_average ( const size_t & q ) const ;

        void test_dev_set ( const set::set & ds , const size_t & q );

        void enlarge ( w & target , const size_t & idx );

        void arcs ( const units::sentence & stc , const std::vector < int > & gold_heads );
    } ;




}






#endif //C___PERCEPTRON_H
