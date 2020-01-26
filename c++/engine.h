//
// Created by pedro on 20/01/2020.
//

#ifndef C___ENGINE_H
#define C___ENGINE_H

#include <iostream>
#include "set.h"
#include "assessment.h"
//#include "parsers/arcstandard/model.h"
#include "time/time.h"
#include "parsers/set_parser.h"
#include "parsers/chu_liu_edmonds/chu_liu_edmonds.h"
#include "parsers/chu_liu_edmonds/feat.h"
#include "parsers/chu_liu_edmonds/perceptron.h"
#include "options.h"


namespace engine {

    timing::stopwatch st;

    void scry ( parsers::chu_liu_edmonds::model::perceptron & scry ) {
        if ( !options::test_set.empty () ) {
            set::set s { options::test_set } ;
            parsers::set_parser  grapph_parser { s } ;

            st.start ( "Chu-Liu-Edmonds parsing" );
            auto gparsed = grapph_parser.parse (
                    [ &scry ] ( units::sentence const & s , parsers::chu_liu_edmonds::stc_parser & e ) {
                        units::sentence ns =  e.parse ( scry ) ;
                        scry.predict_labels ( ns ) ;
                        return ns ;
                    } );
            st.stop ( );

            if ( !options::predicted_set.empty ( ) ) {
                gparsed.write ( std::ofstream ( options::predicted_set ) );
            }
            std::cout << "UAS (graph): \n" << assessment::uas ( s , gparsed );
            std::cout << "LAS (graph): \n" << assessment::las ( s , gparsed );
        }
    }


    void run ( ) {

        if ( ! options::training_set.empty () ) {
            st.start ( "Reading training set from file" );
            set::set s { options::training_set, true } ;
            st.stop ( );
            parsers::chu_liu_edmonds::model::perceptron prcp { options::chunk_size, s.rels_ } ;

            st.start ( "Training perceptron" );
            if ( !options::dev_set.empty ( ) ) {
                prcp.train ( s , options::ephocs , set::set { options::dev_set } );
            } else {
                prcp.train ( s , options::ephocs );
            }
            st.stop ( );

            if ( !options::dump_filename.empty () ) {
                st.start ( "Dumping perceptron" );
                prcp.dump ( options::dump_filename );
                st.stop ( );
            }
            scry ( prcp ) ;

        } else if ( !options::model_filename.empty () ) {
            st.start ( "Reading model from file" ) ;
            parsers::chu_liu_edmonds::model::perceptron prcp { options::model_filename } ;
            st.stop ( ) ;
            scry ( prcp ) ;
        }
        st.time_table_.to_csv ( "execution_times.csv" ) ;
    }
}

#endif //C___ENGINE_H
