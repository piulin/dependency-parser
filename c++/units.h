//
// Created by pedro on 20/10/19.
//

#ifndef C___UNITS_H
#define C___UNITS_H

#include <string>
#include <cstring>
#include <vector>
#include <ostream>

namespace units {

    struct token {

        int         id_ ;
        std::string form_;
        std::string lemma_;
        std::string pos_;
        std::string xpos_;
        std::string morph_;
        int         head_;
        std::string rel_;
        int         irel_ ;

    };


    class sentence {

    public:

        inline explicit sentence ( ) : root_ {  0, "Root", "_LR", "_PR",
                                                                "_", "_", -1, "_"  } { }

        sentence(const sentence& other) = delete;
        sentence(sentence&& other) = default;
        sentence& operator=(const sentence& other) = delete;
        sentence& operator=(sentence&& other) = default;

        inline explicit sentence ( size_t length ) : sentence()  {
            tokens_.resize( length ) ;
        }

        inline void add_token ( token tok ) {
            tokens_.push_back( std::move( tok ) );
        }

        [[nodiscard]] inline const std::vector<token> & tokens ( ) const {
            return tokens_;
        }

        [[nodiscard]] inline size_t size () const {
            return tokens_.size ( ) ;
        }

        [[nodiscard]] const token & root() const {
            return root_ ;
        }

        inline token & operator [] ( size_t const & i ) {
            return tokens_ [ i ] ;
        }
        std::vector<token> tokens_ ;
        units::token root_ ;

    };

}

#endif //C___UNITS_H
