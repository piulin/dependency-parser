//
// Created by pedro on 6/12/19.
//

#ifndef C___MATRIX_H
#define C___MATRIX_H


#include <cstddef>
#include <memory>
#include <iomanip>

namespace parsers::chu_liu_edmonds {

    /**
     * A matrix structure.
     * @tparam T
     */
    template < typename T >
    class matrix {
    public:
        explicit matrix ( size_t const & rows , size_t const & cols )
                : rows_ { rows } ,
                  cols_ { cols } ,
                  m_ { std::make_unique < T[] > ( rows*cols ) } { }

        template < typename U >
        matrix ( matrix < U > const & m ) : matrix { m.rows ( ) , m.cols ( ) } {
            auto p = ptr ( );
            auto r = m.ptr ( );
            for ( int i = 0 ; i < rows_*cols_ ; ++i ) {
                p[ i ] = static_cast< T > ( r[ i ] );
            }
        }

        matrix ( matrix && other ) noexcept = default;

        matrix & operator= ( const matrix & other ) = delete;

        matrix & operator= ( matrix && other ) = delete;

        [[nodiscard]] inline T const * ptr ( ) const {
            return m_.get ( );
        }

        inline T * ptr ( ) {
            return m_.get ( );
        }

        inline size_t const & rows ( ) const {
            return rows_;
        }

        inline size_t const & cols ( ) const {
            return cols_;
        }


    protected:
        size_t                      rows_ ;
        size_t                      cols_ ;
        std::unique_ptr < T[] >     m_ ;
    };

    template < typename T >
    std::ostream & operator<< ( std::ostream & o , matrix < T > const & m ) {
        o << "[\n";
        auto const p = m.ptr ( );
        o << "     " ;
        for ( int j = 0 ; j < m.cols ( ) ; ++j ) {
            o << std::setw ( 3 ) << j << ":";
        }
        o << "\n\n" ;

        for ( int i = 0 ; i < m.rows ( ) ; ++i ) {
            o << std::setw ( 3 ) << i << ": " ;
            for ( int j = 0 ; j < m.cols ( ) ; ++j ) {
                auto const & e = p[ i*m.cols ( ) + j ];
                if ( e != std::numeric_limits<T>::min () )
                    o << std::setw ( 4 ) << p[ i*m.cols ( ) + j ];
                else
                    o << std::setw ( 4 ) << "-" ;
            }
            o << '\n';
        }
        o << "]\n";
        return o;
    }

}

#endif //C___MATRIX_H
