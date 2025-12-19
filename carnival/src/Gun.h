#include "Vectors.h"

class Gun {
private:
	float gunX = 0;
	float gunY = 0;

	float bulletX = 0;
	float bulletY = 0;
	float bulletAngle = 0;

	const float trajectoryStart = 15.0f;		// starting position of bullet trajectory
	float trajectory = 0.0f;					// current bullet trajectory offset
	const float maxDistance = 30.0f;			// max distance bullet can travel
	const float trajectoryIncrease = 0.8f;		// increase bullet trajectory each frame
	float theta = 0.0f;							// angle of gun to mimic swiveling arm 
	const float M_PI = 3.14159265358979323846;

	// bounds
	const float upperY = 2.5f;
	const float lowerY = -1.0f;
	const float upperX = 2.2f;
	const float lowerX = -2.2f;

	bool inMotion = false;

	// bullet's world coordinates
	float bulletWorldX = 0.0f;
	float bulletWorldY = 0.0f;
	float bulletWorldZ = 0.0f;


	// Material properties for drawing
	float gun_ambient[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
	float gun_diffuse[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
	float gun_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float gun_shininess[1] = { 32.0F };

	float bullet_ambient[4] = { 0.878f, 0.129f, 0.153f, 1.0f };
	float bullet_diffuse[4] = { 0.878f, 0.129f, 0.153f, 1.0f };
	float bullet_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float bullet_shininess[1] = { 100.0F };

public:
	Gun();
	void Gun::draw();
	void Gun::shoot();
	void Gun::moveGun(float x, float y);
	void Gun::moveBullet(); 
	float getGunX() { return gunX; }
	float getGunY() { return gunY; }
	bool isInMotion() { return inMotion; }

	// getter for world coordinates (bullet)
	Vector3 getBulletWorldCoords() { return Vector3(bulletWorldX, bulletWorldY, bulletWorldZ); }

	// draw laser for gun
	void drawLaser(GLuint laserShaders);
};