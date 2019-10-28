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
	transform = glm::rotate(transform, glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	glm::mat4 trans = glm::mat4(1.0f);	trans = glm::rotate(topLeft->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	topLeft->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(topMid->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	topMid->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(topRight->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	topRight->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(midLeft->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	midLeft->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(midRight->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	midRight->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(bottomLeft->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	bottomLeft->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(bottomMid->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	bottomMid->SetTransform(trans);	trans = glm::mat4(1.0f);	trans = glm::rotate(bottomRight->GetTransform(), glm::radians(msec / 10.0f), glm::vec3(0.0, 1.0, 0.0));	bottomRight->SetTransform(trans);
}