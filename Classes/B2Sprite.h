#ifndef __B2_SPRITE_H__
#define __B2_SPRITE_H__

#include "Global.h"
#include "spine/spine-cocos2dx.h"
#include <string>
#include "GB2ShapeCache-x.h"

using namespace spine;
using namespace std;

class B2Sprite : public Sprite
{
private:
	b2Body *body;
public:
	B2Sprite();
	~B2Sprite();


	static B2Sprite* create();

	// SCREEN_SIZE
	const Size SCREEN_SIZE = Director::getInstance()->getVisibleSize();
	//b2FixtureDef fixtureDef;
	//Sprite* boom;

	
	b2Body* getB2Body();
	/**
	* world: world containt this->body
	* pos: position to add body in pixel
	*/
	virtual void initBoxPhysic(b2World *world, Point pos);
	/**
	* world: world containt this->body
	* pos: position to add body in pixel
	*/
	virtual void initCirclePhysic(b2World *world, Point pos);
	/**
	* world: world containt this->body
	* pos: position to add body in pixel
	* key of shape cache
	*/
	virtual void initPhysicWithShapeCache(b2World *world, Point pos, string key);
	virtual void changeBodyCategoryBits(uint16 mask);
	virtual void changeBodyMaskBits(uint16 mask);
	virtual void update(float dt);
	//void explosion();

};

#endif // __B2_SPRITE_H__