//
// Created by pedro on 17/11/19.
//

#ifndef C___CHU_LIU_EDMONDS_H
#define C___CHU_LIU_EDMONDS_H

#include <memory>
#include <algorithm>
#include "../../units.h"
#include "matrix.h"
#include "model.h"
#include <iomanip>
#include <tuple>
#include <functional>

namespace parsers::chu_liu_edmonds {

    template < typename T >
    std::ostream & operator<< ( std::ostream & o , std::vector < T > const & m ) {
        o << "[ " ;
        for ( auto const & e : m ){
            o << e << " " ;
        }
        o << "]";
        return o;
    }

    /**
     * Stores adjacency matrix of a graph.
     */
    class adjacency : public matrix < bool > {

    private:
        /**
         * Calculates the nodes that form a loop given some candidates.
         * @param candidates    List of nodes that the loop can consist of.
         * @param loop_size     Size of the desired loop.
         * @return  List of nodes that form a loop of the desired size
         */
        [[nodiscard]] std::vector < int > loop_list ( std::vector < int > const & candidates ,
                                                      int const & loop_size ) const {
            /* List of nodes that is iteratively updated to find the nodes of a loop. This list is ordered. */
            std::vector < int > edge_list ( loop_size + 1 , -1 );
            /* Stores the indexes of the candidates being used during the iterative part of the algorithm. */
            std::vector < short > loop_ixs ( loop_size , 0 );

            int const node = candidates[ 0 ];
            edge_list[ 0 ] = candidates[ 0 ];
            int curr_node = node;
            auto p = ptr ( );

            /* Iterate until we have a valid loop (conditions down) */
            for ( int i = 0 ; i <= loop_size ; ++i ) {
                bool dead_end = true;
                /* iterate the columns of the candidates, and check if a path through them is possible */
                for ( short & node_index = loop_ixs[ i ] ; node_index < candidates.size ( ) ; node_index++ ) {
                    int col = candidates[ node_index ];
                    if ( p[ curr_node*cols_ + col ] ) {
                        curr_node = col;
                        edge_list[ i + 1 ] = curr_node;
                        dead_end = false;
                        node_index++;
                        break;
                    }
                }
                /* if the final node is the first node, then we're done. */
                if ( curr_node == node && i == loop_size - 1 ) {
                    break;
                } else if ( dead_end ) { /* if there is no path available to continue,
 *                                          then go back and try the next one. */
                    curr_node = edge_list[ i - 1 ];
                    loop_ixs[ i ] = 0;
                    i -= 2;
                } else if ( i == loop_size - 1 ) { /* if we are exceeding the desired loop size,
 *                                                  try other way in the same node */
                    curr_node = edge_list[ i ];
                    i--;
                }
            }
            edge_list.pop_back ( ); /* one is repeated */
            return edge_list;
        }

    public:
        explicit adjacency ( size_t const & rows , size_t const & cols ) : matrix < bool > { rows , cols } {
            if ( rows_ != cols ) {
                throw std::runtime_error ( "Rows and cols must be of the same size." );
            }
            std::fill ( this->ptr ( ) , this->ptr ( ) + rows_*cols_ , 0 );
        }


        [[nodiscard]] int empty_column ( ) const {
            for ( int i = 1 ; i < cols_ ; ++i ) {
                int c = 0 ;
                for ( int j = 0 ; j < rows_ ; ++j ) {
                    c += static_cast<int> ( ptr ()[j*cols_+i] );
                }
                if ( c == 0 ) {
                    return i ;
                }
            }
            return -1 ;
        }

        /**
         * Find the first shortest loop in the adjacency matrix and return the its nodes. The nodes are given in circular
         * order.
         *
         * To find suitable loops, m^[i] for all i from 1 to [columns] is performed, then check if the diagonal contains
         * non-null values. If so, the elements of the diagonal are the candidates of the loop.
         *
         * @return List of ordered nodes within a loop. If no loop is found, then an empty list is returned.
         */
        [[nodiscard]] std::vector < int > is_there_loop ( ) const {
            using mul_t = short;
            /* matrices for storing intermediate results */
            matrix < mul_t > r1 { rows_ , cols_ };
            matrix < mul_t > r2 = *this;

            /* pointers of the matrices */
            mul_t * pres = r1.ptr ( );
            mul_t * pmul = r2.ptr ( );
            bool const * padj = ptr ( );

            std::vector < int > nodes_in_loops;
            /* Loop for "to the square of l" matrix calculation */
            for ( int l = 0 ; l < rows_ ; ++l ) {

                /* loops for matrix multiplication */
                #pragma omp parallel for default(shared)
                for ( int i = 0 ; i < rows_ ; i++ ) {
                    for ( int j = 0 ; j < cols_ ; j++ ) {
                        pres[ i*cols_ + j ] = 0;
                        for ( int k = 0 ; k < cols_ ; k++ ) {
                            pres[ i*cols_ + j ] +=
                                    pmul[ i*cols_ + k ]*static_cast<mul_t> ( padj[ k*cols_ + j ] );
                        }
                    }
                }


                /* count number of occurrences in diagonal */
                for ( int m = 0 ; m < rows_ ; ++m ) {
                    if ( pres[ m*cols_ + m ] > 0 ) {
                        nodes_in_loops.emplace_back ( m );
                    }
                }
                /* of we got some elements, then we've got a loop */
                if ( !nodes_in_loops.empty ( ) ) {
                    return loop_list ( nodes_in_loops , l + 2 );
                }

                /* otherwise, go on */
                std::swap ( pres , pmul );
                nodes_in_loops.clear ( );
            }
            /* if no loops, return an empty list */
            return { };
        }

        /**
         * Parse a sentence given a adjacency matrix. This matris MUST NOT CONTAIN LOOPS. This condition is NOT checked.
         * @param s Sentence to be parsed.
         * @return new parsed sentence
         */
        [[nodiscard]] units::sentence to_stc ( units::sentence const & s ) const {
            units::sentence ns;
            auto p = ptr ( );

            /* iterate the tokens */
            for ( int i = 0 ; i < s.size ( ) ; ++i ) {
                units::token t = s.tokens_[ i ];
                int row = -1;
                /* get the head of the token */
                for ( int j = 0 ; j < rows_ ; ++j ) {
                    if ( p[ j*cols_ + i + 1 ] ) {
                        row = j;
                        break;
                    }
                }
                /* update it */
                t.head_ = row;
                ns.tokens_.push_back ( std::move ( t ) );
            }
            return ns ;
        }

        [[nodiscard]] std::vector < int > to_heads ( ) const {
            auto p = ptr ( ) ;
            std::vector < int > heads ( rows_ - 1 ) ;
            for ( int i = 1 ; i < cols_ ; ++i ) {
                int row = -1;
                /* get the head of the token */
                for ( int j = 0 ; j < rows_ ; ++j ) {
                    if ( p[ j*rows_ + i ] ) {
                        row = j ;
                        break;
                    }
                }
                heads [ i - 1 ] = row ;
            }
            return heads ;
        }

    };

    /**
     * Stores a cost matrix of a graph. The layout of the matrix is transposed as compared to the definition of
     * cost matrix.
     * @tparam T type of the cost matrix.
     */
    template < typename T >
    class cost : public matrix < T > {
    public:
        /* cost will be organised the other way round, i.e. a transposed matrix. This is done due to efficiency affairs. */
        explicit cost ( size_t const & rows , size_t const & cols ) : matrix < T > { rows , cols } {
            std::fill ( this->ptr ( ) , this->ptr ( ) + rows*cols , std::numeric_limits<T>::min () );
        }

        /**
         * gives the optimal adjacency matrix that maximizes the cost function (max_element).
         * @return adjacency matrix.
         */
        [[nodiscard]] adjacency adj ( ) const {
            auto const p = this->ptr ( );
            auto const & r = this->rows_;
            auto const & c = this->cols_;
            adjacency ad { c , c } ;
            auto a = ad.ptr ( );
            #pragma omp parallel for default (shared)
            for ( int i = 0 ; i < r ; ++i ) {
                /* cost function that is maximized */
                int j = std::max_element ( p + i*c , p + ( i + 1 )*c ) - ( p + i*c ) ;
                a[ j*c + i + 1 ] = true;
            }
            return ad;
        }

        /**
         * Contract the nodes [edges] of the cost matrix given the cost function (max_element)
         * @param edges edges to be contracted.
         * @return      new cost matrix with a new node added as result of the contraction of nodes [edges]
         *              to the last column and rows.
         */
        [[nodiscard]] auto contract ( std::vector < int > const & edges ) const {
            /* save the VC in the last position of the new graph */
            int const no_edges = edges.size ( );

            /* current cost matrix properties */
            /* NOTICE THAT the cost matrix is NOT a square matrix */
            auto const & r = this->rows_;

            /* Prepare a list with the indices of non contracted nodes for the rows  */
            std::vector < int > non_contracted_rows ( r );
            for ( int i = 0 ; i < r ; ++i ) { non_contracted_rows[ i ] = i + 1; }
            for ( auto const & e : edges ) {
                non_contracted_rows.erase (
                        std::find ( non_contracted_rows.begin ( ) , non_contracted_rows.end ( ) , e ) );
            }
            /* Prepare a list with the indices of non contracted nodes for the cols  */
            std::vector < int > non_contracted_cols = non_contracted_rows;
            non_contracted_cols.insert ( non_contracted_cols.begin ( ) , 0 );

            /* remember that the number of rows is one less that the number of columns */
            std::for_each ( non_contracted_rows.begin ( ) , non_contracted_rows.end ( ) , [ ] ( auto & e ) { e--; } );


            /* Init a new cost matrix with the non target edges untouched */
            auto[cs , pos_rows , pos_cols] = init_contracted_matrix ( no_edges , non_contracted_rows ,
                                                                      non_contracted_cols );
            auto const csp = cs.ptr ( );
            auto const & csr = cs.rows_;
            auto const & csc = cs.cols_;

            /* add the leaving arcs */
            auto v_out_correspondence = leaving_arcs ( cs , non_contracted_rows , edges , pos_rows );

            /* add the incoming arcs */
            auto v_hp_correspondence = incoming_arcs (
                    cs , non_contracted_rows , non_contracted_cols , edges , pos_rows , pos_cols );

            return std::make_tuple ( std::move ( cs ) ,
                                     std::move ( non_contracted_cols ) ,
                                     std::move ( v_hp_correspondence ) ,
                                     std::move ( v_out_correspondence ) ) ;
        }

        int leaving_arcs_highest_index (    int const & vhp,
                                            int const & node_outside_loop,
                                            std::vector < int > const & non_contracted_rows,
                                            std::vector < int > const & edges ) const {
            T * const p = this->ptr ( );
            auto const & c = this->cols_;
            int const & row = node_outside_loop ;
            T max = -1;
            int max_col = -1;
            for ( auto const & col : edges ) {
                if ( p[ row*c + col ] > max ) {
                    max = p[ row*c + col ];
                    max_col = col;
                }
            }
            return max_col;
        }

    private:
        /**
         * Copy the contents of the cost matrix that are not changed during contraction.
         * @param no_edges number of edges that are contracted
         * @param non_contracted_rows indexes of the nodes that won't be contracted(rows).
         * @param non_contracted_cols indexes of the nodes that won't be contracted(cols).
         * @return partially filled contracted cost matrix, correspondence of the old indices with the new ones.
         */
        [[nodiscard]] inline auto init_contracted_matrix ( int const & no_edges ,
                                                           std::vector < int > const & non_contracted_rows ,
                                                           std::vector < int > const & non_contracted_cols ) const {
            /* get stuff of the actual cost matrix */
            T const * const p = this->ptr ( );
            auto const & r = this->rows_;
            auto const & c = this->cols_;

            /* create a new matrix for the contracted cost matrix */
            cost < T > cs { r + 1 - no_edges , c + 1 - no_edges };
            T * const csp = cs.ptr ( );
            auto const & csc = cs.cols_;

            /* these vectors store the indexes of the old rows and columns in the new cost matrix */
            std::vector < int > pos_rows ( r , -1 );
            std::vector < int > pos_cols ( c , -1 );

            int i = 0 , j = 0;
            /* copy the matrix and fill the indexes matrices */
            for ( auto const & row : non_contracted_rows ) {
                pos_rows[ row ] = i;
                for ( auto const & col : non_contracted_cols ) {
                    csp[ i*csc + j ] = p[ row*c + col ];
                    ++j;
                }
                ++i;
                j = 0;
            }
            i = 0;
            for ( auto const & col : non_contracted_cols ) {
                pos_cols[ col ] = i;
                ++i;
            }
            return std::make_tuple ( std::move ( cs ) , std::move ( pos_rows ) , std::move ( pos_cols ) );
        }

        /**
         * Add the scores of the leaving arcs to the new node in the contracted matrix.
         * @param cs    contracted matrix to be filled.
         * @param non_contracted_rows indexes of the rows that won't be contracted.
         * @param edges edges of the loop.
         * @param pos_rows correspondence between indices of contracted cost matrix and current cost matrix.
         */
        inline auto leaving_arcs ( cost < T > & cs ,
                                   std::vector < int > const & non_contracted_rows ,
                                   std::vector < int > const & edges ,
                                   std::vector < int > const & pos_rows ) const {
            T const * const p = this->ptr ( );
            auto const & c = this->cols_;
            T * csp = cs.ptr ( );
            auto const & csc = cs.cols_;
            /* score which are the arcs with maximum score for each row */
            std::vector < int > v_out_correspondence ( c , -1 );
            for ( auto const & row : non_contracted_rows ) {
                T max = std::numeric_limits < T >::min () ;
                int max_row = -1;
                int max_col = -1;
                for ( auto const & col : edges ) {
                    if ( p[ row*c + col ] > max ) {
                        max = p[ row*c + col ];
                        max_col = col;
                        max_row = row;
                    }
                }
                v_out_correspondence[ row + 1 ] = max_col ;
                csp[ csc - 1 + pos_rows[ max_row ]*csc ] = max;
            }
            return v_out_correspondence;
        }

        /**
         * Add the scores of the incoming arcs to the new node
         * @param cs contracted matrix to be updated.
         * @param non_contracted_rows indexes of the rows that won't be contracted.
         * @param non_contracted_cols indexes of the cols that won't be contracted.
         * @param edges edges of the loop
         * @param pos_rows correspondence between row indices of contracted cost matrix and current cost matrix.
         * @param pos_cols correspondence between col indices of contracted cost matrix and current cost matrix.
         * @return
         */
        [[nodiscard]] inline auto incoming_arcs ( cost & cs ,
                                                  std::vector < int > const & non_contracted_rows ,
                                                  std::vector < int > const & non_contracted_cols ,
                                                  std::vector < int > const & edges ,
                                                  std::vector < int > const & pos_rows ,
                                                  std::vector < int > const & pos_cols ) const {

            T const * const p = this->ptr ( );
            auto const & c = this->cols_;
            T * csp = cs.ptr ( );
            auto const & csc = cs.cols_;
            auto const & csr = cs.rows_;

            /* saving some memory */
            auto[minscore , maxscore] = std::minmax_element ( edges.begin ( ) , edges.end ( ) );
            /* transition score of the previous node */
            std::vector < T > scores ( *maxscore - *minscore + 1 );
            /* total score of the loop */
            T score_loop = 0 ;

            /* calculate the total score of the loop, and fill the scores vector. */
            for ( int i = 0 , j = 1 ; i < edges.size ( ) - 1 ; ++i , ++j ) {
                int const & from = edges[ i ];
                int const & to = edges[ i + 1 ] - 1;
                T const & v = p[ from + to*c ];
                score_loop += v;
                scores[ edges[ j ] - *minscore ] = v;
            }
            /* compute the boundary case */
            T const & f = p[ edges.back ( ) + ( edges[ 0 ] - 1 )*c ];
            scores[ edges[ 0 ] - *minscore ] = f;
            score_loop += f;


            std::vector < int > v_hp_correspondences ( c , -1000 ) ;
            /* fill the values for all incoming arcs */
            for ( auto const & col : non_contracted_cols ) {
                T max = std::numeric_limits < T >::min () ;
                int max_row = -1;
                for ( int row : edges ) {
                    row--;
                    T const v = p[ row*c + col ] + score_loop - scores[ row - *minscore + 1 ] ;
                    if ( v > max ) {
                        max = v;
                        max_row = row + 1;
                    }
                }
                v_hp_correspondences[ col ] = max_row;
                csp[ ( csr - 1 )*csc + pos_cols[ col ] ] = max;
            }
            /* this is the solution for the solve_cycle step */
            return  v_hp_correspondences ;
        }

    };

    /**
     * The Chu, Liu, Edmonds parser.
     */
    class stc_parser {
    public:

        explicit stc_parser ( units::sentence const & s ) :
                s_ { s } { }

        /**
         * Parsing a sentence given the model m.
         * @param m model.
         * @return
         */
        template < typename T >
        units::sentence parse ( parsers::chu_liu_edmonds::model::model < T > & m ) {
            /* +1, because of root */
            cost < T > cm { s_.size ( ) , s_.size ( ) + 1 };
            m.eval ( s_ , cm ) ;
            adjacency msp = chu_liu_edmonds ( cm );
            return msp.to_stc ( s_ );
        }

        template < typename T >
        std::vector < int > heads ( parsers::chu_liu_edmonds::model::model < T > & m ) {
            /* +1, because of root */
            cost < T > cm { s_.size ( ) , s_.size ( ) + 1 };
            m.eval ( s_ , cm ) ;
            adjacency msp = chu_liu_edmonds ( cm );
//            std::cout << "ADJACENCY : " << msp ;
            return msp.to_heads () ;
        }

        /**
         * Solves a cycle in the adjacency matrix.
         * @param inital adjacency matrix to be updated.
         * @param v_hpp head node of v_d.
         * @param v_d child node of v_hpp to be disconnected asigned the new head v_hp.
         * @param v_hp The new head of v_d.
         */
        void solve_cycle ( adjacency & inital ,
                           adjacency const & y ,
                           std::vector < int > const & non_contracted_idx ,
                           std::vector < int > const & v_hp_correspondence ,
                           std::vector < int > const & v_out_correspondence ,
                           std::vector < int > const & edges ) {

            auto ip = inital.ptr ( );
            auto const & ips = inital.rows ( );
            auto sp = y.ptr ( );
            auto const & spc = y.cols ( );

            /* set all to 0  ( new matrix )*/
            std::fill ( ip, ip + ips*ips, 0 ) ;

            /* copy y */
            for ( int i = 0 ; i < spc - 1 ; ++i ) {
                for ( int j = 0 ; j < spc -1  ; ++j ) {
                    ip [ non_contracted_idx [ i ]*ips + non_contracted_idx [ j ] ] = sp [ i*spc + j ] ;
                }
            }

            /* find VD, VHP, VHPP */
            int FVHP = -1;
            for ( int i = 0 ; i < spc ; ++i ) {
                if ( sp[ i*spc + spc - 1 ] ) {
                    FVHP = i;
                    break;
                }
            }

            int const & VHP = non_contracted_idx [ FVHP ] ;
            int const & VD = v_hp_correspondence [ VHP ] ;

            /* Connect VHP to VD */
            ip[ VHP*ips + VD ] = true;


            /* Connect loop edges between them */
            for ( int i = 0 ; i < edges.size  ( ) - 1; ++i ) {
                if ( edges [ i + 1 ] != VD ) {
                    ip [ edges [ i ]*ips + edges [ i + 1 ] ] = true ;
                }
            }
            if ( edges.front () != VD ) {
                ip [ edges.back ()*ips + edges.front () ] = true ;
            }

            /* connect loop edges to outer edges */
            for ( int i = 0 ; i < spc ; ++i ) {
                if ( sp [ spc*(spc-1) + i ] ) {
                    int index = non_contracted_idx [ i ] ;
                    ip [ v_out_correspondence [ index ]*ips + index ] = true ;
                }
            }
        }

        /**
         * Iteratively find the maximum spanning tree by assigning best scores and removing cycles.
         * @param cm initial cost matrix
         * @return adjacency matrix -> MST
         */
        template < typename T >
        adjacency chu_liu_edmonds ( cost < T > const & cm ) {
                adjacency a = cm.adj ( );
                std::vector < int > loop = a.is_there_loop ( );
                if ( !loop.empty ( ) ) {
                    auto[s , ncc ,  v_hp_correspondence , v_out_correspondence ] = cm.contract (
                            loop );
                    adjacency an = chu_liu_edmonds ( s );
                    solve_cycle ( a , an  , ncc , v_hp_correspondence , v_out_correspondence , loop ) ;
                }
                return a;
            }


        units::sentence const & s_;
    };

    }


#endif //C___CHU_LIU_EDMONDS_H
