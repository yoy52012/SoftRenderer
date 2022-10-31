#pragma once

#include <memory>
#include <iostream>

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
		glm::vec3 aPosition;
		glm::vec2 aTextureCoord;
		glm::vec3 aNormal;
		glm::vec3 aTangent;
	};

	struct BaseShaderUniforms
	{
		glm::mat4 uModelMatrix;
		glm::mat4 uModelViewProjectMatrix;
		glm::mat4 uInverseTransposeModelMatrix;

		glm::vec3 uLightPosition;
		glm::vec3 uCameraPostion;

		Sampler2D uAlbedoMap;
	};

	struct BaseShaderVaryings
	{
		glm::vec2 vTexCoord;
	};

	struct BaseShader
	{
	public:
		virtual void shaderMain() = 0;
	};

	class BaseVertexShader : BaseShader
	{
	public:
		// Built-in variables
		glm::vec4 gl_Position;

		// Custom variables
		BaseShaderAttributes* attributes = nullptr;
        BaseShaderUniforms*   uniforms = nullptr;
		BaseShaderVaryings*   varyings = nullptr;

		void shaderMain() override
		{
			auto* a = (BaseShaderAttributes*)attributes;
			auto* u = (BaseShaderUniforms*)uniforms;
			auto* v = (BaseShaderVaryings*)varyings;

			glm::vec4 position = glm::vec4(a->aPosition, 1.0f);

			gl_Position = u->uModelViewProjectMatrix * position;

			varyings->vTexCoord = a->aTextureCoord;
		}
	};

	class BaseFragmentShader : BaseShader
	{
	public:

		// inner input
		glm::vec4 gl_FragCoord;
		bool gl_FrontFacing;

		// inner output
		float gl_FragDepth;
		glm::vec4 gl_FragColor;
		bool discard = false;

		// Custom variables
        BaseShaderUniforms* uniforms = nullptr;
		BaseShaderVaryings* varyings = nullptr;

		void shaderMain() override
		{
			gl_FragDepth = gl_FragCoord.z;
		}
	};

	struct Program
	{
		std::shared_ptr<BaseShaderUniforms> uniforms;
		std::shared_ptr<BaseVertexShader> vertexShader;
		std::shared_ptr<BaseFragmentShader> fragmentShader;
	
		bool link()
		{
			if (vertexShader == nullptr)
			{
				std::cerr << "Program link error : Vertex shader is null." << std::endl;
				return false;
			}

			if (fragmentShader == nullptr)
			{
				std::cerr << "Program link error : Fragment shade is null." << std::endl;
				return false;
			}
				
			vertexShader->uniforms = uniforms.get();
			fragmentShader->uniforms = uniforms.get();

			return true;
		}

	};

}