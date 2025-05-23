#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	const static uint DISABLE_INVULNERABILITY = 4;

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	void LivesChange(int lives_gain);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	uint mLevel;
	uint mAsteroidCount;
	uint mExtraLivesPowerup;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	void CreateExtraLives(const uint num_extralives);
	void CreateBlackHole(const uint num_blackholes);
	void CreateInvulnerability(const uint num_powerups);
	shared_ptr<GameObject> CreateExplosion();

	std::vector<shared_ptr<GameObject>> mBlackHoles; 
	
	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint DESTROY_BLACKHOLE = 3;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;

	bool mEnteringName = false;
	std::string mNameEntry = "";
	shared_ptr<GUILabel> mNameEntryLabel;
	std::vector<shared_ptr<GUILabel>> mHighScoreLabels;
};

#endif