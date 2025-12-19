class Bullet {
private:
	float bulletX = 0;
	float bulletY = 0;
	float bulletZ = 5.0f; // initial starting 

	float trajectory = 0.0f; // current trajectory it has travelled
	const float trajectoryIncrease = 0.5f; // how much it moves each time
	const float maxDistance = 30.0f; // max distance it can go 

	// Material properties for drawing
	float bullet_ambient[4] = { 0.878f, 0.129f, 0.153f, 1.0f };
	float bullet_diffuse[4] = { 0.878f, 0.129f, 0.153f, 1.0f };
	float bullet_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float bullet_shininess[1] = { 100.0F };

public:
	// constructor
	Bullet();

	// draw bullet
	void draw();

	// move bullet 
	void moveBullet();
};