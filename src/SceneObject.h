#pragma once

#include "Transform.h"

namespace SoftRenderer
{
	using SceneId = unsigned int;

	class SceneObject
	{
	public:
		SceneObject() = default;
		SceneObject(SceneId id)
		SceneObject(const SceneObject& other) = delete;
		SceneObject(SceneObject&& other) = default;
		~SceneObject() = default;

		SceneObject& operator=(const SceneObject& other) = default;

		SceneId getId() const;


		Transform transform;
	private:
	};


}