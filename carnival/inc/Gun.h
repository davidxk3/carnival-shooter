class Gun 
{
private:
	float gunX = 0;
	float gunY = 0;
	float gunZ = 0;
	// Material properties for drawing

public:
	Gun(float x = 0, float y = 0, float z = 0);
	void Gun::draw();
	void Gun::animate();
};



