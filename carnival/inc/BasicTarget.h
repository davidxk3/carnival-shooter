class BasicTarget
{
private:
	
	// Basic Stuff
	float targetX = -8.0;
	float targetY = -0.5;
	float targetZ = -8.0;
	float targetWidth = 4.0;
	float targetLength = 3.0;
	float targetDepth = 1.0;
	
	bool turn1 = false;
	bool turn2 = false;
	bool leftToRight = true;
	bool rightToLeft = false;
	bool flipped = false;
	float flipAngle = 0;
	float spin = 0;

	// Material properties for drawing
	float mat_ambient[4] = { 0.957f, 0.74f, 0.047f, 1.0f };
	float mat_diffuse[4] = { 0.957f, 0.74f, 0.047f, 1.0f };
	float mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float mat_shininess[1] = { 100.0F };

	float bullseye_ambient[4] = { 0.851f, 0.118f, 0.161f, 1.0f };
	float bullseye_diffuse[4] = { 0.851f, 0.118f, 0.161f, 1.0f };
	float bullseye_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float bullseye_shininess[1] = { 100.0F };

	float bullseyeInner_ambient[4] = { 0.929f, 0.929f, 0.929f, 1.0f };
	float bullseyeInner_diffuse[4] = { 0.929f, 0.929f, 0.929f, 1.0f };
	float bullseyeInner_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float bullseyeInner_shininess[1] = { 100.0F };
private:
	

public:

	BasicTarget();
	void BasicTarget::draw();
	void BasicTarget::animate(bool wave);
	void BasicTarget::flip();
};



