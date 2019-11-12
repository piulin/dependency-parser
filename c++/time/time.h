/* Stopwatch.
    Authors: Pedro G. Bascoy, F. Arguello, Alberto Suarez Garea, and Dora B. Heras,
   © 2019 Universidade de Santiago de Compostela. All rights reserved.
*/
#ifndef TIME_H
#define TIME_H

#include <sys/time.h>
#include <string>
#include <stack>
#include <iostream>
#include <iomanip>
#include <optional>
#include "table.h"


namespace timing {

    class stopwatch
    {
    public:
        using time_t = struct timeval ;
        time_t instantiated_ ;
        ordered::table < std::string, double, double, std::string > time_table_ ;
        std::vector < std::pair< time_t , std::string > > times_ ;

        explicit stopwatch ( ) {
            gettimeofday ( &instantiated_ , NULL ) ;
         }

        ~stopwatch () = default;
        stopwatch ( const stopwatch & other ) = default ;
        stopwatch ( stopwatch && other ) = default ;
        stopwatch & operator= ( const stopwatch & other ) = default ;
        stopwatch & operator= ( stopwatch && other ) = default ;

        inline void start ( std::string const & message ) {
            time_t m ;
        	gettimeofday( &m, NULL ) ;
            double const elapsed_time_since_instantiation = ( m.tv_sec + ( m.tv_usec * 1E-6)) - (instantiated_.tv_sec + (instantiated_.tv_usec * 1E-6)) ;
            std::cout << "[TIMER_STARTED at " << std::fixed <<std::setprecision(2) << elapsed_time_since_instantiation << "]: " << message << "...\n" ;
            times_.push_back ( std::make_pair ( std::move( m ), std::move( message ) ) ) ;
        }

        inline void stop ( std::optional < std::string > opmessage = std::nullopt ) {
            time_t end ;
        	gettimeofday( &end, NULL ) ;
            auto it = [&] ( ) {
                if ( opmessage == std::nullopt ) {
                    return times_.end ( ) - 1 ;
                } else {
                    for ( auto ri = times_.begin ( ) ; ri != times_.end( ) ; ++ri ) {
                        auto & curr_pair = *ri ;
                        if ( curr_pair.second == opmessage.value ( ) ) {
                            return ri ;
                        }
                    }
                }
                return times_.end ( ) - 1 ;
            } ( ) ;
            auto & last = *it ;
            double const elapsed_time_last_start = ( end.tv_sec + ( end.tv_usec * 1E-6 ) ) - (last.first.tv_sec + (last.first.tv_usec * 1E-6)) ;
            double const elapsed_time_since_instantiation = ( last.first.tv_sec + ( last.first.tv_usec * 1E-6)) - (instantiated_.tv_sec + (instantiated_.tv_usec * 1E-6)) ;

            auto message = opmessage.value_or ( last.second ) ;
            std::cout << "[TIMER_STOPPED]: " << message << ": " << std::fixed << std::setprecision(4) << elapsed_time_last_start << " s\n\n" ;
        	time_table_.new_register ( std::move( message ) ,  elapsed_time_since_instantiation, elapsed_time_last_start , "seconds" ) ;
            times_.erase ( it ) ;
        }

    } ;



}


#endif
