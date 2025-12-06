/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Grid.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 12:48:32 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/06 12:31:19 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Grid.hpp"

/* ====================== Constructors & Destructors ====================== */

Grid::~Grid() = default;

Grid::Grid()
{
	m_grid.resize(m_size);

	for ( int y {0}; y < m_size; ++y )
	{
		m_grid[y].resize(m_size);
		for ( int x {0}; x < m_size; ++x )
			m_grid[y][x] = k_empty;
	}
}





/* ====================== Getters & Setters ====================== */

const std::vector<std::vector<char>>& Grid::getGrid() const
{
	return (m_grid);
}

int	Grid::getSize() const
{
	return (m_size);
}

char	Grid::getEmptySymbol() const
{
	return (k_empty);
}

char	Grid::getMissSymbol() const
{
	return (k_miss);
}

char	Grid::getHitSymbol() const
{
	return (k_hit);
}

void	Grid::updateGrid(int x, int y, char symbol)
{
	m_grid[y][x] = symbol;
}





/* ====================== Class Behavior ====================== */

const std::string Grid::getGridMsg() const
{
	std::string	msg {""};
	char		rowLabel {'A'};

	for ( int y {0}; y < m_size; ++y )
	{
		for ( int x {0}; x < m_size; ++x )
		{
			if (m_grid[y][x] == k_hit)
				msg += COLOR RED COLOR_BG BLUE;
			else if (m_grid[y][x] == k_miss || m_grid[y][x] == k_empty)
				msg += COLOR LIGHT_GREY COLOR_BG BLUE;
			else
				msg += COLOR ORANGE COLOR_BG BLUE;
			msg += m_grid[y][x];
			msg += RESET;
		}
		msg += BOLD COLOR YELLOW;
		msg += rowLabel;
		msg += RESET;
		msg += "\n";
		rowLabel++;
	}
	msg += BOLD COLOR YELLOW;
	msg += "12345678";
	msg += RESET;
	msg += "\n";

	return msg;
}

void	Grid::addShip( Battleship& ship )
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> coord_dist(0, 8);
	static std::uniform_int_distribution<> dir_dist(0, 3);

	for ( int attempt {0}; attempt < k_max_attempts; ++attempt )
	{
		int	startX { coord_dist(gen) };
		int	startY { coord_dist(gen) };
		int	dir { dir_dist(gen) };

		if (!validShipPlacement(startX, startY, ship.m_size, k_dirVec[dir]))
			continue;

		placeShip(ship, startX, startY, k_dirVec[dir]);
		return;
	}
}

void	Grid::placeShip(Battleship& ship, int x, int y, const std::pair<int, int>& dir)
{
	for ( int i {0}; i < ship.m_size; ++i )
	{
		m_grid[y][x] = ship.m_symbol;
		ship.m_location.push_back({y, x});

		x += dir.first;
		y += dir.second;
	}
}

bool	Grid::validShipPlacement(int x, int y, int shipSize, const std::pair<int, int>& dir) const
{
	for ( int i {0}; i < shipSize; ++i )
	{
		if (!validShipCoordinate(x, y))
			return false;
		x += dir.first;
		y += dir.second;
	}
	return true;
}

bool	Grid::validShipCoordinate(int x, int y) const
{
	if (x < 0)
		return false;
	if (y < 0)
		return false;
	if (x >= m_size)
		return false;
	if (y >= m_size)
		return false;
	if (m_grid[y][x] != k_empty)
		return false;
	return true;
}

void	Grid::clearGrid()
{
	for ( int y {}; y < m_size; ++y )
		for ( int x {}; x < m_size; ++x )
			m_grid[y][x] = k_empty;
}
