//
// Created by pedro on 17/11/19.
//

#ifndef C___CHU_LIU_EDMONDS_H
#define C___CHU_LIU_EDMONDS_H

#include <memory>
#include <algorithm>
#include "../../units.h"
#include "../parser.h"
#include "model.h"
#include <iomanip>

namespace parsers::chu_liu_edmonds {


    template < typename T >
    std::ostream & operator << ( std::ostream & o , std::vector < T > const & v ) {
        for( auto const & e : v ) {
//            std::cout << e << " " ;
        }
        return o ;
    }

    template < typename T >
    class matrix {
    public:
        explicit matrix ( size_t const & rows, size_t const & cols )
        : rows_ { rows } ,
        cols_ { cols } ,
        m_ { std::make_unique < T [] > ( rows*cols ) } { }

        template < typename U >
        matrix ( matrix<U> const & m ) : matrix { m.rows() , m.cols() } {
            auto p = ptr() ;
            auto r = m.ptr() ;
            for (int i = 0; i < rows_*cols_; ++i) {
                p [ i ] = static_cast< T > ( r [ i ] ) ;
            }
        }

        matrix (matrix && other) noexcept = default ;
        matrix & operator=(const matrix & other) = delete;
        matrix & operator=(matrix && other) = delete;

        [[nodiscard]] inline T const * ptr ( ) const {
            return m_.get ( ) ;
        }
        inline T * ptr ( )  {
            return m_.get ( ) ;
        }

        inline size_t const & rows() const {
            return rows_;
        }

        inline size_t const & cols() const {
            return cols_;
        }


    protected:
        size_t                          rows_       ;
        size_t                          cols_       ;
        std::unique_ptr < T [] >        m_          ;
    }   ;

    template < typename  T >
    std::ostream & operator << ( std::ostream & o , matrix < T > const & m ) {
        o << "[\n" ;
        auto const p = m.ptr() ;
        for (int i = 0; i < m.rows(); ++i) {
            for (int j = 0; j < m.cols() ; ++j) {
                o << std::setw (4) << p [ i*m.cols() + j ];
            }
            o << '\n' ;
        }
        o << "]\n" ;
        return o ;
    }


    class adjacency : public matrix < bool >  {

    private:

        [[nodiscard]] std::vector < int > loop_list ( int const & node, int const & path_size ) const {
            std::vector < int >  edge_list ( path_size + 1 ) ;
            std::fill ( edge_list.begin() + 1, edge_list.end(), -1 ) ;
            edge_list [ 0 ] = node ;
            int curr_node = node ;
            auto p = ptr() ;
            for ( int i = 0; i <= path_size ; ++i ) {
                bool dead_end = true ;
                for ( int col = edge_list [ i + 1 ] + 1 ; col < cols_ ; col++ ) {
                    if ( p [ curr_node*cols_ + col ] ) {
                        curr_node = col ;
                        edge_list [ i + 1 ] = curr_node ;
                        dead_end = false ;
                        break ;
                    }
                }
                if ( curr_node == node && i == path_size - 1 ) {
                    break ;
                } else if ( dead_end ) {
                    curr_node = edge_list [ i - 1 ] ;
                    edge_list [ i + 1 ] = -1 ;
                    i -= 2 ;
                } else if ( i == path_size - 1 ) {
                    curr_node = edge_list [ i ] ;
                    i -- ;
                }
            }
            edge_list. pop_back ( ) ;
            return edge_list ;
        }

    public:
        explicit adjacency (size_t const & rows, size_t const & cols) : matrix < bool > { rows , cols } {
            if ( rows_ != cols ) {
                throw std::runtime_error( "Rows and cols must be of the same size." ) ;
            }
            std::fill( this->ptr(), this->ptr() + rows_*cols_, 0 ) ;
        }

        /**
         *
         * @return node where the loops begins
         */
        [[nodiscard]] std::vector < int > is_there_loop ( ) const {
            using mul_type = short ;
            matrix < mul_type > r1 {rows_, cols_ } ;
            matrix < mul_type > r2 = *this ;

            mul_type * pres = r1.ptr() ;
            mul_type * pmul = r2.ptr() ;
            bool const * padj = ptr() ;
            /* first iteration */
            for (int l = 0; l < rows_ ; ++l) {
                for(int i=0;i<rows_;i++) {
                    for(int j=0;j<cols_;j++) {
                        pres [ i*cols_+j ] = 0 ;
                        for(int k=0;k<cols_;k++) {
                            pres [ i*cols_+j ] +=  pmul [ i * cols_ + k ] * static_cast<mul_type> ( padj [ k * cols_ + j ] ) ;
                        }
                        if ( i==j && pres [ i*cols_+j ] > 0 ) {
//                            std::cout << "LOOP FOUND: " << l+2 << '\n';
                            return loop_list( i, l + 2 ) ;
                        }
                    }
                }
                std::swap( pres , pmul ) ;
            }
            return { } ;
        }

        units::sentence to_stc ( units::sentence const & s ) const {
            units::sentence ns ;
            auto p = ptr() ;
            for (int i = 0; i < s.size ( ) ; ++i) {
                units::token t = s.tokens_ [ i ] ;
                int row = -1 ;
                for (int j = 0; j < rows_ ; ++j) {
                    if ( p [ j*cols_ + i + 1 ] ) {
                        row = j ;
                        break;
                    }
                }
                t.head_ = row ;
                ns.tokens_.push_back( std::move ( t ) ) ;
            }
            return ns ;
        }



    } ;

    template < typename T >
    class cost : public matrix < T > {
    public:
        /* cost will be organised the other way round, i.e. a transposed matrix. This is done due to efficiency affairs. */
        explicit cost (size_t const & rows, size_t const & cols) : matrix < T > { rows , cols } {
            std::fill( this->ptr(), this->ptr() + rows*cols , -1  ) ;
        }

        [[nodiscard]] adjacency adj (  ) const {
            auto const p    = this->ptr() ;
            auto const & r  = this->rows_ ;
            auto const & c  = this->cols_ ;
            adjacency ad { c, c } ;
            auto a = ad.ptr ( ) ;
            for ( int i = 0; i < r ; ++i ) {
                    int j = std::max_element( p + i*c, p + ( i + 1 )*c ) - (p + i*c) ;
                    a [ j*c + i + 1 ] = true ;
            }
            return ad ;
        }

        struct contract_t {
            cost c ;
            int del_from ;
            int del_to ;
            int node ;
        };

        contract_t contract ( std::vector < int > const & edges ) const  {
            /* save the VC in the last position of the new graph */
            int no_edges = edges.size () ;
            auto const p = this->ptr() ;
            auto const & r  = this->rows_ ;
            auto const & c  = this->cols_ ;
            cost cs { r + 1 - no_edges , c + 1 - no_edges } ;
            auto const csp = cs.ptr() ;
            auto const & csr  = cs.rows_ ;
            auto const & csc  = cs.cols_ ;

            /* prepare the destinantions indexes */
            std::vector < int > destin_rows ( r ) ;
            for (int i = 0; i < r ; ++i) { destin_rows [ i ] = i + 1 ; }
            for ( auto const & e : edges ) {
                destin_rows.erase(std::find (destin_rows.begin ( ) , destin_rows.end ( ) , e ) ) ;
            }
            std::vector < int > destin_cols = destin_rows ;
            destin_cols.insert(destin_cols.begin(), 0 ) ;
            std::for_each(destin_rows.begin(), destin_rows.end(), [] (auto & e ) { e--; } ) ;

            std::vector < int > pos_rows ( r ) ;
            std::vector < int > pos_cols ( c ) ;


            /* copy the rows and columns that are not supposed to be modified. */
            {
                int i = 0 , j = 0 ;
                for ( auto const & row : destin_rows ) {
                    pos_rows [ row ] = i ;
                    for ( auto const & col : destin_cols ) {
                        csp [ i*csc + j ] = p [ row*c + col ] ;
                        ++j ;
                    }
                    ++i ;
                    j = 0 ;
                }
                i=0 ;
                for (auto const & col :destin_cols ) {
                    pos_cols [ col ] = i;
                    ++i ;
                }
            }

//            std::cout << cs ;


            /* leaving arcs */
            for ( auto const & row : destin_rows ) {
                int max = -1 ;
                int max_row = -1 ;
                for ( auto const & col : edges ) {
                    if ( p [ row*c + col ] > max ) {
                        max = p [ row*c + col ] ;
                        max_row = row ;
                    }
                }
                csp [csc - 1 + pos_rows [ max_row ] * csc ] = max ;
            }

            /* incoming arcs */
            auto [minscore,maxscore] = std::minmax_element(edges.begin(),edges.end());
            std::vector < int > scores ( *maxscore - *minscore + 1 ) ;
            int score_loop = 0 ;

            {
                for (int i = 0, j = 1; i < edges.size() - 1; ++i, ++j ) {
                    int const & from = edges[ i ] ;
                    int const & to = edges[ i + 1 ] - 1 ;
                    int const & v = p [ from + to*c ] ;
                    score_loop += v ;
                    scores [ edges [ j ] - *minscore ] = v ;
                }
                int const & f = p [ edges.back() + ( edges[ 0 ] - 1 )*c ] ;
                scores [ edges [ 0 ] - *minscore ] = f;
                score_loop += f ;
            }

            std::vector < int > through ( csc ) ;
            int glmax = -1 ;
            int glmax_row = -1 ;
            int max_col = -1 ;
            for ( auto const & col : destin_cols ) {
                int max = -1 ;
                int max_row = -1 ;
                for ( int row : edges ) {
                    row-- ;
                    int v = p [ row*c + col ] + score_loop - scores [ row - *minscore + 1 ] ;
                    if ( v > max ) {
                        max = v ;
                        max_row = row + 1 ;
                    }
                }
                if ( max > glmax ) {
                    glmax = max ;
                    glmax_row = max_row ;
                    max_col = col ;
                }
                csp [ (csr - 1)*csc + pos_cols [ col ] ] = max ;
//                std::cout << cs ;
            }

            int del = -1 ;
            auto pos = std::find ( edges.begin(), edges.end(), glmax_row ) ;
            if ( pos != edges.begin() ) {
                del = *(pos-1) ;
            } else {
                del = edges.back ( ) ;
            }
//            std::cout << "SHRINKED: " << cs ;

            return { std::move(cs), del, glmax_row, max_col }  ;
        }

    private:

    } ;


    class stc_parser  {
    public:

        explicit stc_parser ( units::sentence const & s ) :
        s_ { s }
        /* first column not needed */{

        }

        /* chapuza */
        void fill_costs_matrix ( parsers::chu_liu_edmonds::model::model & m, matrix <int > & cm ) {
            int * pm = cm.ptr ( ) ;
            for (int i = 0; i < cm.rows(); ++i) {
                for (int j = 0; j < cm.cols(); ++j) {
                    if ( i+1 == j  ) {
                        pm [ i*cm.cols() + j ] = -1 ;
                    } else {
                        pm [ i*cm.cols() + j ] = m.eval() ;
                    }
                }
            }
        }

        units::sentence parse ( parsers::chu_liu_edmonds::model::model & m )  {
            /* +1, because of root */
            cost < int > cm  { s_.size() , s_.size() + 1 }   ;
            fill_costs_matrix( m, cm ) ;
//            std::cout << cm ;
            return chu_liu_edmonds( cm ).to_stc( s_ ) ;
        }

        void solve_cycle(adjacency & inital, int const & from, int const & to, int const & node) {

            auto ip = inital.ptr() ;
            auto const & ips = inital.rows() ;

            ip [ node*ips + to ] = true ;
            ip [ from*ips + to ] = false ;

//            std::cout << inital ;

        }

        adjacency chu_liu_edmonds (cost < int > const & cm ) {
            adjacency a = cm.adj() ;
//            std::cout << "ADJACENCY MATRIX: " << a ;
            std::vector < int > loop = a.is_there_loop ( ) ;
//            std::cout << "Loop list: " << loop << '\n';
            if ( !loop.empty() ) {
                auto [ s , from, to , node ] = cm.contract( loop ) ;
                adjacency an = chu_liu_edmonds( s ) ;
                solve_cycle ( a, from, to, node ) ;

            }
            return a ;
        }
        units::sentence const &     s_      ;
    }   ;




}

#endif //C___CHU_LIU_EDMONDS_H
