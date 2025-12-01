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
#include <map>
#include <exception>
#include "Grid.hpp"
#include "Battleship.hpp"
#include "enums.hpp"

class MPGame
{
	public:
		~MPGame();
		MPGame() = delete;
		MPGame( const std::string& player_one_name, const std::string& player_two_name );
		MPGame( const MPGame& ) = delete;
		MPGame& operator=( const MPGame& ) = delete;

		typedef struct s_player_data
		{
			const Grid*					opponent_grid {};
			Grid*						player_grid {};
			std::vector<Battleship>*	opponent_ships {};
			int*						opponent_nships {};
		}								t_player_data;

		void startGame();

		const Grid&								getPlayerOneGridObject() const;
		const Grid&								getPlayerTwoGridObject() const;
		const Grid&								getPlayerOneShotsGridObject() const;
		const Grid&								getPlayerTwoShotsGridObject() const;
		std::map<std::string, t_player_data>&	getPlayerList();
		const std::string&						getCurrentPlayer() const;
		const std::string&						getSunkName() const;

		bool				validInput( const std::string& input, const std::string& playerName ) const;
		ShotResult			processShot( const std::string& input, const std::string& playerName );

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

		int										m_player_one_nShips { NSHIPS };
		int										m_player_two_nShips { NSHIPS };
		Grid									m_player_two_grid {};
		Grid									m_player_one_grid {};
		Grid									m_player_two_shots_grid {};
		Grid									m_player_one_shots_grid {};
		std::vector<Battleship> 				m_player_one_game_ships {};
		std::vector<Battleship>				 	m_player_two_game_ships {};
		std::string								m_sunk_name {};
		std::map<std::string, t_player_data>	m_players {};
		std::string								m_curr_player {};
		std::vector<std::string>				m_player_names {};

		void	populateGrid( Grid& grid, std::vector<Battleship>& game_ships );
		void	enemySunk( std::vector<Battleship>::iterator it, const std::string& playerName );
		void	switchTurns();

		t_player_data	getPlayerData( const std::string& playerName );
};
