#include "CubeSlice.h"



CubeSlice::CubeSlice(glm::vec3 centerPos, int plane)
{
	SceneNode* center = new SceneNode();
	glm::mat4 matrix = glm::mat4(1.0f);
	matrix = glm::translate(matrix, centerPos);
	center->SetTransform(matrix);
	AddChild(center);

	topLeft = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y+2.1f, centerPos.z + 2.1f) );
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x - 2.1f, centerPos.y, centerPos.z - 2.1f) );
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x - 2.1f, centerPos.y+2.1f, centerPos.z) );
		break;
	}
	center->SetTransform(matrix);
	AddChild(topLeft);

	topMid = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y + 2.1f, centerPos.z));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y, centerPos.z - 2.1f));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y + 2.1f, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(topMid);

	topRight = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y + 2.1f, centerPos.z - 2.1f));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x + 2.1f, centerPos.y, centerPos.z - 2.1f));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x + 2.1f, centerPos.y + 2.1f, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(topRight);

	midLeft = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y, centerPos.z + 2.1f));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x - 2.1f, centerPos.y, centerPos.z));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x - 2.1f, centerPos.y, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(midLeft);

	midRight = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y, centerPos.z - 2.1f));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x + 2.1f, centerPos.y, centerPos.z));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x + 2.1f, centerPos.y, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(midRight);

	bottomLeft = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y - 2.1f, centerPos.z + 2.1f));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x - 2.1f, centerPos.y, centerPos.z + 2.1f));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x - 2.1f, centerPos.y - 2.1f, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(bottomLeft);

	bottomMid = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y - 2.1f, centerPos.z));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y, centerPos.z + 2.1f));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y - 2.1f, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(bottomMid);

	bottomRight = new SceneNode();
	matrix = glm::mat4(1.0f);
	switch (plane)
	{
	case 0:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x, centerPos.y - 2.1f, centerPos.z - 2.1f));
		break;
	case 1:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x + 2.1f, centerPos.y, centerPos.z + 2.1f));
		break;
	case 2:
		matrix = glm::translate(matrix, glm::vec3(centerPos.x + 2.1f, centerPos.y - 2.1f, centerPos.z));
		break;
	}
	center->SetTransform(matrix);
	AddChild(bottomRight);

}

//
//CubeSlice::~CubeSlice()
//{
//}

void CubeSlice::Update(float msec)
{
	transform = glm::mat4(1.0f);
	transform = glm::rotate(transform, glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));
}