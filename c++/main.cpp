#include <iostream>
#include "set.h"
#include "assessment.h"
#include "parsers/arcstandard/model.h"
#include "time/time.h"
#include "parsers/set_parser.h"
#include "parsers/chu_liu_edmonds/chu_liu_edmonds.h"


std::string const path_train = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/" ;
std::string const path_dev = "/home/pedro/Documentos/CL/SDP/lab/datasets/english/dev/" ;


int main ( ) {

    timing::stopwatch st ;




    st.start( "Reading set from file" ) ;
//    set::set s( "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/wsj_train.only-projective.first-1k.conll06" ) ;
    set::set s( path_train + "wsj_train.only-projective.conll06" ) ;
//    set::set s( "/home/pedro/Documentos/CL/SDP/lab/datasets/graph/s.conll06.txt" ) ;
//    set::set s( "/home/pedro/Documentos/CL/SDP/lab/datasets/english/train/sample3.conll06" ) ;
    st.stop( ) ;

    parsers::chu_liu_edmonds::stc_parser graph_parser { s.sentences()[ 0 ]  } ;
    parsers::chu_liu_edmonds::model::file_model fm { "/home/pedro/Documentos/CL/SDP/lab/datasets/graph/random.txt" } ;
    parsers::chu_liu_edmonds::model::rd_model crm ;

    parsers::set_parser<parsers::chu_liu_edmonds::stc_parser> grapph_parser { s }  ;

    st.start( "Graph parser" ) ;
    auto gparsed = grapph_parser.parse ( [&crm] (units::sentence const & s, parsers::chu_liu_edmonds::stc_parser & e) {
        return e.parse(crm ) ;
    } ) ;

    st.stop( ) ;
    gparsed.write(std::ofstream("c.txt"));
    std::cout << "UAS (graph): \n" << assessment::uas(s, gparsed) ;

    std::exit(0);


    parsers::set_parser<parsers::arcstandard::stc_parser> random_parser { s } ;
    parsers::set_parser<parsers::arcstandard::stc_parser> transition_parser { s } ;

    st.start("Transition parsing");
    auto transition_set = transition_parser.parse ( [] ( auto const & s, auto & p ) {
        parsers::arcstandard::smart_oracle smor { s, p } ;
        parsers::arcstandard::oracle_model m { smor } ;
        return p.parse( m ) ;
    } ) ;
    st.stop( ) ;

    auto rm = parsers::arcstandard::model::rd_model() ;


    st.start("Assess") ;
    std::cout << "UAS (transition): \n" << assessment::uas(s, transition_set) ;
    st.stop() ;

    st.time_table_.to_csv("reference.csv") ;

}