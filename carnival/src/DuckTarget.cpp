#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h> // For wglSwapInterval
#endif

#define FREEGLUT_STATIC
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Vectors.h"
#include "CubeMesh.h"
#include "DuckTarget.h"



// allow duck's position to be set
DuckTarget::DuckTarget(float x, bool flip)
{
	this->duckX = x;
	
	// if duck should be flipped initially
	if (flip) {
		this->spin = -180;
		this->leftToRight = false;
		this->rightToLeft = true;
	}
}

void DuckTarget::draw()
{
	glPushMatrix();
	glTranslatef(duckX, duckY, duckZ);
	glTranslatef(0, -2.5, 0);
	glRotatef(spin, 0.0, 0.0, 1.0);
	glTranslatef(0, 2.5, 0);
	glRotatef(flipAngle, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glScalef(0.5, 0.5, 0.5);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// Build Body 
	glPushMatrix();
	glScalef(targetWidth, targetLength, targetDepth);
	gluSphere(gluNewQuadric(), 1.0, 20, 20);
	glPopMatrix();

	// Build BullsEye
	glPushMatrix();
		// Position Bullseye wrt body
		glTranslatef(-0, 0, -1.05 * targetDepth);
		glPushMatrix();
		glRotatef(-90.0, 0.0, 1.0, 0.0);
		glScalef(0.08 * targetWidth, 0.5 * targetWidth, 0.5 * targetWidth);
		// draw bullseye
		// apply shaders to determine which pixels should be shaded in
		glUseProgram(shaderProgram);
		gluSphere(gluNewQuadric(), 1.0, 20, 20);
		// detach shaders
		glUseProgram(0);
		glPopMatrix();

		// Matrix to store ModelView Matrix 
		float mv[16];
		// Get the ModelView Matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, mv);

		// Get x, y, and z coordinates respectively from the matrix so the target's position is stored as a value always in world coordinates
		targetWorldCoords.x = mv[12];
		targetWorldCoords.y = mv[13];
		targetWorldCoords.z = mv[14];


	glPopMatrix(); // end Bulls Eye


	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// Build Neck
	glPushMatrix();
	  // Position neck wrt body
	  glTranslatef(-0.55 * targetWidth, 0.3 * targetLength, 0.05 * targetDepth);
	  // Neck
	  glPushMatrix();
	  glRotatef(65.0, 0.0, 0.0, 1.0);
	  glRotatef(90.0, 0.0, 1.0, 0.0);
	  glScalef(0.2 * targetWidth, 0.45 * targetWidth, 1.95 * targetDepth);
	  gluCylinder(gluNewQuadric(), 0.8, 0.8, 2.0, 20, 20);
	  glPopMatrix();
	glPopMatrix();

	// Build Head	
	glPushMatrix();
	  // Position head wrt body
	  glTranslatef(-0.3 * targetWidth, 1.5 * targetLength, 0.05 * targetDepth);

	  // Head
	  glPushMatrix();
	  glScalef(1.05 * 0.5 * targetWidth, 1.05 * 0.5 * targetWidth, targetDepth);
	  gluSphere(gluNewQuadric(), 1.0, 20, 20);
	  glPopMatrix();

	  // Beak (position wrt to head)
	  glMaterialfv(GL_FRONT, GL_AMBIENT, beakmat_ambient);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, beakmat_specular);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, beakmat_diffuse);
	  glMaterialfv(GL_FRONT, GL_SHININESS, beakmat_shininess);
	  glPushMatrix();
	    glTranslatef(-0.1 * targetWidth, -0.1 * targetLength, 0);
	    glRotatef(-10.0, 0.0, 0.0, 1.0);
	    glRotatef(-90.0, 0.0, 1.0, 0.0);
	    glScalef(0.3 * targetWidth, 0.5 * targetWidth, 1.25 * targetDepth);
	    gluCylinder(gluNewQuadric(), 0.8, 0.1, 2.0, 20, 20);
	  glPopMatrix();

	glPopMatrix(); // end Head and Beak

	// Build Tail
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glPushMatrix();
	  // Position tail wrt body
	  glTranslatef(0.7 * targetWidth, 0.3 * targetLength, 0);

	  // Tail
	  glPushMatrix();
	  glRotatef(45.0, 0.0, 0.0, 1.0);
	  glRotatef(90.0, 0.0, 1.0, 0.0);
	  glScalef(0.3 * targetWidth, 0.5 * targetWidth, 1.25 * targetDepth);
	  gluCylinder(gluNewQuadric(), 0.8, 0.2, 2.0, 20, 20);
	  glPopMatrix();
	glPopMatrix();
  glPopMatrix();
}


void DuckTarget::flip()
{
	flipped = true;
}

void DuckTarget::animate(bool wave)
{
	if (leftToRight)
	{
		duckX += 0.05f;
		if (wave)
		  duckY += 0.1 * sin(3.14159265 / 2.0 * (duckX - -8.0));
		if (duckX >= 8.0)
		{
			turn1 = true;
			leftToRight = false;
			duckX = 8.0;
			duckY = -0.5;
		}
	}
	else if (turn1)
	{
		spin -= 1.0;
		if (spin <= -180.0)
		{
			spin = -180.0;
			duckX = 8.0;
			rightToLeft = true;
			turn1 = false;
		}
	}
	else if (rightToLeft)
	{
		duckX -= 0.05f;
		if (duckX <= -8.0)
		{
			turn2 = true;
			rightToLeft = false;
			duckX = -8.0;
		}
	}
	else if (turn2)
	{
		spin -= 1.0;
		if (spin <= -360.0)
		{
			spin = 0;
			duckX = -8.0;
			leftToRight = true;
			turn2 = false;
		}
	}
	if (flipped)
	{
		if (leftToRight)
		{
			flipAngle -= 5.0;
			if (flipAngle < -90)
				flipAngle = -90;
		}
		// reset it
		else if (rightToLeft)
		{
			flipAngle += 5.0;
			if (flipAngle >= 0.0)
			{
				flipAngle = 0;
				flipped = false;
			}
		}

	}
}

bool DuckTarget::hit(Vector3 bulletCoords)
{
	// if not flipped, ex. already hit
	if (!flipped) {
		// if duck is within the outer range of bullseye, ex. within the circle x^2 + y^2 = r^2 & its z coordinates are within duck's Z (tolerance = -1/1), then its a hit
		return pow((bulletCoords.x - targetWorldCoords.x), 2) + pow((bulletCoords.y - targetWorldCoords.y), 2) < pow(targetRadius, 2) && (bulletCoords.z >= (targetWorldCoords.z - 1.0f) && bulletCoords.z <= (targetWorldCoords.z + 1.0f));
	}
	return false;
}

void DuckTarget::getShaders(GLuint shaderProgram) {
	// set the shader program (bullsEye shader) so DuckTarget can access it
	this->shaderProgram = shaderProgram;
}