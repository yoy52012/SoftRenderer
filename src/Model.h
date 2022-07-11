#pragma once

#include <string>

#include "SceneObject.h"

namespace SoftRenderer
{
	class Model
	{
	public:
		Model();
		~Model();

		bool loadFromFile(const std::string& filepath);


	private:

	};


}