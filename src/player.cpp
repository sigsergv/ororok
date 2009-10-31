/*
 * player.cpp
 *
 *  Created on: Nov 1, 2009
 *      Author: Sergei Stolyarov
 */

#include "player.h"

Player * Player::inst = 0;

struct Player::Private
{

};

Player::Player()
{
	p = new Private;
}

Player * Player::instance()
{
	if (Player::inst == 0) {
		Player::inst = new Player();
	}

	return Player::inst;
}
