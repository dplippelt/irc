/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Battleship.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 12:33:39 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/13 11:57:39 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <string_view>

class Battleship
{
	public:
		~Battleship();
		Battleship() = delete;
		Battleship( std::string_view shipName, int shipType, char shipSymbol, int shipSize );
		Battleship( const Battleship& );
		Battleship& operator=( const Battleship& );

		std::string											m_name {};
		int													m_type {};
		char												m_symbol {};
		int													m_size {};
		int													m_health {};
		std::pair<std::pair<int, int>, std::pair<int, int>>	m_location {};
};
