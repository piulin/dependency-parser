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

    void scry ( parsers::chu_liu_edmonds::model::scryer & scry ) {
        if ( !options::test_set.empty () ) {
            set::set s { options::test_set } ;
            parsers::set_parser < parsers::chu_liu_edmonds::stc_parser > grapph_parser { s } ;

            st.start ( "Chu-Liu-Edmonds parsing" );
            auto gparsed = grapph_parser.parse (
                    [ &scry ] ( units::sentence const & s , parsers::chu_liu_edmonds::stc_parser & e ) {
                        return e.parse ( scry );
                    } );
            st.stop ( );

            if ( !options::predicted_set.empty ( ) ) {
                gparsed.write ( std::ofstream ( options::predicted_set ) );
            }
            std::cout << "UAS (graph): \n" << assessment::uas ( s , gparsed );
        }
    }


    void run ( ) {

        if ( ! options::training_set.empty () ) {
            st.start ( "Reading training set from file" );
            set::set s( options::training_set ) ;
            st.stop ( );
            parsers::chu_liu_edmonds::model::perceptron prcp { options::chunk_size };


            st.start ( "Training perceptron" );
            prcp.train ( s, options::ephocs );
            st.stop ( );

            if ( !options::dump_filename.empty () ) {
                st.start ( "Dumping perceptron" );
                prcp.dump ( options::dump_filename );
                st.stop ( );
            }
            parsers::chu_liu_edmonds::model::scryer sc { prcp };
            scry ( sc ) ;

        } else if ( !options::model_filename.empty () ) {
            st.start ( "Reading model from file" ) ;
            parsers::chu_liu_edmonds::model::perceptron prcp { options::model_filename };
            parsers::chu_liu_edmonds::model::scryer sc { prcp } ;
            st.stop ( ) ;
            scry ( sc ) ;
        }
        st.time_table_.to_csv ( "execution_times.csv" ) ;
    }
}

#endif //C___ENGINE_H
