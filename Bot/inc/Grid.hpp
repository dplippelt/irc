/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Grid.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 12:41:29 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/26 14:43:37 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include <iostream>
#include <random>
#include "Battleship.hpp"
#include "enums.hpp"
#include "macros.hpp"

class Grid
{
	public:
		~Grid();
		Grid();
		Grid( const Grid& ) = delete;
		Grid& operator=( const Grid& ) = delete;

		std::vector<std::vector<char>>	getGrid() const;
		int								getSize() const;
		char							getEmptySymbol() const;
		char							getMissSymbol() const;
		char							getHitSymbol() const;
		void							updateGrid(int x, int y, char symbol);

		void	addShip( Battleship& ship );
		void	clearGrid();

		const std::string getGridMsg() const;

	private:
		static const inline int									k_max_attempts { 100 };
		static const inline char								k_empty { ' ' };
		static const inline char								k_miss { 'O' };
		static const inline char								k_hit { 'X' };
		static const inline std::vector<std::pair<int, int>>	k_dirVec { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

		int								m_size { 8 };
		std::vector<std::vector<char>>	m_grid {};

		bool	validShipPlacement( int x, int y, int shipSize, const std::pair<int, int>& dir ) const;
		bool	validShipCoordinate( int x, int y ) const;
		void	placeShip( Battleship& ship, int x, int y, const std::pair<int, int>& dir );
};
