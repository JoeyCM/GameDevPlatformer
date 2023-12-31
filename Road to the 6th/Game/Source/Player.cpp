#include "Player.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Map.h"
#include "ModuleFadeToBlack.h"


Player::Player() : Entity(EntityType::PLAYER)
{
	name.Create("Player");
}

Player::~Player() {

}

bool Player::Awake() {

	return true;
}

bool Player::Start() {

	//L02: DONE 1: Initialize Player parameters
	startPos.x = parameters.attribute("x").as_int();
	startPos.y = parameters.attribute("y").as_int();

	texturePath = parameters.attribute("texturepath").as_string();

	width = 16;
	height = 16;

	idlePlayer.PushBack({ 0, 0, 13, 19 });
	idlePlayer.PushBack({ 13, 1, 13, 19 });
	idlePlayer.PushBack({ 13, 1, 13, 19 });
	idlePlayer.PushBack({ 0, 0, 13, 19 });
	idlePlayer.loop = true;
	idlePlayer.speed = 0.1f;

	runPlayer.PushBack({ 0, 20, 13, 19 });
	runPlayer.PushBack({ 15, 21, 14, 18 });
	runPlayer.PushBack({ 30, 20, 13, 19 });
	runPlayer.PushBack({ 45, 21, 13, 18 });
	runPlayer.loop = true;
	runPlayer.speed = 0.3f;

	attackPlayer.PushBack({ 0, 128, 65, 33 });
	attackPlayer.PushBack({ 64, 128, 65, 33 });
	attackPlayer.PushBack({ 128, 128, 65, 33 });
	attackPlayer.PushBack({ 192, 128, 65, 33 });
	attackPlayer.PushBack({ 256, 128, 65, 33 });
	attackPlayer.PushBack({ 319, 128, 65, 33 });
	attackPlayer.PushBack({ 384, 128, 65, 33 });
	attackPlayer.PushBack({ 448, 128, 65, 33 });
	attackPlayer.loop = false;
	attackPlayer.speed = 0.3f;

	diePlayer.PushBack({ 0, 40, 13, 19 });
	diePlayer.loop = false;
	diePlayer.speed = 0.1f;

	hitPlayer.PushBack({ 450, 64, 65, 33 });
	hitPlayer.loop = false;
	hitPlayer.speed = 0.1f;

	//initilize textures
	texture = app->tex->Load(texturePath);

	jumpSFXPath = app->configNode.child("player").child("SFXset").attribute("jumpSFXPath").as_string();
	dieSFXPath = app->configNode.child("player").child("SFXset").attribute("dieSFXPath").as_string();
	hurtSFXPath = app->configNode.child("player").child("SFXset").attribute("hurtSFXPath").as_string();
	pickCoinSFXPath = app->configNode.child("player").child("SFXset").attribute("pickCoinSFXPath").as_string();
	levelCompletedSFXPath = app->configNode.child("player").child("SFXset").attribute("levelCompletedSFXPath").as_string();
	selectSFXPath = app->configNode.child("player").child("SFXset").attribute("selectSFXPath").as_string();
	shortRangeAttackSFXPath = app->configNode.child("player").child("SFXset").attribute("shortRangeAttackSFXPath").as_string();

	// Loading the set of SFX, BETTER HERE FOR ENABLE/DISABLE
	jumpSFX = app->audio->LoadFx(jumpSFXPath);
	dieSFX = app->audio->LoadFx(dieSFXPath);
	hurtSFX = app->audio->LoadFx(hurtSFXPath);
	pickCoinSFX = app->audio->LoadFx(pickCoinSFXPath);
	levelCompletedSFX = app->audio->LoadFx(levelCompletedSFXPath);
	selectSFX = app->audio->LoadFx(selectSFXPath);
	shortRangeAttackSFX = app->audio->LoadFx(shortRangeAttackSFXPath);

	currentAnim = &idlePlayer;
	dead = false;
	lives = 3;

	//Add physics to the player - initialize physics body
	pbody = app->physics->CreateCircle(position.x - 2, position.y, width / 1.5, bodyType::DYNAMIC, ColliderType::PLAYER);

	pbody->listener = this;

	onGround = true;
	jumping = false;
	jumpCount = 2;

	return true;
}

bool Player::PreUpdate() {

	return true;
}

bool Player::Update()
{
	/*float speedMultiplier = app->GetAverageFPS() / 60; //Calculates the multiplier for changing speed by framerate

	LOG("PLAYER SPEED = %f", currentAnim->speed);

	if (app->scene->capTo30fps == true) {

		currentAnim->speed = currentAnim->speed *2;
	}*/

	currentAnim = &idlePlayer;

	//Enable/Disable Debug
	if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
	{
		app->physics->debug = !app->physics->debug;
		app->audio->PlayFx(selectSFX);
	}

	if (godMode == true) {

		velocity = { 0, 0 };
		pbody->body->SetGravityScale(0);

		// Fly around the map
		if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y = -5;
		}
		if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y = 5;
		}
		if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			isFliped = true;
			velocity.x = -5;
			if (isFliped == true && fliped == SDL_FLIP_NONE) {
				fliped = SDL_FLIP_HORIZONTAL;
			}
		}
		if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			isFliped = false;
			velocity.x = 5;
			if (isFliped == false && fliped == SDL_FLIP_HORIZONTAL) {
				fliped = SDL_FLIP_NONE;
			}
		}


	}
	else if(godMode == false && dead == false)
	{
		velocity = { 0, -GRAVITY_Y };

		//L02: DONE 4: modify the position of the player using arrow keys and render the texture
		if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {

			longPress = false;

			if (onGround == true) {
				jumping = true;
				jumpingTime = 0;

				app->audio->PlayFx(jumpSFX);
			}
			else if (onGround == false && jumpCount > 1 && jumping == true) {
				jumpingTime = 0;
				Jump();
				jumpCount--;

				app->audio->PlayFx(jumpSFX);
			}

			onGround = false;
		}
		if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {

			longPress = true;

			if (onGround == true) {
				jumping = true;
				jumpingTime = 0;

				app->audio->PlayFx(jumpSFX);
			}

			onGround = false;
		}

		if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {

			isFliped = true;

			velocity.x = -5;

			if (isFliped == true && fliped == SDL_FLIP_NONE) {
				fliped = SDL_FLIP_HORIZONTAL;
			}
			currentAnim = &runPlayer;

		}
		if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			isFliped = false;
	

			velocity.x = 5;

			if (isFliped == false && fliped == SDL_FLIP_HORIZONTAL) {
				fliped = SDL_FLIP_NONE;
			}
			currentAnim = &runPlayer;

		}

		//Reset player position input
		if (app->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN
			|| app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
		{
			ResetPlayerPos();
			app->audio->PlayFx(selectSFX);
		}

		// Being hit anim if slime or bat attacks the player
		if (onCollision) {
			currentAnim = &hitPlayer;

			if (hitPlayer.HasFinished()) {
				onCollision = false;
				hitPlayer.Reset();
			}
		}

		//Attacking animation function
		if (app->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
		{
			if (timeToAttack >= cooldownTime) 
			{
				attacking = true;
				app->audio->PlayFx(shortRangeAttackSFX);
			}

		}
		Attack();
		timeToAttack++;

		//Jumping Function
		if (jumping == true && jumpingTime <= 12) {
			Jump();
		}


		longPress = false;
	}

	pbody->body->SetLinearVelocity(velocity);
	
	// Link player's texture with pbody when moving, if player's dies then stop motion
	if (dead == true) {
		currentAnim = &diePlayer;
		position.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - 20;
		pbody->body->SetAwake(false);
	}
	else {
		position.x = METERS_TO_PIXELS(pbody->body->GetTransform().p.x - (width));
		position.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y - (height/1.5));
	}
	
	SDL_Rect rect = currentAnim->GetCurrentFrame();
	app->render->DrawTexture(texture, position.x, position.y, &rect, fliped);
	currentAnim->Update();
	

	return true;
}

bool Player::PostUpdate() {

	
	return true;
}

bool Player::CleanUp()
{

	return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {

	// L07 DONE 7: Detect the type of collision

	switch (physB->cType)
	{
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		break;
	case ColliderType::COIN:
		LOG("Collision COIN");
		coins++;
		app->audio->PlayFx(pickCoinSFX);
		LOG("COINS: %d", coins);
		break;
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		jumpVel = GRAVITY_Y;
		if (app->input->GetKey(SDL_SCANCODE_SPACE) != KEY_REPEAT) {
			onGround = true;
			jumping = false;
		}
		jumpCount = 2;
		break;
	case ColliderType::WATER:
		LOG("Collision WATER");
		if (godMode == false) {
			dead = true;
			coins = 0;
			app->audio->PlayFx(dieSFX);
			app->fade->FadeToBlack((Module*)app->scene, (Module*)app->endingscreen, 60);
		}
		break;
	case ColliderType::WIN_ZONE:
		LOG("Collision WIN ZONE");
		app->audio->PlayFx(levelCompletedSFX);
		app->fade->FadeToBlack((Module*)app->scene, (Module*)app->titlescreen, 90);
		break;
	case ColliderType::ENEMY:
		LOG("Collision ENEMY SLIME");
		if (godMode == false) {
			lives--;
			onCollision = true;
		}
	
		if (lives <= 0 && godMode == false) {
			dead = true;
			coins = 0;
			app->audio->PlayFx(dieSFX);
			app->fade->FadeToBlack((Module*)app->scene, (Module*)app->endingscreen, 60);
		}
		else if(godMode == false) {
			app->audio->PlayFx(hurtSFX);
		}
		break;
	case ColliderType::FLYING_ENEMY:
		LOG("Collision FLYING ENEMY BAT");
		if (godMode == false) {
			lives--;
			onCollision = true;
		}

		if (lives <= 0 && godMode == false) {
			dead = true;
			coins = 0;
			app->audio->PlayFx(dieSFX);
			app->fade->FadeToBlack((Module*)app->scene, (Module*)app->endingscreen, 60);
		}
		else if (godMode == false) {
			app->audio->PlayFx(hurtSFX);
		}
		break;
	case ColliderType::SLIME_HITBOX:
		LOG("Collison SLIME HEAD HITBOX");
		app->scene->slime->lives--;
		app->scene->slime->onCollision = true;
		if (app->scene->slime->lives <= 0) {
			app->scene->slime->dead = true;
		}
		app->audio->PlayFx(app->scene->slime->stompSFX);
		break;
	case ColliderType::BAT_HITBOX:
		LOG("Collison BAT HEAD HITBOX");
		app->scene->bat->lives--;
		app->scene->bat->onCollision = true;
		if (app->scene->bat->lives <= 0) {
			app->scene->bat->dead = true;
		}
		app->audio->PlayFx(app->scene->bat->stompSFX);
		break;
	case ColliderType::WALL:
		LOG("Collision WALL");
		break;
	case ColliderType::CAMERAFIX:
		LOG("Collision CameraFix");
		app->scene->cameraFix = true;
		break;
	case ColliderType::NONCAMERAFIX:
		LOG("Collision NONCameraFix");
		app->scene->cameraFix = false;
		break;
	case ColliderType::CAMERAFIX_2:
		LOG("Collision CameraFix 2");
		app->scene->cameraFix2 = true;
		break;
	case ColliderType::NONCAMERAFIX_2:
		LOG("Collision NONCameraFix 2");
		app->scene->cameraFix2 = false;
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	}

}

void Player::Jump() {
	velocity.y = jumpVel;

	//Mini Jump
	if (longPress == true)
		jumpVel = -15.0f;
	else if(jumpCount == 0)
		jumpVel = -20.0f;
	else
		jumpVel = -5.0f;

	jumpingTime++;
}

void Player::Attack() {

	if (attacking)
	{
		currentAnim = &attackPlayer;

		if (attackTime > 0)
		{
			if (fliped == false)
				hitbox = app->physics->CreateRectangleSensor(METERS_TO_PIXELS(pbody->body->GetTransform().p.x) + 15, METERS_TO_PIXELS(pbody->body->GetTransform().p.y), 35, 10, bodyType::STATIC, ColliderType::PLAYER_ATTACK_HITBOX);

			if (fliped == true)
				hitbox = app->physics->CreateRectangleSensor(METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - 15, METERS_TO_PIXELS(pbody->body->GetTransform().p.y), 35, 10, bodyType::STATIC, ColliderType::PLAYER_ATTACK_HITBOX);

		}
		attackTime--;

		if (attackPlayer.HasFinished()) {
			attacking = false;
			attackPlayer.Reset();
			attackPlayer.ResetLoopCount();
			timeToAttack = 0;
			attackTime = 1;

			app->physics->world->DestroyBody(hitbox->body);
		}
	}
}

void Player::ResetPlayerPos() {

	pbody->body->SetAwake(true);
	velocity = { 0, 0 };
	pbody->body->SetTransform(PIXEL_TO_METERS(startPos), 0.0f);
	app->scene->cameraFix2 = false;
	app->scene->cameraFix = false;
	app->render->camera.x = 0;
	dead = false;
	
	LOG("--RESETING PLAYER--");
}