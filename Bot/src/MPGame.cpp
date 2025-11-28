/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MPGame.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 13:35:08 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/28 12:48:39 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MPGame.hpp"

/* ====================== Constructors & Destructors ====================== */

MPGame::~MPGame() = default;

MPGame::MPGame( const std::string& player_one_name, const std::string& player_two_name )
{
	m_players.insert({player_one_name, 1});
	m_players.insert({player_two_name, 2});

	populateGrid( m_player_one_grid, m_player_one_game_ships );
	populateGrid( m_player_two_grid, m_player_two_game_ships );
}





/* ====================== Public Interface ====================== */

bool	MPGame::validInput( const std::string& input, int curr_player ) const
{
	if ( input.length() != 2 )
		return false;

	int	x { input[1] - '0' - 1 };
	int	y { std::toupper(input[0]) - 'A' };

	const Grid* player_grid {};

	switch (curr_player)
	{
	case 1:
		player_grid = &m_player_one_shots_grid;
		break;
	case 2:
		player_grid = &m_player_two_shots_grid;
		break;
	default:
		throw std::runtime_error("Invalid player number: '" + std::to_string(curr_player) + "'");
	}

	if ( x < 0 )
		return false;
	if ( y < 0 )
		return false;
	if ( x >= player_grid->getSize() )
		return false;
	if ( y >= player_grid->getSize() )
		return false;
	if ( player_grid->getGrid()[y][x] != player_grid->getEmptySymbol() )
		return false;
	return true;
}

ShotResult	MPGame::processShot( const std::string& input, int curr_player )
{
	bool		sunk {};
	int			x { input[1] - '0' - 1 };
	int			y { std::toupper(input[0]) - 'A' };

	t_player_data	data { getPlayerData(curr_player) };

	if ( data.opponent_grid->getGrid()[y][x] == data.opponent_grid->getEmptySymbol() )
	{
		data.player_grid->updateGrid(x, y, data.player_grid->getMissSymbol());
		return ShotResult::MISS;
	}

	data.player_grid->updateGrid(x, y, data.player_grid->getHitSymbol());

	for( auto it { data.opponent_ships->begin() }; it != data.opponent_ships->end(); ++it )
	{
		if ( it->m_symbol == data.opponent_grid->getGrid()[y][x] )
		{
			it->m_health--;
			if (it->m_health == 0)
			{
				enemySunk(it, curr_player);
				sunk = true;
				break;
			}
			break;
		}
	}

	if (!(*data.opponent_nships))
		return ShotResult::WON;
	if (sunk)
		return ShotResult::SUNK;
	return ShotResult::HIT;
}





/* ====================== Getters ====================== */


const Grid&	MPGame::getPlayerOneGridObject() const
{
	return m_player_one_grid;
}

const Grid&	MPGame::getPlayerTwoGridObject() const
{
	return m_player_two_grid;
}

const Grid&	MPGame::getPlayerOneShotsGridObject() const
{
	return m_player_one_shots_grid;
}

const Grid&	MPGame::getPlayerTwoShotsGridObject() const
{
	return m_player_two_shots_grid;
}

const std::string&	MPGame::getSunkName() const
{
	return m_sunk_name;
}

const std::map<std::string, int>&	MPGame::getPlayerList() const
{
	return m_players;
}

MPGame::t_player_data	MPGame::getPlayerData( int curr_player )
{
	t_player_data data {};

	switch (curr_player)
	{
	case 1:
		data.opponent_grid = &m_player_two_grid;
		data.player_grid = &m_player_one_shots_grid;
		data.opponent_ships = &m_player_two_game_ships;
		data.opponent_nships = &m_player_two_nShips;
		break;
	case 2:
		data.opponent_grid = &m_player_one_grid;
		data.player_grid = &m_player_two_shots_grid;
		data.opponent_ships = &m_player_one_game_ships;
		data.opponent_nships = &m_player_one_nShips;
		break;
	default:
		throw std::runtime_error("Invalid player number: '" + std::to_string(curr_player) + "'");
	}

	return data;
}





/* ====================== Class Behavior ====================== */

void	MPGame::populateGrid( Grid& grid, std::vector<Battleship>& game_ships )
{
	for ( int i {0}; i < NSHIPS; ++i )
	{
		Battleship ship {	k_allShips[i].name,
							k_allShips[i].type,
							k_allShips[i].symbol,
							k_allShips[i].size };

		grid.addShip(ship);
		game_ships.push_back(ship);
	}
}

void	MPGame::enemySunk( std::vector<Battleship>::iterator it, int curr_player )
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

	t_player_data	data { getPlayerData(curr_player) };

	for ( int y { startCoord.first }; y <= endCoord.first; ++y )
		for ( int x { startCoord.second }; x <= endCoord.second; ++x )
			data.player_grid->updateGrid(x, y, it->m_symbol);

	m_sunk_name = it->m_name;
	(*data.opponent_nships)--;
	data.opponent_ships->erase(it);
}
