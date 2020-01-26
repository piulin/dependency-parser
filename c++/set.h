//
// Created by pedro on 22/10/19.
//

#ifndef C___SET_H
#define C___SET_H

#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdio>
#include "units.h"
#include <optional>
#include <set>
#include <unordered_map>

namespace set {


    class set {

    public:


        explicit set( std::string const & file_path, bool const & get_relations = false ) {
            FILE * fp = fopen(file_path.c_str(),"r");
//            std::ifstream coll06 ( file_path ) ;
//            if ( !coll06.is_open() ) {
//                throw std::runtime_error ( "[Failure]: file \"" + file_path + "\" not found." );
//            }
            if ( fp == nullptr ) {
                throw std::runtime_error ( "[Failure]: file \"" + file_path + "\" not found." );
            }
//            std::set < std::string > rels ;
            std::unordered_map < std::string, int > rels ;

            units::sentence stc     ;
            int     id, head        ;
            char    form[100]       ;
            char    lemma[100]      ;
            char    pos[100]        ;
            char    xpos[100]       ;
            char    morph[100]      ;
            char    rel[100]        ;
            char    useless[100]    ;

            /* Get the first line of the file. */
            char *      line_buf = nullptr ;
            size_t      line_buf_size ;
            ssize_t     line_size ;
            line_size = getline ( &line_buf , &line_buf_size , fp ) ;

            /* Loop through until we are done with the file. */
            int rel_count = 0 ;
            while (line_size >= 0) {
                if ( *line_buf != '\n' ) {
                    sscanf ( line_buf , "%d\t%s\t%s\t%s\t%s\t%s\t%d\t%s\t" ,
                             &id , form , lemma , pos , xpos , morph , &head , rel );
                    units::token tok { id , form , lemma , pos , xpos , morph , head , rel } ;
                    if ( get_relations ) {
                        auto it = rels.find ( rel ) ;
                        if ( it == rels.end () ) {
                            rels[ rel ] = rel_count ;
                            tok.irel_ = rel_count ;
                            rels_.push_back ( rel ) ;
                            rel_count++ ;
                        } else {
                            tok.irel_ = it->second ;
                        }
                    }
                    stc.add_token ( std::move ( tok ) ) ;
                } else if ( stc.size() ) {
                    sentences_.push_back( std::move( stc ) ) ;
                    stc = units::sentence { } ;
                }
                line_size = getline(&line_buf, &line_buf_size, fp);
            }
            if ( stc.size() > 0 ) {
                sentences_.push_back( std::move ( stc ) ) ;
            }
            free ( line_buf ) ;

//            for ( auto && r : rels ) {
//                rels_.emplace_back ( std::move ( r ) ) ;
//            }
            fclose(fp);

//            while ( fscanf( fp, "%d\t%s\t%s\t%s\t%s\t%s\t%d\t%s\t%s\t%s\n",
//                    &id, form, lemma, pos, xpos, morph, &head, rel, useless,useless  ) != EOF ) {
//                std::cout << form << " " << rel << "\n" ;
//            }
//            exit(0) ;
//            for (  std::string line ; getline( coll06, line ) ; ) {
//                /* then, new sentence */
//                if ( line != "" ) {
//
//                    fscanf( fp, "%d\t%s\t%s\t%s\t%s\t%s\t%d\t%s\n",&id, form, lemma, pos, xpos, morph, &head, rel  );
////                    tk.id_ = std::atoi ( strtok( line.data() , "\t") ) ;
////                    tk.form_   = strtok( nullptr, "\t") ;
////                    tk.lemma_  = strtok( nullptr, "\t") ;
////                    tk.pos_    = strtok( nullptr, "\t") ;
////                    tk.xpos_   = strtok( nullptr, "\t") ;
////                    tk.morph_  = strtok( nullptr, "\t") ;
////                    tk.head_   = std::atoi ( strtok( nullptr, "\t") ) ; /* or a pointer */
////                    tk.rel_    = strtok(nullptr, "\t") ;
//
//                    stc.add_token(  units::token { id, form, lemma, pos, xpos, morph, head, rel }  ) ;
//                } else {
//                    sentences_.push_back( std::move( stc ) ) ;
//                    stc = units::sentence { } ;
//                }
//            }
//            if ( stc.size() > 0 ) {
//                sentences_.push_back( std::move ( stc ) ) ;
//            }

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
        std::vector < std::string > rels_ ;
        std::vector < units::sentence > sentences_ ;
    };



}




#endif //C___SET_H
