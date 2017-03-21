#ifndef __HUD_H__
#define __HUD_H__

#include "Button.h"


class Hud : public Layer
{
public:

	virtual bool init();
	const Size SCREEN_SIZE = Director::getInstance()->getVisibleSize();	
	CREATE_FUNC(Hud);

protected:
	CC_SYNTHESIZE(Sprite*, avatar, AvatarSprite);
	CC_SYNTHESIZE(Sprite*, bloodBoard, BloodBoard);
	CC_SYNTHESIZE(Sprite*, moneyBoard, MoneyBoard);
	CC_SYNTHESIZE(Sprite*, scoreBoard, ScoreBoard);
	CC_SYNTHESIZE(Button*, btnAttack, BtnAttack);
	CC_SYNTHESIZE(Button*, btnSkill_1, BtnSkill_1);
	CC_SYNTHESIZE(Button*, btnSkill_2, BtnSkill_2);
	CC_SYNTHESIZE(Button*, btnSkill_3, BtnSkill_3);

private:
	TMXTiledMap *tmxMap;
	void addProfile();
	void addButton();

};

#endif // __HUD_H__