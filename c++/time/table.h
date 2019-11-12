/* Table data structure
    Authors: Pedro G. Bascoy, F. Arguello, Alberto Suarez Garea, and Dora B. Heras,
   © 2019 Universidade de Santiago de Compostela. All rights reserved.
*/
#ifndef MODERN_TABLE_H
#define MODERN_TABLE_H

#include <algorithm>
#include <vector>
#include <iomanip>
#include <tuple>
#include <iostream>
#include <fstream>


namespace ordered {


	/**
	* @author Pedro G. Bascoy
	* @date 2018
	*
	* @brief A table for stacking information of execution time, memory usage, etc.
	*		Provided with a csv writer and a naïve join function.
	* @warning C++14 required.
	*
	* @types Ts tuple of types.
	*/
template <typename ... Ts >
class table {
public:
	explicit table() : rows(), tuple_size ( sizeof...(Ts) ) { }
	explicit table ( std::vector < std::tuple < Ts ... > > && v ) : rows ( std::move(v) ) , tuple_size ( sizeof...(Ts))  { }

	/**
	 * Add a row into the table.
	 * @param vals Values corresponding to the colums of the row to be inserted.
	 */
	inline void new_register ( Ts const & ... vals ) {
		rows.push_back( std::make_tuple ( vals ...  ) ) ;
	}
	inline void new_register ( Ts && ... vals ) {
		rows.push_back( std::make_tuple ( std::forward (vals) ... ) ) ;
	}

	inline void update_regiser ( size_t const i , Ts && ... vals ) {
        rows[ i ] = std::make_tuple ( std::forward ( vals )... ) ;
	}

	/**
	 * Export the current table to a CSV file using ',' as column separator
	 * @param fn File path in which the csv will be saved.
	 */
	void to_csv ( std::string const & fn ) const {
        std::ofstream file ( fn ) ;
//        if ( !file.is_open ( ) ) { throw excpt::io ( fn ); }
		std::for_each( rows.begin() , rows.end(), [&file,this] ( std::tuple< Ts ... > const & t ) {
			auto printer = [&file, this] (auto && ... at) {
				this->printline ( file, at ... ) ;
			} ;
			apply ( printer, t ) ;
		} ) ;
		std::cout << "[INFO]: Saved " << fn << " file" << std::endl ;
	}
	/**
	 * Join columns with the same value (using operator==) applying a user-defined operation.
	 * @templateparam 	col_join 	Column index used to join. Same row values within col_join column will join.
	 * @templateparam	col_operation Column where the callable function will be applied.
	 * @param 			callable 	Function applied to the joined rows.
	 * @param 			iv       	Value used for join the first row.
	 */
	template < size_t col_join = 0, size_t col_operation = 1, typename F, typename Iv >
	table< Ts ... >  join( F && callable, Iv const & iv ) const {

		std::vector < std::tuple < Ts ... > > join {} ;
		std::for_each ( rows.begin(), rows.end(), [&iv, &join, &callable, this] ( std::tuple < Ts ... > const & t ) {

			auto pos = _is_in_join< col_join >( std::get< col_join > ( t ), join ) ;
			if ( pos == join.end() ) { /* create a new register */
		 		std::tuple < Ts ... > first (t) ;
				std::get < col_operation > ( first ) = std::forward < F > ( callable ) ( iv , std::get < col_operation > ( first ) ) ;
				join.push_back ( std::move(first) ) ;
			} else { /* otherwise apply operation callable */
				std::tuple < Ts ... >  & upgradable = *pos ;
				auto const & val1 = std::get< col_operation > ( upgradable )	;
				auto const & val2 = std::get< col_operation > ( t )			;
				std::get< col_operation > ( upgradable ) = std::forward < F > ( callable )( val1 , val2  ) ;
			}

		} ) ;

		return table( std::move(join) ) ;

	}
	/**
	 * Delete rows according to a comparator
	 * @templateparam	col_drop	Selects the column target to be compared
	 * @param comparator 			Function comparator
	 */
	template < size_t col_drop, typename F >
	void drop_row_if ( F && comparator ) {
		/* std::for_each is not safe if elements are erased while iterating */
		auto it = rows.begin() ;
		while ( it != rows.end() ) {
			std::tuple < Ts ... > const & t = *it ;
			if ( comparator ( std::get < col_drop > ( t ) ) ) {
				rows.erase ( it ) ;
			}
			it ++ ;
		}
	}

	/**
	 * Join the rows specified by the user by creating a new row (added as a new row at the bottom).
	 * @templateparam col_operation Column target to the function operation
	 * @param operation 			Function called when joining rows
	 * @param iterable  			Iterable containing the rows indexes to be joined
	 * @param iv        			Initial value. Value used for join the first row.
	 * @param vals      			Values of the new row that will be added.
	 */
	template < size_t col_operation, typename F, typename It, typename Iv >
	void join_rows ( F && operation, It const & iterable, Iv iv,  Ts && ... vals ) {
		Iv & upgradable = iv ;
		std::for_each ( iterable.begin(), iterable.end(), [&upgradable, &operation, this] ( auto && e ) mutable {
			upgradable =  std::forward < F > ( operation ) ( upgradable , std::get < col_operation > ( rows[e] ) ) ;
		} ) ;
		auto upgradable_tuple = std::make_tuple ( std::move (vals) ... ) ;
		std::get < col_operation > ( upgradable_tuple ) = upgradable ;
		rows.push_back ( std::move ( upgradable_tuple ) ) ;
	}


private:
	std::vector < std::tuple < Ts... > > 	rows ;
	size_t const 							tuple_size ;


	/* apply a function to the tuple elements (IT SHOULD RETURN decltype(auto) for non-void lambdas) */
	template < typename F, typename Tuple, size_t ... it >
	constexpr decltype(auto) apply_q (F&& f, Tuple && t, std::index_sequence < it ... > ) const {
		return std::forward < F > ( f ) (std::get < it > ( std::forward < Tuple > ( t )) ... ) ;
	}
	/* (IT SHOULD RETURN decltype(auto)) for non-void lambdas */
	template < typename F, typename Tuple >
	constexpr decltype(auto) apply (F&& f, Tuple && t) const {
		using indices = std::make_index_sequence < std::tuple_size < std::decay_t < Tuple > >::value > ;
		return apply_q ( std::forward< F > ( f ), std::forward < Tuple > ( t ), indices { } ) ;
	}

	template < size_t col, typename T >
	constexpr auto _is_in_join (T const & e, std::vector < std::tuple < Ts ... > >  & j) const {
		return std::find_if ( j.begin(), j.end(), [&e] ( std::tuple < Ts ... > const & tt ) {
			return std::get< col > ( tt ) == e ; }
		) ;
	}
	inline void printline(std::ofstream & of) const { of << '\n' ; }
	template < typename U, typename ...  Us >
	inline void printline(std::ofstream & of, U const & e, Us const & ... elms ) const {
		if ( std::is_same< U, double >::value ) {
	 		of << std::fixed << std::setprecision( 6 ) << e << ", " ;
		} else {
			of << e << ", ";
		}
		printline(of, elms ... ) ;
	}



} ;

}

#endif
