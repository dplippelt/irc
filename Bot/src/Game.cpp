/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 13:35:08 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/26 15:02:42 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game.hpp"

/* ====================== Constructors & Destructors ====================== */

Game::~Game() = default;

Game::Game()
{
	populateGrid();
}





/* ====================== Public Interface ====================== */

bool	Game::validInput(const std::string& input) const
{
	if ( input.length() != 2 )
		return false;

	int	x { input[1] - '0' - 1 };
	int	y { std::toupper(input[0]) - 'A' };

	if ( x < 0 )
		return false;
	if ( y < 0 )
		return false;
	if ( x >= m_grid.getSize() )
		return false;
	if ( y >= m_grid.getSize() )
		return false;
	if ( m_player_grid.getGrid()[y][x] != m_player_grid.getEmptySymbol() )
		return false;
	return true;
}

ShotResult	Game::processShot(const std::string& input)
{
	bool		sunk {};
	int			x { input[1] - '0' - 1 };
	int			y { std::toupper(input[0]) - 'A' };

	if ( m_grid.getGrid()[y][x] == m_grid.getEmptySymbol() )
	{
		m_player_grid.updateGrid(x, y, m_player_grid.getMissSymbol());
		return ShotResult::MISS;
	}

	m_player_grid.updateGrid(x, y, m_player_grid.getHitSymbol());

	for( auto it { m_game_ships.begin() }; it != m_game_ships.end(); ++it )
	{
		if ( it->m_symbol == m_grid.getGrid()[y][x] )
		{
			it->m_health--;
			if (it->m_health == 0)
			{
				enemySunk(it);
				sunk = true;
				break;
			}
			break;
		}
	}

	if (!m_nShips)
		return ShotResult::WON;
	if (sunk)
		return ShotResult::SUNK;
	return ShotResult::HIT;
}





/* ====================== Getters ====================== */


const Grid&	Game::getGridObject() const
{
	return m_grid;
}

const Grid&	Game::getPlayerGridObject() const
{
	return m_player_grid;
}

const std::string&	Game::getSunkName() const
{
	return m_sunk_name;
}





/* ====================== Class Behavior ====================== */

void	Game::populateGrid()
{
	for ( int i {0}; i < m_nShips; ++i )
	{
		Battleship ship {	k_allShips[i].name,
							k_allShips[i].type,
							k_allShips[i].symbol,
							k_allShips[i].size };

		m_grid.addShip(ship);
		m_game_ships.push_back(ship);
	}
}

void	Game::enemySunk(std::vector<Battleship>::iterator it)
{
	std::pair<int, int>	startCoord { it->m_location.first };
	std::pair<int, int>	endCoord { it->m_location.second };

	if ( startCoord.first > endCoord.first )
	{
		int	temp { startCoord.first };
		startCoord.first = endCoord.first;
		endCoord.first = temp;
	}
	if ( startCoord.second > endCoord.second )
	{
		int	temp { startCoord.second };
		startCoord.second = endCoord.second;
		endCoord.second = temp;
	}

	for ( int y { startCoord.first }; y <= endCoord.first; ++y )
		for ( int x { startCoord.second }; x <= endCoord.second; ++x )
			m_player_grid.updateGrid(x, y, it->m_symbol);

	m_sunk_name = it->m_name;
	m_nShips--;
	m_game_ships.erase(it);
}
