#include "HoangDuocSu.h"
#include "layer/GameScene.h"
#include "manager/RefManager.h"
#include "AudioEngine.h"


HoangDuocSu::HoangDuocSu(string jsonFile, string atlasFile, float scale) : BaseHero(jsonFile, atlasFile, scale)
{
}

HoangDuocSu * HoangDuocSu::create(string jsonFile, string atlasFile, float scale)
{
	HoangDuocSu* hds = new HoangDuocSu(jsonFile, atlasFile, scale);
	if (hds && hds->init())
	{
		hds->setTag(TAG_HERO);

		hds->update(0.0f);

		hds->stateMachine = new StateMachine(hds);
		hds->stateMachine->setCurrentState(MLand);

		hds->setBoxHeight(hds->getBoundingBox().size.height / 4.0f);

		//
		hds->blash = Sprite::create("Animation/HoangDuocSu/blash.png");
		hds->blash->setScale(scale / 2);
		hds->blash->setPosition(hds->getContentSize() / 2);
		hds->blash->setVisible(false);
		hds->addChild(hds->blash);

		hds->wave = Sprite::create("Animation/HoangDuocSu/skill3.png");
		hds->wave->setScale(scale / 3);
		hds->wave->setPosition(hds->getContentSize() / 2);
		hds->wave->setVisible(false);
		hds->addChild(hds->wave);

		hds->scoreRatio = 2;

		hds->autorelease();
		return hds;
	}
	else
	{
		delete hds;
		hds = nullptr;
		return nullptr;
	}
}

void HoangDuocSu::initCirclePhysic(b2World * world, Point pos)
{
	BaseHero::initCirclePhysic(world, pos);

	// rada here
	shield = Rada::create("Animation/CoLong/blash.png");
	shield->setScale(this->getTrueRadiusOfHero() / shield->getContentSize().width * 2.2f);
	shield->setVisible(false);
	shield->initCirclePhysic(world, Vec2(this->getB2Body()->GetPosition().x, this->getB2Body()->GetPosition().y));
	shield->changeBodyCategoryBits(BITMASK_WOODER);
	shield->changeBodyMaskBits(BITMASK_ENEMY | BITMASK_SLASH | BITMASK_BOSS | BITMASK_WOODER | BITMASK_COIN_BAG);
}

void HoangDuocSu::initSwordPhysic(b2World * world, Point position)
{
	b2BodyDef bodyDef;
	b2PolygonShape shape;
	b2FixtureDef fixtureDef;

	shape.SetAsBox(trueRadiusOfHero / PTM_RATIO, trueRadiusOfHero * 1.55f / PTM_RATIO);

	fixtureDef.density = 0.0f;
	fixtureDef.friction = 0.0f;
	fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &shape;

	fixtureDef.filter.categoryBits = BITMASK_WOODER;
	fixtureDef.filter.maskBits = BITMASK_WOODER | BITMASK_SLASH | BITMASK_BOSS | BITMASK_COIN_BAG | BITMASK_ENEMY;

	bodyDef.position.Set(position.x / PTM_RATIO, position.y / PTM_RATIO);
	bodyDef.type = b2_dynamicBody;

	swordBody = world->CreateBody(&bodyDef);
	swordBody->CreateFixture(&fixtureDef);

	spSkeleton* skeleton = this->getSkeleton();
}

void HoangDuocSu::fastAndFurious()
{
	this->setVisible(false);
	runEffectSkill1();
	// create a rada to change bitmask instead
	this->isNoDie = true;
	this->getB2Body()->SetGravityScale(0);
	shield->changeBodyCategoryBits(BITMASK_SWORD);

	this->schedule([&](float dt) {
		checkDurationSkill1++;

		this->getB2Body()->SetLinearVelocity(b2Vec2(getMoveVel() * 5.0f, 0.0f));

		if (checkDurationSkill1 >= getDurationSkill1() * 35) {
			effectSkill1->setVisible(false);
			this->setVisible(true);
		}

		if (checkDurationSkill1 >= getDurationSkill1() * 60) {
			setScoreRatio(1);
			this->setIsNoDie(false);
			shield->changeBodyCategoryBits(BITMASK_WOODER);
			this->getB2Body()->SetGravityScale(1);
			auto currentVelY = getB2Body()->GetLinearVelocity().y;
			this->getB2Body()->SetLinearVelocity(b2Vec2(getMoveVel() / 4, currentVelY));
			setIsDoneDuration1(true);
			checkDurationSkill1 = 0;
			setIsInSpecialMode(false);
			unschedule("KeySkill1");
		}

	}, 1.0f / 60, "KeySkill1");		//  run every delta second
}


void HoangDuocSu::doCounterSkill1()
{
	setScoreRatio(2);
	isInSpecialMode = true;
	fastAndFurious();
}

void HoangDuocSu::createDCTC(Point pos, float angle)
{
	float vx = SCREEN_SIZE.width * 1.5f / PTM_RATIO * cosf(angle / 180 * PI);
	float vy = SCREEN_SIZE.width * 1.5f / PTM_RATIO * sinf(angle / 180 * PI);

	auto dctc = (DaCauBongPhap*)poolSkill2->getObjectAtIndex(indexSkill2++);
	dctc->setVel(b2Vec2(vx, vy));
	dctc->setVisible(true);

	auto gameLayer = (GameScene*) this->getParent();

	dctc->setPosition(pos.x + this->getTrueRadiusOfHero() / 2, pos.y);
	dctc->initCirclePhysic(gameLayer->world, dctc->getPosition());

	if (!dctc->getIsAdded()) {
		this->getParent()->addChild(dctc, ZORDER_SMT);
		dctc->setIsAdded(true);
	}

	listDCTC.push_back(dctc);

	if (indexSkill2 == 12) indexSkill2 = 0;
}

void HoangDuocSu::slashDCTC()
{
	this->schedule([&](float dt) {
		checkDurationSkill2++;

		if (!isDoneDuration2 && checkDurationSkill2 >= getDurationSkill2() * 60) {
			counterSkill2 = 0;
			effectSkill2->setVisible(false);
			setIsDoneDuration2(true);
		}

		// only to update
		if (!listDCTC.empty()) {
			for (auto dctc : listDCTC) {
				if (!dctc->getB2Body()) continue;
				if (dctc->getPositionX() - (this->getPositionX() + SCREEN_SIZE.width * 0.255f) > SCREEN_SIZE.width / 2) {
					auto gameLayer = (GameScene*) this->getParent();

					gameLayer->world->DestroyBody(dctc->getB2Body());
					dctc->setB2Body(nullptr);
					dctc->setVisible(false);
				}
				else
					dctc->updateMe();
			}
		}

		if (checkDurationSkill2 >= getDurationSkill2() * 85) {
			setScoreRatio(1);
			checkDurationSkill2 = 0;
			unschedule("KeySkill2");
		}

	}, 1.0f / 60, "KeySkill2");		//  run every delta time
}

void HoangDuocSu::doCounterSkill2()
{
	effectSkill2->setVisible(true);
	setScoreRatio(2);
	slashDCTC();
}


void HoangDuocSu::killAll()
{
	auto boss = (BaseEnemy*) this->getParent()->getChildByTag(TAG_BOSS);
	if (boss != nullptr && boss->getPositionX() < this->getPositionX() + SCREEN_SIZE.width * 0.75f) {
		boss->die();
		log("%i", boss->getHealth());
	}

	isKillAll = true;
	wave->setVisible(true);
	//auto originScale = blash->getScale();
	auto scaleFactor = Director::getInstance()->getContentScaleFactor();
	auto scale = ScaleBy::create(0.64f, 150 * scaleFactor);

	auto hide = CallFunc::create([&]() {
		wave->setVisible(false);
		isKillAll = false;
	});

	wave->runAction(Sequence::create(scale, hide, scale->reverse(), nullptr));
}

void HoangDuocSu::doCounterSkill3()
{
	//effectSkill3->setVisible(true);
	/*this->getB2Body()->SetGravityScale(0);
	this->getB2Body()->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

	isDoneMoving = false;*/
	setScoreRatio(2);
	this->schedule([&](float dt) {
		/*if (!isDoneMoving) {
			if (fabs(this->getPositionY() - follow->getPositionY()) < offset)
				isDoneMoving = true;

			b2Vec2 newPos;
			if (this->getPositionY() < follow->getPositionY())
				newPos = b2Vec2(getB2Body()->GetPosition().x, getB2Body()->GetPosition().y + offset / 2 / PTM_RATIO);
			else
				newPos = b2Vec2(getB2Body()->GetPosition().x, getB2Body()->GetPosition().y - offset / 2 / PTM_RATIO);

			this->getB2Body()->SetTransform(newPos, this->getB2Body()->GetAngle());

		}*/

		if ((checkDurationSkill3 % 13 == 0 && checkDurationSkill3 <= (getDurationSkill3() * 10) - 7)) {
			killAll();
			//log("AA");
		}

		checkDurationSkill3++;

		if (checkDurationSkill3 >= getDurationSkill3() * 10) {
			//this->getB2Body()->SetGravityScale(1);
			//effectSkill3->setVisible(false);
			setScoreRatio(1);
			setIsDoneDuration3(true);
			checkDurationSkill3 = 0;
			unschedule("KeySkill3");
		}

	}, 0.1f, "KeySkill3");		//  run every delta time
}

void HoangDuocSu::runEffectSkill1()
{
	auto pos = Vec2(this->getPositionX(), this->getPositionY());
	effectSkill1->setPosition(pos);
	effectSkill1->setVisible(true);
	effectSkill1->clearTracks();
	effectSkill1->addAnimation(0, "animation", false);
	effectSkill1->setToSetupPose();

	/*this->clearTracks();
	this->addAnimation(0, "skill1", false);
	this->setToSetupPose();*/
}

void HoangDuocSu::createEffect()
{
	auto scale_1 = getBoxHeight() / 170;
	effectSkill1 = new SkeletonAnimation("Animation/HoangDuocSu/Skill_1_effect.json", "Animation/HoangDuocSu/Skill_1_effect.atlas", scale_1);
	effectSkill1->autorelease();
	effectSkill1->setVisible(false);
	this->getParent()->addChild(effectSkill1, ZORDER_ENEMY);

	auto scale_2 = getBoxHeight() / 140;
	effectSkill2 = new SkeletonAnimation("Animation/HoangDuocSu/Skill_2_effect1.json", "Animation/HoangDuocSu/Skill_2_effect1.atlas", scale_2);
	effectSkill2->autorelease();
	effectSkill2->setOpacity(100);
	effectSkill2->setVisible(false);
	effectSkill2->clearTracks();
	effectSkill2->addAnimation(0, "animation", true);
	effectSkill2->setToSetupPose();
	this->addChild(effectSkill2);

	/*auto scale_3 = getBoxHeight() / 175;
	effectSkill3 = new SkeletonAnimation("Animation/HoangDuocSu/Skill_3_effect1.json", "Animation/HoangDuocSu/Skill_3_effect1.atlas", scale_3);
	effectSkill3->autorelease();
	effectSkill3->setVisible(false);
	effectSkill3->clearTracks();
	effectSkill3->addAnimation(0, "animation", true);
	effectSkill3->setToSetupPose();
	this->addChild(effectSkill3);*/
}

void HoangDuocSu::addStuff()
{
	// slash here
	//createSlash();
	BaseHero::addStuff();

	createEffect();
}

void HoangDuocSu::createPool()
{
	/*auto gameLayer = (GameScene*) this->getParent();
	follow = gameLayer->getFollow();
	offset = SCREEN_SIZE.height / 32;*/

	poolSkill2 = CCArray::createWithCapacity(12);
	poolSkill2->retain();

	for (int i = 0; i < 12; ++i) {
		auto value = random(0.8f, 1.5f);
		auto scale = getTrueRadiusOfHero() / value / 128;
		auto dctc = DaCauBongPhap::create("Animation/HoangDuocSu/Skill_2_effect2.json", 
			"Animation/HoangDuocSu/Skill_2_effect2.atlas", scale);
		poolSkill2->addObject(dctc);
	}
}

void HoangDuocSu::run()
{
	clearTracks();
	addAnimation(0, "run", true);
	setToSetupPose();

	if (!getSmokeRun()->isVisible()) {
		getSmokeRun()->setVisible(true);
	}

	//log("run");
}

void HoangDuocSu::normalJump()
{
	BaseHero::normalJump();
	clearTracks();
	addAnimation(0, "jump", false);
	setToSetupPose();

	getSmokeRun()->setVisible(false);

	//log("jump");
}

void HoangDuocSu::doubleJump()
{
	BaseHero::doubleJump();
	clearTracks();
	addAnimation(0, "jumpx2", false);
	setToSetupPose();

	getSmokeJumpX2()->setPosition(this->getPosition());
	getSmokeJumpX2()->setVisible(true);
	smokeJumpX2Ani();
	//log("jumpx2");
}

void HoangDuocSu::landing()
{
	clearTracks();
	addAnimation(0, "landing", true);
	setToSetupPose();

	getSmokeRun()->setVisible(false);

	//log("land");
}

void HoangDuocSu::die()
{
	BaseHero::die();
	AudioManager::playSound(SOUND_DQDIE);
}

void HoangDuocSu::attackNormal()
{
	if (!isDoneDuration2) {
		counterSkill2++;
		if ((counterSkill2 % 3) == 0) {
			createDCTC(getBoneLocation("bone58"), 25);
			createDCTC(getBoneLocation("bone58"), -25);
		}

		createDCTC(getBoneLocation("bone58"), 15);
		createDCTC(getBoneLocation("bone58"), 5);
		createDCTC(getBoneLocation("bone58"), -5);
		createDCTC(getBoneLocation("bone58"), -15);
	}

	BaseHero::attackNormal();
	changeSwordCategoryBitmask(BITMASK_SWORD);

	setIsPriorAttack(true);

	//runSlash();

	clearTracks();
	
	if (isDoneDuration2) {
		auto r = rand() % 2;
		if (r) {
			addAnimation(0, "attack1", false);
		}
		else {
			addAnimation(0, "attack2", false);
		}
	} else 
		addAnimation(0, "attack1", false);
	

	//log("atttack*");
	setToSetupPose();

	getSlashBreak()->setVisible(false);

}

void HoangDuocSu::attackLanding()
{
	if (!isDoneDuration2) {
		counterSkill2++;
		if ((counterSkill2 % 3) == 0) {
			createDCTC(getBoneLocation("bone58"), 25);
			createDCTC(getBoneLocation("bone58"), -25);
		}

		createDCTC(getBoneLocation("bone58"), 15);
		createDCTC(getBoneLocation("bone58"), 5);
		createDCTC(getBoneLocation("bone58"), -5);
		createDCTC(getBoneLocation("bone58"), -15);
	}
	BaseHero::attackLanding();
	changeSwordCategoryBitmask(BITMASK_SWORD);
	setIsPriorAttack(true);
	//runSlashLand();

	clearTracks();
	if (!isDoneDuration2) {
		addAnimation(0, "attack1", false);
	}
	else {
		addAnimation(0, "attack3", false);
		this->getSwordBody()->SetTransform(getSwordBody()->GetPosition(), PI / 2);
	}

	setToSetupPose();

	//log("atttack");
	getSlashBreak()->setVisible(false);

}

void HoangDuocSu::injured()
{
	AudioManager::playSound(SOUND_DQHIT);
	clearTracks();
	addAnimation(0, "injured", false);
	setToSetupPose();

	//log("injured");

}

void HoangDuocSu::listener()
{

	this->setEndListener([&](int trackIndex) {

		if (strcmp(getCurrent()->animation->name, "injured") == 0) {

			this->getBloodScreen()->setVisible(false);

			if (getFSM()->globalState == MAttack) {
				getFSM()->setPreviousState(MInjured);
				getFSM()->setGlobalState(MRun);
			}

			else if (getFSM()->globalState == MDoubleJump || getFSM()->globalState == MRevive) {
				getFSM()->setPreviousState(MInjured);
				getFSM()->setGlobalState(MLand);
			}

			getFSM()->revertToGlobalState();
			setIsPriorInjured(false);
		}

		else if (strcmp(getCurrent()->animation->name, "revive") == 0) {
			getReviveMe()->setVisible(false);
			getFSM()->changeState(MLand);
			auto gameLayer = (GameScene*) this->getParent();
			initCirclePhysic(gameLayer->world, this->getPosition());

			gameLayer->getHud()->resumeIfVisible();
			gameLayer->enableCalling();

			noActive = false;
		}


		else if ((strcmp(getCurrent()->animation->name, "attack1") == 0) ||
			(strcmp(getCurrent()->animation->name, "attack2") == 0) ||
			(strcmp(getCurrent()->animation->name, "attack3") == 0)) {
			if(strcmp(getCurrent()->animation->name, "attack3") == 0)
				this->getSwordBody()->SetTransform(getSwordBody()->GetPosition(), 0);
			changeSwordCategoryBitmask(BITMASK_ENEMY);

			setIsPriorAttack(false);
			if (getFSM()->globalState == MInjured) {
				getFSM()->setPreviousState(MAttack);
				getFSM()->setGlobalState(MRun);
			}

			else if (getFSM()->globalState == MDoubleJump || getFSM()->globalState == MRevive) {
				getFSM()->setPreviousState(MAttack);
				getFSM()->setGlobalState(MLand);
			}

			getFSM()->revertToGlobalState();

		}

		// SKILL 1
		else if (strcmp(getCurrent()->animation->name, "skill1") == 0) {
			getFSM()->revertToGlobalState();
			setIsPriorSkill1(false);
		}

		//// SKILL 2
		//else if (strcmp(getCurrent()->animation->name, "skill2") == 0) {
		//	getFSM()->revertToGlobalState();
		//	setIsPriorSkill2(false);
		//}

		else if (strcmp(getCurrent()->animation->name, "skill3") == 0) {
			getFSM()->revertToGlobalState();
			setIsPriorSkill3(false);
		}

		else if (strcmp(getCurrent()->animation->name, "die") == 0) {
			this->pause();
			auto gamelayer = (GameScene*)this->getParent();
			gamelayer->dieGame();
		}
	});
}

void HoangDuocSu::stopSkillAction(bool stopSkill1, bool stopSkill2, bool stopSkill3)
{
	setScoreRatio(1);
	if (stopSkill1 && !getIsDoneDuration1()) {
		this->setIsNoDie(false);
		shield->changeBodyCategoryBits(BITMASK_WOODER);
		this->getB2Body()->SetGravityScale(1);

		setIsInSpecialMode(false);
		setIsDoneDuration1(true);
		unschedule("KeySkill1");
		checkDurationSkill1 = 0;
	}

	if (stopSkill2 && !getIsDoneDuration2()) {
		effectSkill2->setVisible(false);
		setIsDoneDuration2(true);
		if (!listDCTC.empty()) {
			for (auto dctc : listDCTC) {
				if (!dctc->getB2Body()) continue;

				auto gameLayer = (GameScene*) this->getParent();

				gameLayer->world->DestroyBody(dctc->getB2Body());
				dctc->setB2Body(nullptr);
				dctc->setVisible(false);
			}
		}
		listDCTC.clear();
		unschedule("KeySkill2");
		checkDurationSkill2 = 0;
	}

	if (stopSkill3 && !getIsDoneDuration3()) {
		//effectSkill3->setVisible(false);
		isKillAll = false;
		setIsDoneDuration3(true);
		unschedule("KeySkill3");
		checkDurationSkill3 = 0;
	}
}

void HoangDuocSu::doDestroyBodies(b2World* world)
{
	BaseHero::doDestroyBodies(world);
	world->DestroyBody(shield->getB2Body());
	shield->setB2Body(nullptr);
}

void HoangDuocSu::updateMe(float dt)
{
	BaseHero::updateMe(dt);

	getFSM()->Update();

	if (getB2Body() == nullptr)
		return;

	if (shield != nullptr) {
		auto mainPos = this->getB2Body()->GetPosition();
		shield->getB2Body()->SetTransform(b2Vec2(mainPos.x + trueRadiusOfHero / PTM_RATIO, mainPos.y), 0.0f);
	}

	auto currentVelY = getB2Body()->GetLinearVelocity().y;

	if (getFSM()->currentState == MDie) {
		getB2Body()->SetLinearVelocity(b2Vec2(0, currentVelY));
		return;
	}

	if (this->getPositionY() < 0) {
		return;
	}
	/*
	if (!isDoneDuration3) {
		getB2Body()->SetLinearVelocity(b2Vec2(getMoveVel(), 0));
		return;
	}*/

	if (!isDriverEagle) {
		getB2Body()->SetLinearVelocity(b2Vec2(getMoveVel(), currentVelY));
	}

	if (!getIsPriorAttack() && !getIsPriorInjured()) {

		if (getB2Body()->GetLinearVelocity().y < 0) {
			getFSM()->changeState(MLand);
			return;
		}

		if (getOnGround() && !getIsDriverEagle()) {
			getFSM()->changeState(MRun);
		}
	}

}
