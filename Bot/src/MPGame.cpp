/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MPGame.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 13:35:08 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/29 09:29:39 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MPGame.hpp"

/* ====================== Constructors & Destructors ====================== */

MPGame::~MPGame() = default;

MPGame::MPGame( const std::string& player_one_name, const std::string& player_two_name )
	: m_curr_player { player_two_name }
	, m_player_names { player_one_name, player_two_name }
{
	t_player_data player_one_data { &m_player_two_grid, &m_player_one_shots_grid, &m_player_two_game_ships, &m_player_two_nShips };
	t_player_data player_two_data { &m_player_one_grid, &m_player_two_shots_grid, &m_player_one_game_ships, &m_player_one_nShips };

	m_players.insert({player_one_name, player_one_data});
	m_players.insert({player_two_name, player_two_data});

	populateGrid( m_player_one_grid, m_player_one_game_ships );
	populateGrid( m_player_two_grid, m_player_two_game_ships );
}





/* ====================== Public Interface ====================== */

bool	MPGame::validInput( const std::string& input, const std::string& playerName ) const
{
	if ( input.length() != 2 )
		return false;

	int	x { input[1] - '0' - 1 };
	int	y { std::toupper(input[0]) - 'A' };

	auto player_it { m_players.find(playerName) };
	if (player_it == m_players.end())
		throw std::runtime_error("Invalid player name: '" + playerName + "'");

	const Grid* player_grid { player_it->second.player_grid };

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

ShotResult	MPGame::processShot( const std::string& input, const std::string& playerName )
{
	bool		sunk {};
	int			x { input[1] - '0' - 1 };
	int			y { std::toupper(input[0]) - 'A' };

	t_player_data	data { getPlayerData(playerName) };

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
				enemySunk(it, playerName);
				sunk = true;
				break;
			}
			break;
		}
	}

	switchTurns();

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

const std::string&	MPGame::getCurrentPlayer() const
{
	return m_curr_player;
}

std::map<std::string, MPGame::t_player_data>&	MPGame::getPlayerList()
{
	return m_players;
}

MPGame::t_player_data	MPGame::getPlayerData( const std::string& playerName )
{
	auto player_it { m_players.find(playerName) };
	if (player_it == m_players.end())
		throw std::runtime_error("Invalid player name: '" + playerName + "'");

	return player_it->second;
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

void	MPGame::enemySunk( std::vector<Battleship>::iterator it, const std::string& playerName )
{
	t_player_data	data { getPlayerData(playerName) };

	int	x {};
	int	y {};

	for ( auto itt { it->m_location.begin() }; itt != it->m_location.end(); ++itt )
	{
		x = itt->second;
		y = itt->first;
		data.player_grid->updateGrid(x, y, it->m_symbol);
	}

	m_sunk_name = it->m_name;
	(*data.opponent_nships)--;
	data.opponent_ships->erase(it);
}

void	MPGame::switchTurns()
{
	m_curr_player = (m_player_names[0] == m_curr_player) ? m_player_names[1] : m_player_names[0];
}
