/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Battleship.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 12:37:30 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/13 11:54:21 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Battleship.hpp"

/* ====================== Constructors & Destructors ====================== */

Battleship::~Battleship() = default;

Battleship::Battleship( std::string_view shipName, int shipType, char shipSymbol, int shipSize )
	: m_name {shipName}
	, m_type {shipType}
	, m_symbol {shipSymbol}
	, m_size {shipSize}
	, m_health {shipSize}
{
}

Battleship::Battleship( const Battleship& ) = default;

Battleship& Battleship::operator=( const Battleship& ) = default;
