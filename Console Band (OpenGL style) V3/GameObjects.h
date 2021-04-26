#pragma once
#include <GameEmgine.h>
#include <EmGineAudioPlayer.h>
#include <memory>
typedef GameEmGine Game;

//some extra utility functions
namespace util
{
	bool circleCollision(Model* m1, Model* m2, float r1, float r2)
	{
		Vec3 p1 = m1->getLocalPosition() * Vec3 { 1, 0, 1 }, p2 = m2->getLocalPosition() * Vec3 { 1, 0, 1 };


		if((p1 - p2).length() <= (r1 + r2))
			return true;

		return false;
	}

	bool boxCollision(Model* m1, Model* m2, float r1, float r2)
	{
		Vec3 p1 = m1->getLocalPosition() * Vec3 { 1, 1, 0 }, p2 = m2->getLocalPosition() * Vec3 { 1, 1, 0 };
		if((p1 - p2).length() <= (r1 + r2) * .5f)
			return true;

		return false;
	}

	bool mouseCircleCollision(Model* m, float r)
	{
		Vec3 p1 = {1, 1, 0}, p2 = m->getLocalPosition() * Vec3 { 1, 1, 0 };

		//App::GetMousePos(p1.x, p1.y);


		if((p1 - p2).length() <= r * .5)
			return true;

		return false;
	}

	bool mouseBoxCollision(Model* m, Vec2 s)
	{
		Vec3 p1 = {1, 1, 0}, p2 = m->getLocalPosition() * Vec3 { 1, 1, 0 };

		//App::GetMousePos(p1.x, p1.y);

		if(std::abs(p1.x - p2.x) > s.w * .25)
			return false;
		if(std::abs(p1.y - p2.y) > s.h * .25)
			return false;

		return true;
	}
}

class WayPoint: public Model
{
public:
	WayPoint():Model() { init(); };
	WayPoint(WayPoint& model, cstring tag = ""):Model(model, tag) { init(); }
	WayPoint(PrimitiveMesh* sprite, cstring tag = "Model"):Model(sprite, tag) { init(); }
	WayPoint(cstring spriteFile, cstring tag = "Model"):Model(spriteFile, tag) { init(); }

	void init()
	{}
	void update(float dt)
	{}

};

class Enemy: public Model
{
protected:
	float speed = 10;
	float health = 100;
	float radious = 0;
	int currentDest = 0;
	std::vector<std::shared_ptr<WayPoint>>dests;

public:

	Enemy():Model() {};
	Enemy(Enemy& model, cstring tag = ""):Model(model, tag) {}
	Enemy(PrimitiveMesh* sprite, cstring tag = "Model"):Model(sprite, tag) {}
	Enemy(cstring spriteFile, cstring tag = "Model"):Model(spriteFile, tag) {}

	void addDamage(float dmg) { health -= dmg; }
	float getHealth() { return health; }

	void setWayPoints(std::vector<std::shared_ptr<WayPoint>>& points) { dests = points; }

	virtual void init() = 0;

	virtual void update(float dt)
	{
		//speed = 0;
		if(!m_active)return;

		if(dests.empty()) { m_active = false; return; }

		if((dests[0]->getLocalPosition() - getLocalPosition()).length() <= speed * dt)
		{
			translate(dests[0]->getLocalPosition());
			dests.erase(dests.begin());
		}

		if(dests.empty())return;

		Vec3 direction = (dests[0]->getLocalPosition() - getLocalPosition()).normal();

		translateBy(direction * speed * dt);
	}

};

class QuarterEnemy: public Enemy
{
public:
	QuarterEnemy():Enemy() { init(); };
	QuarterEnemy(QuarterEnemy& model, cstring tag = ""):Enemy(model, tag) { init(); }

	void init()
	{
		//m_size.x = 50;
		speed *= 1;
		setColour(0.5, 0, 0.5);
	}

	void update(float dt)
	{
		Enemy::update(dt);
	}


};

class EighthEnemy: public Enemy
{
public:
	EighthEnemy():Enemy() { init(); };
	EighthEnemy(EighthEnemy& model, cstring tag = ""):Enemy(model, tag) { init(); }

	void init()
	{
		//	m_size.x = 50;
		speed *= 2;
		health = 50;
		setColour(.5, .5, 0);
	}

	void update(float dt)
	{
		Enemy::update(dt);
	}


};

class WholeEnemy: public Enemy
{
public:
	WholeEnemy():Enemy() { init(); };
	WholeEnemy(WholeEnemy& model, cstring tag = ""):Enemy(model, tag) { init(); }

	void init()
	{
		//	m_size.x = 50;
		speed *= .25;
		health = 400;
		setColour(255.0f / 255, 128.0f / 255, 0);
	}

	void update(float dt)
	{
		Enemy::update(dt);
	}


};


class Projectile: public Model
{
protected:
	std::vector<std::shared_ptr<Enemy>>* enemyList = nullptr;
	float speed = 15;
	float damage = 50;

	Model* destObj;
	Vec3 dest;
	Vec3 direction;

public:
	static std::shared_ptr< Model> referenceObj;
	Projectile() { init(); };


	Projectile(const Projectile& model, cstring tag = "Projectile"):Model(*(Projectile*)&model, tag)
	{
		*this = model;
		init();
	}
	void init()
	{
		referenceObj = std::shared_ptr<Model>(new Model(new PrimitiveSphere({1,1}, 4, 1)));
		m_type = "Projectile";
		//m_size.x = 10;
		setColour(1, 0, 0);
	}

	void setEnemy(Model* enemy)
	{
		destObj = enemy;
		dest = enemy->getLocalPosition();
		direction = (dest - getLocalPosition()).normal();
	}
	void setEnemyList(std::vector<std::shared_ptr<Enemy>>* list) { enemyList = list; };
	void setDamage(float dmg) { damage = dmg; }

	void update(float dt)
	{

		if(!m_active)return;
		if(!destObj)return;
		if(!enemyList)return;

		translateBy(direction * dt * speed);

		for(auto& enemy : *enemyList)
			if(util::circleCollision(this, enemy.get(), std::max(getWidth(), getDepth()), std::min(getWidth(), getDepth())))
			{
				enemy->addDamage(damage);
				m_active = false;
				break;
			}
	}


	bool operator==(Projectile other)
	{
		return
			getLocalPosition() == other.getLocalPosition() &&
			getDimentions() == other.getDimentions() &&
			std::string(getTag()) == other.getTag();
	}
};
std::shared_ptr<Model> Projectile::referenceObj;

class Tower: public Model
{
public:
	Tower(uint projectileLimit = 0):Model() { init(); };
	Tower(Tower& model, uint projectileLimit = 0, cstring tag = ""):
		Model(model, tag)
	{
		*this = model;
		init();
	}

	void setEnemyList(std::vector<std::shared_ptr<Enemy>>* list) { enemyList = list; };

	void setSongBPM(double bpm) { m_bps = 60 / bpm; }

	virtual void init() { /*bullets.setCapacity(50);*/ };

	virtual void update(float dt) = 0;

	static void bulletUpdate(float dt)
	{
		std::vector<std::shared_ptr<Projectile>>& bulletsRef = bullets.getObjectList();
		for(auto& a : bulletsRef)
			a->update(dt);
	}



protected:
	std::vector<std::shared_ptr<Enemy>>* enemyList;
	static ObjectPool<Projectile> bullets;

	float  m_damage = 0;
	float  m_duration = 0.6f;
	float  m_attackRange = 2;
	double m_bps = 0;
	int beatOn = 0, lastBeatOn = 0;
	double timer = 0;

};

ObjectPool<Projectile> Tower::bullets(10);


#include <algorithm>
class QuarterTower: public Tower
{
public:
	QuarterTower(uint projectileLimit = 0):Tower(projectileLimit) { init(); };
	QuarterTower(Tower& model, cstring tag = ""):Tower(model, 10, tag) { init(); }

	void init()
	{
		setColour(0, 255.0f / 255, 255.f / 255);

		//	m_size = {100,100};
	}

	void update(float dt)
	{
		for(auto p : bullets.getObjectList())
			p->update(dt);

		timer = AudioPlayer::getTimePosition(0) * .001;

		if((beatOn = (int)(timer / m_bps)) != lastBeatOn)
		{
			lastBeatOn = beatOn;

			Model* daOne = nullptr;
			auto a = enemyList->rbegin();

			std::sort(enemyList->rbegin(), enemyList->rend(),
					  [&](std::shared_ptr<Enemy> a, std::shared_ptr<Enemy> b)->bool{return (getLocalPosition() - a->getLocalPosition()) < (getLocalPosition() - b->getLocalPosition()); });
			for(; a < enemyList->rend(); ++a)
				if(util::circleCollision(a->get(), this,
				   std::max(a[0]->getWidth(), a[0]->getDepth()) * .5f,
				   std::max(this->getWidth(), this->getHeight()) * 2))
				{

					if(!AudioPlayer::createAudio("sfx/kick.wav", "kick"))
						OutputDebugStringA("Audio Not Playing");
					AudioPlayer::play();

					daOne = (*a).get();
					break;
				}

			if(daOne)
			{
				Projectile& proj = bullets.getNewObject();

				proj.create(*Projectile::referenceObj);

				proj.translate(this->getLocalPosition());
				proj.setEnemyList(enemyList);
				proj.setEnemy(daOne);// = ;//enemy destination
				proj.setActive(true);
				proj.setDamage(50);

				Game::addModel(&proj);
			}
		}
	}


};

class EigthTower: public Tower
{
protected:
	int restCount = 0;

public:
	EigthTower(uint projectileLimit = 0):Tower(projectileLimit) { init(); };
	EigthTower(Tower& model, cstring tag = ""):Tower(model, 10, tag) { init(); }

	void init()
	{
		setColour(255.0f / 255, 255.0f / 255, 0);
		//	m_size = {100,100};
	}

	void update(float dt)
	{
		for(auto p : bullets.getObjectList())
			p->update(dt);

		timer = AudioPlayer::getTimePosition(0) * .001;

		if((beatOn = (int)(timer / (m_bps * 0.25)) + 1) != lastBeatOn)
		{
			lastBeatOn = beatOn;
			if(restCount++ == 2)
			{
				restCount = 2;
				if(beatOn % 4)
					restCount = 0;

				Model* daOne = nullptr;
				auto a = enemyList->rbegin();

				std::sort(enemyList->rbegin(), enemyList->rend(),
						  [&](std::shared_ptr<Enemy> a, std::shared_ptr<Enemy> b)->bool
				{return (getLocalPosition() - a->getLocalPosition()) < (getLocalPosition() - b->getLocalPosition()); });
				for(; a < enemyList->rend(); ++a)
					if(util::circleCollision(a->get(), this,
					   std::max(a[0]->getWidth(), a[0]->getDepth()) * .5f,
					   std::max(this->getWidth(), this->getHeight()) * 2))
					{

						if(!AudioPlayer::createAudio("sfx/short hat.wav", "kick"))
							OutputDebugStringA("Audio Not Playing");
						AudioPlayer::play();

						daOne = (*a).get();
						break;
					}

				if(daOne)
				{
					Projectile& proj = bullets.getNewObject();

					proj.create(*Projectile::referenceObj);
					proj.translate(this->getLocalPosition());
					proj.setEnemyList(enemyList);
					proj.setEnemy(daOne);// = ;//enemy destination
					proj.setActive(true);
					proj.setDamage(25);

					Game::addModel(&proj);
				}
			}
		}
	}

};

class WholeTower: public Tower
{
public:
	WholeTower(uint projectileLimit = 0):Tower(projectileLimit) { init(); };
	WholeTower(Tower& model, cstring tag = ""):Tower(model, 10, tag) { init(); }

	void init()
	{
		setColour(255.0f / 255, 128.0f / 255, 0);

		//m_size = {100,100};
	}

	void update(float dt)
	{
		for(auto p : bullets.getObjectList())
			p->update(dt);

		timer = AudioPlayer::getTimePosition(0) * .001;


		if((beatOn = (int)(timer / m_bps) + 2) != lastBeatOn)
		{
			lastBeatOn = beatOn;
			if(!(beatOn % 4))
			{


				Model* daOne = nullptr;
				auto a = enemyList->rbegin();

				std::sort(enemyList->rbegin(), enemyList->rend(),
						  [&](std::shared_ptr<Enemy> a, std::shared_ptr<Enemy> b)->bool{return (getLocalPosition() - a->getLocalPosition()) < (getLocalPosition() - b->getLocalPosition()); });
				for(; a < enemyList->rend(); ++a)
					if(util::circleCollision(a->get(), this,
					   std::max(a[0]->getWidth(), a[0]->getDepth()) * .5f,
					   std::max(this->getWidth(), this->getHeight()) * 2))
					{

						if(!AudioPlayer::createAudio("sfx/clap.wav", "kick"))
							OutputDebugStringA("Audio Not Playing");
						AudioPlayer::play();

						daOne = (*a).get();
						break;
					}

				if(daOne)
				{
					Projectile& proj = bullets.getNewObject();

					proj.create(*Projectile::referenceObj);
					proj.translate(this->getLocalPosition());
					proj.setEnemyList(enemyList);
					proj.setEnemy(daOne);// = ;//enemy destination
					proj.setActive(true);
					proj.setDamage(100);

					Game::addModel(&proj);
				}
			}
		}
	}

};
