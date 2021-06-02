#include "Shader.h"

namespace SoftRenderer
{
	Shader::VertexData Shader::vertexShader(const Vertex& iv)
	{
		VertexData o = {};
		o.worldPosition = mModelMatrix * glm::vec4(iv.position, 1.0f);
		//o.worldNormal = mModelMatrix * glm::viv.normal;
		o.clipPostion = mProjMatrix * mViewMatrix * o.worldPosition;
		o.texcoord = iv.texcoord;
		o.color = iv.color;

		return o;
	}

	glm::vec4 Shader::fragmentShader(const Shader::VertexData& v2f)
	{
		glm::vec4 color;
		//color = v2f.color;
		color = mUniforms.albedo->sampleRepeat(v2f.texcoord);
		return color;
	}
}