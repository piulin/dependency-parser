//
// Created by pedro on 20/01/2020.
//

#ifndef C___OPTIONS_H
#define C___OPTIONS_H

#include <unistd.h>
#include <string>
#include <stdexcept>

namespace options {

    std::string training_set;
    std::string test_set;
    std::string dev_set ;
    std::string dump_filename;
    std::string model_filename;
    std::string predicted_set;

    size_t chunk_size = 10000 ;
    size_t epochs = 5 ;


    void parse ( int const & argc , char * * & argv ) {
        using namespace std::string_literals;
        int c;

        opterr = 0;
        while ( ( c = getopt ( argc , argv , "t:a:r:d:e:c:o:f:" ) ) != -1 ) {
            switch ( c ) {
                case 't':
                    training_set = optarg;
                    break;
                case 'a':
                    test_set = optarg;
                    break;
                case 'r':
                    model_filename = optarg;
                    break;
                case 'd':
                    dump_filename = optarg;
                    break;
                case 'o':
                    predicted_set = optarg;
                    break;
                case 'e':
                    epochs = atoi( optarg );
                    break;
                case 'c':
                    chunk_size = atoi( optarg ) ;
                    break;
                case 'f':
                    dev_set = optarg ;
                    break;
                case '?':
                    throw std::invalid_argument ( "Invalid option: -"s + char ( optopt ) );
                default:
                    throw std::invalid_argument ( "Invalid option: -"s + char ( optopt ) );
            }
        }


    }

}
#endif //C___OPTIONS_H
