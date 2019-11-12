#include <iostream>
#include "set.h"
#include "assessment.h"
#include "model.h"
#include "time/time.h"
#include "parsers/parser.h"


std::string const path_train = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/" ;
std::string const path_dev = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/dev/" ;


int main ( ) {

    timing::stopwatch st ;

    st.start( "Reading set from file" ) ;
//    set::set s( "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/wsj_train.only-projective.first-1k.conll06" ) ;
    set::set s( "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/wsj_train.only-projective.conll06" ) ;
    st.stop( ) ;
//    set::set s( "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/sample2.conll06" ) ;
    parsers::parser random_parser { s } ;
    parsers::parser transition_parser { s } ;

    st.start("Transition parsing");
    auto transition_set = transition_parser.oracle_parse ( ) ;
    st.stop( ) ;

    auto rm = model::rd_model() ;

    st.start( "Random parsing" ) ;
    auto random_set = random_parser.parse(rm ) ;
    st.stop();


    st.start("Assess") ;
    std::cout << "UAS (random): \n" << assessment::uas(s, random_set) ;
    std::cout << "UAS (transition): \n" << assessment::uas(s, transition_set) ;
    st.stop() ;

    st.time_table_.to_csv("reference.csv") ;

}