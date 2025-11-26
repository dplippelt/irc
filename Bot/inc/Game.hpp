	/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 13:06:08 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/12 13:23:47 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <exception>
#include "Grid.hpp"
#include "Battleship.hpp"
#include "enums.hpp"

enum ShotResult
{
	MISS,
	HIT,
	SUNK,
	WON
};

class Game
{
	public:
		~Game();
		Game();
		Game( const Game& ) = delete;
		Game& operator=( const Game& ) = delete;

		void startGame();

		const Grid&			getGridObject() const;
		const Grid&			getPlayerGridObject() const;
		const std::string&	getSunkName() const;
		bool				validInput(const std::string& input) const;
		ShotResult			processShot(const std::string& input);

	private:
		typedef struct s_ship
		{
			std::string name {};
			int			type {};
			char		symbol {};
			int			size {};
		}				t_ship;

		static const inline int							k_max_ships { 5 };
		static const inline int							k_max_grid_attempts { 10 };
		static const inline std::vector<std::string>	k_ship_names { "CARRIER", "BATTLESHIP", "CRUISER", "SUBMARINE", "DESTROYER" };
		static const inline std::vector<char>			k_ship_symbols { 'K', 'B', 'C', 'S', 'D' };
		static const inline std::vector<int>			k_ship_sizes { 5, 4, 3, 3, 2 };
		static const inline std::vector<t_ship>			k_allShips {{k_ship_names[CARRIER], CARRIER, k_ship_symbols[CARRIER], k_ship_sizes[CARRIER]},
																	{k_ship_names[BATTLESHIP], BATTLESHIP, k_ship_symbols[BATTLESHIP], k_ship_sizes[BATTLESHIP]},
																	{k_ship_names[CRUISER], CRUISER, k_ship_symbols[CRUISER], k_ship_sizes[CRUISER]},
																	{k_ship_names[SUBMARINE], SUBMARINE, k_ship_symbols[SUBMARINE], k_ship_sizes[SUBMARINE]},
																	{k_ship_names[DESTROYER], DESTROYER, k_ship_symbols[DESTROYER], k_ship_sizes[DESTROYER]}};

		int							m_nShips { NSHIPS };
		Grid						m_grid {};
		Grid						m_player_grid {};
		std::vector<Battleship> 	m_game_ships {};
		std::string					m_sunk_name {};

		void	populateGrid();
		void	enemySunk(std::vector<Battleship>::iterator it);
};
