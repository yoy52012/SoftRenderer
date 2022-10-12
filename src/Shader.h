#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "MathUtils.h"
#include "Mesh.h"
#include "Texture.h"

namespace SoftRenderer
{
	class Shader
	{
	public:
		using Ptr = std::shared_ptr<Shader>;

		struct VertexData
		{
			glm::vec4 worldPosition;
			glm::vec3 worldNormal;
			glm::vec2 texcoord;
			glm::vec4 color;
			glm::vec4 clipPostion;
			glm::vec2 screenPosition;
			float clipW;

			static VertexData lerp(const VertexData& v1, const VertexData& v2, float f)
			{
				VertexData o = {};
				o.worldPosition = glm::lerp(v1.worldPosition, v2.worldPosition, f);
				o.worldNormal = glm::lerp(v1.worldNormal, v2.worldNormal, f);
				o.texcoord = glm::lerp(v1.texcoord, v2.texcoord, f);
				o.color = glm::lerp(v1.color, v2.color, f);
				o.clipPostion = glm::lerp(v1.clipPostion, v2.clipPostion, f);
				o.screenPosition = glm::lerp(v1.screenPosition, v2.screenPosition, f);

				return o;
			}

			static VertexData barycentricLerp(const VertexData& v0, const VertexData& v1, const VertexData& v2, const glm::vec3& w)
			{
				VertexData o = {};
				o.worldPosition = w.x * v0.worldPosition + w.y * v1.worldPosition + w.z * v2.worldPosition;
				o.worldNormal = w.x * v0.worldNormal + w.y * v1.worldNormal + w.z * v2.worldNormal;
				o.texcoord = w.x * v0.texcoord + w.y * v1.texcoord + w.z * v2.texcoord;
				o.color = w.x * v0.color + w.y * v1.color + w.z * v2.color;
				o.screenPosition = w.x * v0.screenPosition + w.y * v1.screenPosition + w.z * v2.screenPosition;
				o.clipW = w.x * v0.clipW + w.y * v1.clipW + w.z * v2.clipW;
				return o;
			}
		};


		struct Uniforms
		{
			Sampler2D albedoMap;
		};

	public:
		Shader() = default;
		~Shader() = default;

		Shader& operator= (const Shader& shader)
		{
			mModelMatrix = shader.mModelMatrix;
			mViewMatrix = shader.mViewMatrix;
			mProjMatrix = shader.mProjMatrix;

			return *this;
		}

		void setModelMatrix(const glm::mat4& modelMat)
		{
			mModelMatrix = modelMat;
		}

		void setViewMatrix(const glm::mat4& viewMat)
		{
			mViewMatrix = viewMat;
		}

		void setProjMatrix(const glm::mat4& projMat)
		{
			mProjMatrix = projMat;
		}

		virtual VertexData vertexShader(const Vertex& iv);

		virtual glm::vec4 fragmentShader(const VertexData& v2f);

		Uniforms mUniforms;


	protected:
		glm::mat4 mModelMatrix = glm::mat4(1.0f);
		glm::mat4 mViewMatrix = glm::mat4(1.0f);
		glm::mat4 mProjMatrix = glm::mat4(1.0f);

	};


	struct BaseShaderAttributes
	{
		glm::vec3 position;
		glm::vec2 textureCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct BaseShaderUniforms
	{
		glm::mat4 modelMatrix;
		glm::mat4 modelViewProjectMatrix;
		glm::mat4 inverseTransposeModelMatrix;

		glm::vec3 lightPosition;
		glm::vec3 cameraPostion;
	};

	struct BaseShaderVaryings
	{

	};

	class BaseShader
	{
	public:
		BaseShaderUniforms* uniforms = nullptr;
		
		virtual void shaderMain() = 0;
	};

	class BaseVertexShader : BaseShader
	{
	public:
		// Built-in variables
		glm::vec4 gl_Postion;

		// Custom variables
		BaseShaderAttributes* attributes;
		BaseShaderVaryings*   varyings;

		void shaderMain() override
		{
			auto* a = (BaseShaderAttributes*)attributes;
			auto* u = (BaseShaderUniforms*)uniforms;
			auto* v = (BaseShaderVaryings*)varyings;

			glm::vec4 position = glm::vec4(a->position, 1.0f);
			gl_Postion = u->modelViewProjectMatrix * position;

		}
	};

	class BaseFragmentShader : BaseShader
	{
	public:

		// Built-in variables
		glm::vec4 gl_FragCoord;
		glm::vec4 gl_FragColor;
		bool gl_FrontFacing;
		float gl_FragDepth;

		// Custom variables
		BaseShaderVaryings* varyings;

		void shaderMain() override
		{
			gl_FragDepth = gl_FragCoord.z;
		}
	};

	struct ShaderContext
	{
		std::shared_ptr<BaseVertexShader> vertexShader;
		std::shared_ptr<BaseFragmentShader> fragmentShader;
	};

}