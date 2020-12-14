/*
 * RayCastingWindow.cc
 *
 *  Created on: 24 окт. 2020 г.
 *      Author: unyuu
 */
#include <iostream>
#include <cmath>
#include "RayCastingWindow.h"
#include <cassert>

RayCastingWindow::RayCastingWindow(int width, int height)
: Window(width, height)
{
	_map = std::make_shared<Map>("map01.txt");
	_player = std::make_shared<Player>(_map);
	assert(_renderer != nullptr);
	_wall_texture = std::make_shared<Texture>(_renderer, "stena.jpg");
}

void RayCastingWindow::render()
{
	SDL_Rect r_sky { 0, 0, width(), height() / 2 };
	SDL_Rect r_floor { 0, height() / 2, width(), height() / 2 };
	SDL_SetRenderDrawColor(_renderer.get(), 64, 128, 192, 255);
	SDL_RenderFillRect(_renderer.get(), &r_sky);
	SDL_SetRenderDrawColor(_renderer.get(), 0, 128, 0, 255);
	SDL_RenderFillRect(_renderer.get(), &r_floor);

	SDL_SetRenderDrawBlendMode(_renderer.get(), SDL_BLENDMODE_BLEND);

	// Рисование стен (с использованием алгоритма бросания лучей)
	constexpr double H = 0.5;
	constexpr double EPS = 0.001;
	constexpr double FOV = Pi/3;
	double sd = width()/(2 * tan (FOV/2));
	double gam, bet;
	double dx, dy, rx, ry;
	double D, D_Horiz, D_Vert;
	double tx, tx_Horiz, tx_Vert;
	int h;
	for (int col = 0; col < width(); ++col) {
		// Здесь будет алгоритм
		gam = atan((col - width()/2) / sd);
			bet = _player->dir() + gam;
			//пересечение с горизонтальной линией
				if (sin(bet) > EPS)
				{
					dy = 1;
					ry = floor(_player->y()) + EPS;
					dx = 1 / tan(bet);
					rx = _player->x() - (_player->y() - ry) * dx;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					D_Horiz = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else if (sin(bet) < -EPS)
				{
					dy = -1;
					ry = ceil(_player->y()) - EPS;
					dx = 1 / tan(-bet);
					rx = _player->x() - (ry - _player->y()) * dx;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					D_Horiz = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else D_Horiz = INFINITY;
				tx_Horiz = rx - floor(rx);
				//пересечение с вертикальной линией
				if(cos(bet) > EPS){
					dx = 1;
					rx = floor(_player->x()) + EPS;
					dy = tan(bet);
					ry = _player->y() - (_player->x() - rx) * dy;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					D_Vert = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else if(cos(bet) < -EPS){
					dx = -1;
					rx = ceil(_player->x()) - EPS;
					dy = tan(-bet);
					ry = _player->y() - (rx - _player->x()) * dy;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					D_Vert = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else D_Vert = INFINITY;

				tx_Vert = ry - floor(ry);

				if (D_Horiz < D_Vert){
					D = D_Horiz;
					tx = tx_Horiz;
				}

				else{
					D = D_Vert;
					tx = tx_Vert;
				}
				h = int((sd * H)/D / cos(gam)) ;

				if (_textured) draw_textured_col(col, h, tx);
				else draw_col(col, h);
				SDL_SetRenderDrawColor(_renderer.get(), 0, 0, 0, 255);
				SDL_RenderDrawLine(_renderer.get(), _player->x(), _player->y(), rx, ry);
	}

	// Рисование карты

	SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 255, 64);
	for (int y = 0; y < _map->height(); ++y)
		for (int x = 0; x < _map->width(); ++x) {
			SDL_Rect r { x * 100, y * 100, 100, 100 };
			if (_map->wall(x, y))
				SDL_RenderFillRect(_renderer.get(), &r);
		}

	SDL_Rect r_player {
		int(_player->x() * 100)-10,
		int(_player->y() * 100)-10,
		20, 20
	};
	SDL_Rect r_player_eye {
		int(_player->x() * 100 + 20*cos(_player->dir()))-5,
		int(_player->y() * 100 + 20*sin(_player->dir()))-5,
		10, 10
	};

	SDL_SetRenderDrawColor(_renderer.get(), 255, 64, 64, 255);
	SDL_RenderFillRect(_renderer.get(), &r_player);
	SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 0, 255);
	SDL_RenderFillRect(_renderer.get(), &r_player_eye);
}

void RayCastingWindow::draw_col(int col, int h)
{
	SDL_SetRenderDrawColor(_renderer.get(), 64, 64, 64, 255);
	int y1 = height() / 2 - h / 2;
	int y2 = height() / 2 + h / 2;
	SDL_RenderDrawLine(_renderer.get(), col, y1, col, y2);

}

void RayCastingWindow::handle_keys(const Uint8 *keys)
{
	if (keys[SDL_SCANCODE_W]) _player->walk_forward();
	if (keys[SDL_SCANCODE_S]) _player->walk_backward();
	if (keys[SDL_SCANCODE_D]) _player->shift_right();
	if (keys[SDL_SCANCODE_A]) _player->shift_left();
	if (keys[SDL_SCANCODE_E]) _player->turn_right();
	if (keys[SDL_SCANCODE_Q]) _player->turn_left();
}

void RayCastingWindow::draw_textured_col(int col, int h, double tx)
{
	SDL_Rect what { int(floor(_wall_texture->width() * tx)),
		0, 1, _wall_texture->height()};
	SDL_Rect where { col, height()/2 - h/2, 1, h };
	_wall_texture->draw(&what, &where);
}
