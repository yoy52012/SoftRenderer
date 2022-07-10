#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace SoftRenderer
{
	class Transform
	{
	public:
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		

		bool operator==(const Transform& rhs)
		{
			return this->position == rhs.position && this->rotation == rhs.rotation && this->scale == rhs.scale;
		}

		bool operator!=(const Transform& rhs)
		{
			return this->position != rhs.position || this->rotation != rhs.rotation || this->scale != rhs.scale;
		}

		glm::mat4 getMatrix()
		{
			glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 rotationMat = glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
			glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

			return translateMat * rotationMat * scaleMat;
		}
	};
}