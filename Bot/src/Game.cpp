/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 13:35:08 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/20 14:42:49 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game.hpp"

/* ====================== Constructors & Destructors ====================== */

Game::~Game() = default;

Game::Game()
{
	startGame();
}

Game::Game( int nShips )
	: m_nShips { nShips }
{
	if ( m_nShips > k_max_ships )
		throw std::runtime_error("Error: cannot start game with more than " + std::to_string(k_max_ships) + " ships on the grid");

	startGame();
}





/* ====================== Public Interface ====================== */

void	Game::startGame()
{
	// std::string	input {};

	populateGrid();

	// #ifdef DEBUG
	// // m_grid.printGrid();
	// #endif

	// m_player_grid.printGrid();

	// while (true)
	// {
	// 	std::cout << "Enter coordinates to shoot at (e.g. B3): ";
	// 	std::cin >> input;
	// 	if ( std::cin.eof() )
	// 		return;
	// 	if ( std::cin.fail() || !validInput(input) )
	// 	{
	// 		std::cin.clear();
	// 		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	// 		continue;
	// 	}

	// 	processShot(input);
	// 	m_player_grid.printGrid();

	// 	if (!m_nShips)
	// 	{
	// 		std::cout << "Congrats, you sunk all of the enemy's ships!" << std::endl;
	// 		return;
	// 	}
	// }
}





/* ====================== Getters ====================== */


const Grid& Game::getGridObject() const
{
	return m_grid;
}

const Grid& Game::getPlayerGridObject() const
{
	return m_player_grid;
}

/* ====================== Class Behavior ====================== */

void	Game::populateGrid()
{
	bool	success {};

	for ( int grid_attempt {0}; grid_attempt < k_max_grid_attempts; ++grid_attempt )
	{
		success = true;

		for ( int i {0}; i < m_nShips; ++i )
		{
			Battleship ship {	k_allShips[i].name,
								k_allShips[i].type,
								k_allShips[i].symbol,
								k_allShips[i].size };

			try
			{
				m_grid.addShip(ship);
				m_game_ships.push_back(ship);
			}
			catch ( const std::exception& e )
			{
				std::cout << e.what() << std::endl;
				if (grid_attempt == k_max_grid_attempts - 1)
					throw std::runtime_error("Error: unable to populate the grid with " + std::to_string(m_nShips) + " ships. Please try again with a larger grid or less ships.");

				std::cout << "Trying again with a new empty grid..." << std::endl;
				m_grid.clearGrid();
				m_game_ships.clear();
				success = false;
				break;
			}
		}

		if ( success )
			break;
	}
}

void	Game::processShot(const std::string& input)
{
	int	x { input[1] - '0' - 1 };
	int	y { std::toupper(input[0]) - 'A' };

	if ( m_grid.getGrid()[y][x] == m_grid.getEmptySymbol() )
	{
		std::cout << "\nYou miss!\n" << std::endl;
		m_player_grid.updateGrid(x, y, m_player_grid.getMissSymbol());
		return;
	}

	std::cout << "\nYou hit an enemy ship!\n" << std::endl;
	m_player_grid.updateGrid(x, y, m_player_grid.getHitSymbol());

	for( auto it { m_game_ships.begin() }; it != m_game_ships.end(); ++it )
	{
		if ( it->m_symbol == m_grid.getGrid()[y][x] )
		{
			it->m_health--;
			if (it->m_health == 0)
			{
				enemySunk(it);
				break;
			}
			break;
		}
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

	std::cout << "You sunk the enemy's " + it->m_name + "! Congrats, keep going!\n" << std::endl;
	m_nShips--;
	m_game_ships.erase(it);
}

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
