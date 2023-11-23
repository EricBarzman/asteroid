/*
* 
* Le code ci-dessous a �t� r�alis� sous la tutelle de One_Loner_Coder. Il utilise
* d'ailleurs le moteur graphique et de jeu PixelGameEngine, sorte d'outil pour cr�er
* un jeu vid�o peu complexe et ex�cuter des fonctions draw().
* 
* Il ne s'agit donc pas tant d'un projet personnel qu'un preuve d'un travail
* d'apprentissage en C++.
* 
* Le but �tait de recr�er le classique "Ast�roid", un jeu utilisant fortement des notions
* de trigonom�trie, de calculs d'angles, les fonctions sinus et cosinus.
* 
* Eric Barzman
* 
*/


#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

//Le fameux PixelGameEngine
#include "olcPixelGameEngine.h"

//Notre Asteroid va �tre contenu dans une classe qui h�rite du PixelGameEngine
class Asteroids : public olc::PixelGameEngine
{
public:
	Asteroids()
	{
		sAppName = "Asteroids";
	}

private:
	
	//Objets de l'espace
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
	vector<sSpaceObject> vecLasers;
	sSpaceObject player;
	bool bDead = false;
	int nScore = 0;

	vector<pair<float, float>> vecModelShip;
	vector<pair<float, float>> vecModelAsteroid;

protected:
	// Une fonction appel� par le PixelGameEngine
	virtual bool OnUserCreate()
	{
		vecModelShip = 
		{	//Un triangle isoc�le
			{ 0.0f, -5.0f},
			{-2.5f, +2.5f},
			{+2.5f, +2.5f}
		};

		// Les vertices permettent de donner un aspect imparfait et ruggueux � l'ast�roide,
		// l'�loignant de sa forme circulaire de base.
		// Le syst�me de d�tection de collision le consid�rera n�anmoins comme un circle
		int verts = 20;
		for (int i = 0; i < verts; i++)
		{
			float noise = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
			vecModelAsteroid.push_back(make_pair(noise * sinf(((float)i / (float)verts) * 6.28318f), 
												 noise * cosf(((float)i / (float)verts) * 6.28318f)));
		}

		ResetGame();
		return true;
	}

	void ResetGame()
	{
		// R�initialise la position du joueur
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		vecLasers.clear();
		vecAsteroids.clear();

		// On cr�e deux ast�roides
		vecAsteroids.push_back({ (int)16, 20.0f, 20.0f, 8.0f, -6.0f, 0.0f });
		vecAsteroids.push_back({ (int)16, 100.0f, 20.0f, -5.0f, 3.0f, 0.0f });

		bDead = false;
		nScore = false;
	}

	// Ici, on d�finit l'�cran comme un tore, c-�-d que ce qui sort d'un c�t� r�appara�t de l'autre
	void WrapCoordinates(float ix, float iy, float &ox, float &oy)
	{
		ox = ix;
		oy = iy;
		if (ix < 0.0f)	ox = ix + (float)ScreenWidth();
		if (ix >= (float)ScreenWidth())	ox = ix - (float)ScreenWidth();
		if (iy < 0.0f)	oy = iy + (float)ScreenHeight();
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
	}

	virtual void Draw(int x, int y)
	{
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);		
		olc::PixelGameEngine::Draw(fx, fy, olc::WHITE);
	}

	//Utile pour la d�tection des collisions
	bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x-cx)*(x-cx) + (y-cy)*(y-cy)) < radius;
	}

	// Fonction appel�e par PixelGameEngine
	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (bDead)
			ResetGame();

		// Vider l'�cran
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

		// Piloter le vaisseau
		if (GetKey(olc::Key::LEFT).bHeld)
			player.angle -= 5.0f * fElapsedTime;
		if (GetKey(olc::Key::RIGHT).bHeld)
			player.angle += 5.0f * fElapsedTime;

		// Acc�l�ration en avant
		if (GetKey(olc::Key::UP).bHeld)
		{
			// L'acc�l�ration (d�pendant de l'angle) affecte la vitesse, en fonction du temps
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}

		// La vitesse, � son tour, affecte la position
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		// Le joueur se retrouve de l'autre c�t� de l'�cran, effet Tore
		WrapCoordinates(player.x, player.y, player.x, player.y);

		// D�tection de collisions avec les ast�roides
		for (auto &a : vecAsteroids)
			if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
				bDead = true; // A�e a�e a�e

		// Tire des coups de laser vers l'avant
		if (GetKey(olc::Key::SPACE).bReleased)
			vecLasers.push_back({ 0, player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 100.0f });

		// Update et draw des asteroids
		for (auto &a : vecAsteroids)
		{
			// La vitesse affecte la position
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime; // Rotation de l'ast�roide

			// L'asteroide r�appara�t de l'autre c�t� de l'�cran
			WrapCoordinates(a.x, a.y, a.x, a.y);

			// Afficher l'ast�roide
			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, (float)a.nSize);
		}

		// Un nouvel ast�roide cr�� apr�s collision est d'abord conserv� dans un vector temporaire
		// Il ne sera pas compter par la boucle for tout de suite
		vector<sSpaceObject> newAsteroids;

		// Tirs de laser
		for (auto &b : vecLasers)
		{
			// Position mise � jour, l'�cran est un tore etc
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			WrapCoordinates(b.x, b.y, b.x, b.y);
			b.angle -= 1.0f * fElapsedTime;

			// Collision avec les asteroides
			for (auto &a : vecAsteroids)
			{
				if(IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					// Un ast�roide a �t� touch�. Le laser est �limin�.
					// Pour cela, on le place HORS de l'�cran. Il sera enlev� par l'algorithme de nettoyage
					b.x = -100;

					// Cr�ation des ast�roides plus petits
					if (a.nSize > 4)
					{
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						newAsteroids.push_back({ (int)a.nSize >> 1 ,a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), 0.0f });
						newAsteroids.push_back({ (int)a.nSize >> 1 ,a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), 0.0f });
					}

					// On retire l'ast�roide, de la m�me fa�on que le laser
					a.x = -100;
					nScore += 100;
				}
			}
		}

		// On ajoute les nouveaux asteroides au vector
		for(auto a:newAsteroids)
			vecAsteroids.push_back(a);

		// On retire les ast�roides r�duits � peau de chagrin
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), [&](sSpaceObject o) { return (o.x < 0); });
			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		if (vecAsteroids.empty()) // S'il n'y a plus d'ast�roides... c'est gagn� !
		{
			nScore += 1000;
			vecAsteroids.clear();
			vecLasers.clear();

			// On cr�e deux ast�roides, � une position o� le joueur ne se trouve pas
			// c-�-d 90� � gauche et � droite de celui-ci.
			vecAsteroids.push_back({ (int)16, 30.0f * sinf(player.angle - 3.14159f/2.0f) + player.x,
											  30.0f * cosf(player.angle - 3.14159f/2.0f) + player.y,
											  10.0f * sinf(player.angle), 10.0f*cosf(player.angle), 0.0f });

			vecAsteroids.push_back({ (int)16, 30.0f * sinf(player.angle + 3.14159f/2.0f) + player.x,
											  30.0f * cosf(player.angle + 3.14159f/2.0f) + player.y,
											  10.0f * sinf(-player.angle), 10.0f*cosf(-player.angle), 0.0f });
		}

		// Pour �liminer les lasers sortis de l'�cran (on les y a plac� quand ils ont touch� un ast�roide)
		if (vecLasers.size() > 0)
		{
			auto i = remove_if(vecLasers.begin(), vecLasers.end(), [&](sSpaceObject o) { return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() - 1 || o.y >= ScreenHeight() - 1); });
			if (i != vecLasers.end())
				vecLasers.erase(i);
		}

		// Affiche les lasers
		for (auto b : vecLasers)
			Draw(b.x, b.y);

		// Affiche le vaisseau
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

		// Affiche le score
		DrawString(2, 2, "SCORE: " + nScore);
		return true;
	}

	void DrawWireFrameModel(const vector<pair<float, float>> &vecModelCoordinates,
		float x, float y, float r = 0.0f, float s = 1.0f)
	{
		// pair.first = les x
		// pair.second = les y
		
		// Translation des coordonn�es
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotation
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Mise � l'�chelle
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		// Translation
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		// Dessine un polygone ferm�
		for (int i = 0; i < verts + 1; i++)
		{
			int j = (i + 1);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second, 
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second);
		}
	}
};

//Le PixelGameEngine permet de lancer une fen�tre
int main()
{
	Asteroids game;
	game.Construct(320, 280, 4, 4);
	game.Start();
	return 0;
}
