#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

#include "olcConsoleGameEngine.h"

class OneLoneCoder_Asteroids : public olcConsoleGameEngine
{
public :
	OneLoneCoder_Asteroids()
	{
		m_sAppName = L"Asteroids";
	}

private:
	struct sSpaceObject
	{
		int nSize;
		float x;
		float y;
		float dx;
		float dy;
		float angle;
	};

	vector<sSpaceObject> vecAsteroids;
	vector<sSpaceObject> vecBullets;
	sSpaceObject player;
	bool bDead = false;
	int nScore = 0;
	vector<pair<float, float >> vecModelShip;
	vector<pair<float, float >> vecModelAsteroid;

protected:

	virtual bool OnUserCreate()
	{
		vecModelShip =
		{
			{ 0.0f, -5.0f },
			{ -2.5f, +2.5f },
			{ +2.5f, +2.5f }
		}; //Isoceles triangle

		int verts = 20;
		for (int i = 0; i < verts; i++)
		{
			float noise = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
			float a = ( (float)i / (float)verts ) * 6.28318f;
			vecModelAsteroid.push_back(make_pair(noise * sinf( ( (float)i / (float)verts ) * 6.28318f),
												noise * cosf(((float)i / (float)verts ) * 6.28318f)));
		}

		ResetGame();
		return true;
	}

	void ResetGame()
	{
		vecAsteroids.clear();
		vecBullets.clear();

		vecAsteroids.push_back({ (int)16, 20.0f, 20.0f, 8.0f, -6.0f, 0.0f });
		vecAsteroids.push_back({ (int)16, 100.0f, 20.0f, -5.0f, 3.0f, 0.0f });

		//Initialize player
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		bDead = false;
		nScore = 0;
	}

	void WrapCoordinates(float ix, float iy, float& ox, float& oy)
	{
		ox = ix;
		oy = iy;
		if (ix < 0.0f) ox = ix + (float)ScreenWidth();
		if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();
		if (iy < 0.0f) oy = iy + (float)ScreenHeight();
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
	}

	virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}

	bool isPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
	}


	virtual bool OnUserUpdate(float fElapsedTime)
	{
		//Check if player is dead
		if (bDead)
			ResetGame();

		//Clear screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		//Steer player
		if (m_keys[VK_LEFT].bHeld)
			player.angle -= 5.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)
			player.angle += 5.0f * fElapsedTime;

		//Thrust player
		if (m_keys[VK_UP].bHeld)
		{
			//Acceleration
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}

		//Velocity changes position
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		//Keep ship within screen
		WrapCoordinates(player.x, player.y, player.x, player.y);


		//Check ship collision with asteroids
		for (auto& a : vecAsteroids)
			if (isPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
				bDead = true;


		//Fire bullets
		if (m_keys[VK_SPACE].bReleased)
			vecBullets.push_back({ 0, player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 100.0f });


		//Update and draw asteroids
		for (auto&a : vecAsteroids)
		{
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime;
			WrapCoordinates(a.x, a.y, a.x, a.y);

			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, (float)a.nSize, FG_YELLOW);
		}

		vector<sSpaceObject> newAsteroids;

		//Update and draw bullets
		for (auto& b : vecBullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			b.angle -= 1.0f * fElapsedTime;
			WrapCoordinates(b.x, b.y, b.x, b.y);

			//Check collision
			for (auto& a : vecAsteroids)
			{
				if (isPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					b.x = -100;

					if (a.nSize > 4)
					{
						//Create two child asteroids
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						newAsteroids.push_back({ (int)a.nSize >> 1, a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), 0.0f });
						newAsteroids.push_back({ (int)a.nSize >> 1, a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), 0.0f });
					}

					a.x = -100;
					nScore += 100;
				}
			}
		}

		//Appends new Asteroids to existing vector
		for (auto a : newAsteroids)
			vecAsteroids.push_back(a);

		//Remove Asteroid
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if (vecAsteroids.begin(), vecAsteroids.end(), [&](sSpaceObject o) { return (o.x < 0); });
			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		//If all asteroids are destroyed
		if (vecAsteroids.empty())
		{
			nScore += 1000;
			vecAsteroids.clear();
			vecBullets.clear();

			vecAsteroids.push_back({ (int)16,
										30.0f * sinf(player.angle - 3.14159f / 2.0f),
										30.0f * cosf(player.angle) - 3.14159f / 2.0f,
										10.0f * sinf(player.angle),
										10.0f * cosf(player.angle),
										 0.0f });

			vecAsteroids.push_back({ (int)16,
										30.0f * sinf(player.angle - 3.14159f / 2.0f),
										30.0f * cosf(player.angle) - 3.14159f / 2.0f,
										10.0f * sinf(-player.angle),
										10.0f * cosf(-player.angle),
										0.0f });
		}

		//Remove bullets
		if (vecBullets.size() > 0)
		{
			auto i = remove_if(vecBullets.begin(), vecBullets.end(),
				[&](sSpaceObject o) { return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() - 1 || o.y >= ScreenHeight() - 1); });
			if (i != vecBullets.end())
				vecBullets.erase(i);
		}

		//Draw bullets
		for (auto b : vecBullets)
			Draw(b.x, b.y);


		//Draw ship
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

		//Draw score
		DrawString(2, 2, L"SCORE: " + to_wstring(nScore));

		return true;
	}

	void DrawWireFrameModel(const vector < pair < float, float>>& vecModelCoordinates,
		float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE)

	{
		//pair.first = x coord
		//pair.second = y coord

		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		//Rotate
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Scale
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		//Translate
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		//Draw Closed Polygon
		for (int i = 0; i < verts + 1; i++)
		{
			int j = (i + 1);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}
	}

};

int main()
{
	OneLoneCoder_Asteroids game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();
	return 0;
}