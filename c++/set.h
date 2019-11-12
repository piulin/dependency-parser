//
// Created by pedro on 22/10/19.
//

#ifndef C___SET_H
#define C___SET_H

#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>
#include "units.h"

namespace set {


    class set {

    public:

        explicit set( std::string const & file_path ) {

            std::ifstream coll06 ( file_path ) ;
            if ( !coll06.is_open() ) {
                throw std::runtime_error ( "[Failure]: file \"" + file_path + "\" not found." );
            }

            units::sentence stc ;
            for (  std::string line ; getline( coll06, line ) ; ) {
                /* then, new sentence */
                if ( line != "" ) {

                    int const id = std::atoi ( strtok( line.data() , "\t") ) ;
                    std::string form   = strtok( nullptr, "\t") ;
                    std::string lemma  = strtok( nullptr, "\t") ;
                    std::string pos    = strtok( nullptr, "\t") ;
                    std::string xpos   = strtok( nullptr, "\t") ;
                    std::string morph  = strtok( nullptr, "\t") ;
                    int const   head         = std::atoi ( strtok( nullptr, "\t") ) ; /* or a pointer */
                    std::string rel    = strtok(nullptr, "\t") ;

                    stc.add_token( units::token { id,
                                                  std::move(form),
                                                  std::move(lemma),
                                                  std::move(pos),
                                                  std::move(xpos),
                                                  std::move(morph),
                                                  head,
                                                  std::move(rel) } ) ;
                } else {
                    sentences_.push_back( std::move( stc ) ) ;
                    stc = units::sentence { } ;
                }
            }
            if ( stc.size() > 0 ) {
                sentences_.push_back( std::move ( stc ) ) ;
            }

        }

        explicit set ( std::vector < units::sentence > & stcs ) :
        sentences_ { std::move ( stcs ) } { }


        template<typename P>
        void write(P && printer) {
            for ( units::sentence const & stc : sentences_ ) {
                for ( units::token const & tok : stc.tokens() ) {
                    printer << tok.id_ << "\t"
                            << tok.form_ << "\t"
                            << tok.lemma_ << "\t"
                            << tok.pos_ << "\t"
                            << tok.xpos_ << "\t"
                            << tok.morph_ << "\t"
                            << tok.head_ << "\t"
                            << tok.rel_ << "\t"
                            << "_\t_\n" ;
                }
                printer << "\n" ;
            }

        }

        [[nodiscard]] inline const std::vector<units::sentence> & sentences () const {
            return sentences_;
        }

        std::vector < units::sentence > sentences_ ;
    };



}




#endif //C___SET_H
