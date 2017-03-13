#include "EnemyWooder.h"

EnemyWooder::EnemyWooder(string jsonFile, string atlasFile, float scale):BaseEnemy(jsonFile, atlasFile,scale)
{
}

EnemyWooder * EnemyWooder::create(string jsonFile, string atlasFile, float scale)
{
	EnemyWooder *enemy = new EnemyWooder(jsonFile, atlasFile,scale);
	enemy->update(0.0f);
	enemy->setTag(TAG_ENEMY_WOODER);
	enemy->setScaleX(1);
	enemy->setTimeScale(1.4f);
	return enemy;

}

void EnemyWooder::run()
{
}

void EnemyWooder::attack()
{
}

void EnemyWooder::die()
{
	//auto world = this->body->GetWorld();
	//world->DestroyBody(this->body);
	//body->SetType(b2_dynamicBody);
	this->setIsDie(true);
	this->clearTracks();
	this->addAnimation(0,"broken",false);
	this->setToSetupPose();
}

void EnemyWooder::updateMe(float dt)
{
	BaseEnemy::updateMe(dt);
	
}
