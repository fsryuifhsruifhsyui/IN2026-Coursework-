#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "HighScoreKeeper.h"
#include "BonusLife.h"
#include "BlackHole.h"
#include "Invulnerability.h"

shared_ptr<GUILabel> mTitleLabel;
shared_ptr<GUILabel> mStartLabel;
shared_ptr<GUILabel> mInstructionsLabel;
shared_ptr<GUILabel> mDifficultyLabel;
bool gameStarted = false;

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mExtraLivesPowerup = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	Animation* extralife_anim = AnimationManager::GetInstance().CreateAnimationFromFile("Heart_fs", 64, 64, 64, 64, "Heart_fs.png");
	Animation* blackhole_anim = AnimationManager::GetInstance().CreateAnimationFromFile("blackhole_fs", 128, 128, 128, 128, "blackhole_fs.png");
	Animation* invuln_anim = AnimationManager::GetInstance().CreateAnimationFromFile("invulnerability_fs", 64, 64, 64, 64, "invulnerability_fs.png");

	// Menu GUI Stuff
	mTitleLabel = make_shared<GUILabel>(" ASTEROIDS ");
	mTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mTitleLabel->SetColor(GLVector3f(0.8f, 0.2f, 1.0f));
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mTitleLabel), GLVector2f(0.5f, 0.7f));

	mStartLabel = make_shared<GUILabel>("Press SPACE key to start");
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mStartLabel->SetColor(GLVector3f(0.8f, 0.2f, 1.0f));
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mStartLabel), GLVector2f(0.5f, 0.5f));

	mInstructionsLabel = make_shared<GUILabel>("Press 'I' to see game instructions");
	mInstructionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mInstructionsLabel->SetColor(GLVector3f(0.0f, 1.0f, 0.0f));
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mInstructionsLabel), GLVector2f(0.5f, 0.4f));

	mDifficultyLabel = make_shared<GUILabel>("Press 'D' to change game difficulty");
	mDifficultyLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mDifficultyLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mDifficultyLabel->SetColor(GLVector3f(1.0f, 0.6f, 0.6f));
	mGameDisplay->GetContainer()->AddComponent(static_pointer_cast<GUIComponent>(mDifficultyLabel), GLVector2f(0.5f, 0.3f));


	// Create some asteroids and add them to the world
	CreateAsteroids(2);
	
	//Create the GUI
	CreateGUI();
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mGameOverLabel->SetVisible(false);

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{

	if (mEnteringName) {
		if (key == 13 || key == '\r') { // Enter key
			int score = mScoreKeeper.GetScore();
			HighScoreKeeper::SaveScore(mNameEntry, score);
			auto topScores = HighScoreKeeper::LoadScores();

			// Hide entry label
			mGameDisplay->GetContainer()->RemoveComponent(mNameEntryLabel);
			mEnteringName = false;

			// Show top 3 scores
			for (auto& label : mHighScoreLabels) {
				mGameDisplay->GetContainer()->RemoveComponent(label);
			}
			mHighScoreLabels.clear();

			for (int i = 0; i < topScores.size() && i < 3; ++i) {
				std::string text = std::to_string(i + 1) + ". " + topScores[i].name + " - " + std::to_string(topScores[i].score);
				auto label = make_shared<GUILabel>(text);
				label->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
				label->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
				mGameDisplay->GetContainer()->AddComponent(label, GLVector2f(0.5f, 0.2f - i * 0.05f));
				mHighScoreLabels.push_back(label);
			}
		}
		else if (key == 8 && !mNameEntry.empty()) { // Backspace
			mNameEntry.pop_back();
		}
		else if (isprint(key) && mNameEntry.length() < 10) {
			mNameEntry += key;
		}

		if (mNameEntryLabel) {
			mNameEntryLabel->SetText("Enter Name: " + mNameEntry);
		}
		return;
	}

	if (!gameStarted && key == ' ') {
		// Start the game
		gameStarted = true;
		mTitleLabel->SetVisible(false);
		mStartLabel->SetVisible(false);
		mInstructionsLabel->SetVisible(false);
		mDifficultyLabel->SetVisible(false);
		mScoreLabel->SetVisible(true);
		mLivesLabel->SetVisible(true);

		// Create game objects
		mGameWorld->AddObject(CreateSpaceship());
		CreateAsteroids(1);
	}
	else if (!gameStarted && (key == 'i' || key == 'I')) {
		// Show instructions
		mDifficultyLabel->SetVisible(false);
		mTitleLabel->SetText("HOW TO PLAY:");
		mStartLabel->SetText(" Arrow keys to move, SPACE to shoot!");
		mInstructionsLabel->SetText("Press SPACE to start game");
	}
	else if (!gameStarted && (key == 'd' || key == 'D')) {
		mDifficultyLabel->SetVisible(false);
		mTitleLabel->SetText("Heart - Gain Extra Lives");
		mStartLabel->SetText("Black Hole - Destroy Asteroids");
		mInstructionsLabel->SetText("Gold Shield - Temporary Invincible");
	}
	else if (gameStarted && key == ' ') {
		mSpaceship->Shoot();
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
	// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
        if (mSpaceship) {
            mSpaceship->Rotate(90);
        }
	// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
		if (mSpaceship) {
			mSpaceship->Rotate(90);
		}
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0) 
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
	}

	if (object->GetType() == GameObjectType("BonusLife"))
	{
		mExtraLivesPowerup++;
	}

	if (object->GetType() == GameObjectType("Invulnerability"))
	{
		SetTimer(5000, DISABLE_INVULNERABILITY);
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 1 + mLevel;
		if (mLevel % 3 == 0) {
			CreateExtraLives(2);
		}
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);

		// Start name entry mode
		mEnteringName = true;
		mNameEntry = "";
		mNameEntryLabel = make_shared<GUILabel>("Enter Name: ");
		mNameEntryLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		mNameEntryLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
		mGameDisplay->GetContainer()->AddComponent(mNameEntryLabel, GLVector2f(0.5f, 0.35f));
	}

	if (value == DESTROY_BLACKHOLE)
	{
		if (!mBlackHoles.empty()) {
			mGameWorld->FlagForRemoval(mBlackHoles.front());
			mBlackHoles.erase(mBlackHoles.begin());
		}
	}

	if (value == DISABLE_INVULNERABILITY)
	{
		if (mSpaceship) mSpaceship->SetInvulnerable(false);
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateExtraLives(const uint num_bonuslife)
{
	mExtraLivesPowerup = num_bonuslife;
	for (uint i = 0; i < num_bonuslife; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("Heart_fs");
		shared_ptr<Sprite> bonuslife_sprite 
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		shared_ptr<GameObject> bonuslife = make_shared<BonusLife>();
		bonuslife->SetBoundingShape(make_shared<BoundingSphere>(bonuslife->GetThisPtr(), 3.0f));
		bonuslife->SetSprite(bonuslife_sprite);
		bonuslife->SetScale(0.1f);
		mGameWorld->AddObject(bonuslife);
	}
}

void Asteroids::CreateBlackHole(const uint num_blackholes)
{
	for (uint i = 0; i < num_blackholes; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("blackhole_fs");
		shared_ptr<Sprite> blackhole_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);

		shared_ptr<GameObject> blackhole = make_shared<BlackHole>();
		blackhole->SetBoundingShape(make_shared<BoundingSphere>(blackhole->GetThisPtr(), 2.0f));
		blackhole->SetSprite(blackhole_sprite);
		blackhole->SetScale(0.2f);

		mGameWorld->AddObject(blackhole);
		mBlackHoles.push_back(blackhole); 

		SetTimer(3000, DESTROY_BLACKHOLE); 
	}
}

void Asteroids::CreateInvulnerability(const uint num_powerups)
{
	for (uint i = 0; i < num_powerups; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("invulnerability_fs");
		shared_ptr<Sprite> invuln_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);

		shared_ptr<GameObject> invulnerability = make_shared<Invulnerability>();
		invulnerability->SetBoundingShape(make_shared<BoundingSphere>(invulnerability->GetThisPtr(), 3.0f));
		invulnerability->SetSprite(invuln_sprite);
		invulnerability->SetScale(0.2f);

		mGameWorld->AddObject(invulnerability);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);

	if (score % 120 == 0 && score != 0) {
		CreateBlackHole(1);
	}
	if (score % 80 == 0 && score != 0){
		CreateInvulnerability(1); 
	}

}

void Asteroids::LivesChange(int lives_gain)
{
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_gain;
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		SetTimer(1000, CREATE_NEW_PLAYER); 
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}




