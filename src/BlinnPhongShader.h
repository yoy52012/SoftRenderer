#pragma once
#include "Shader.h"

namespace SoftRenderer
{
    #define BlinnPhongShaderAttributes BaseShaderAttributes

    struct BlinnPhongShaderVaryings : BaseShaderVaryings
    {
        glm::vec2 textureCoord;
        glm::vec3 worldNormal;
        glm::vec3 cameraDirection;
        glm::vec3 lightDirection;
    };  

    struct BlinnPhongShaderUniforms : BaseShaderUniforms
    {
        BlinnPhongShaderUniforms() = default;
        ~BlinnPhongShaderUniforms() = default;

        glm::vec3 emmissiveColor = glm::vec3(0);
        glm::vec3 diffuseColor   = glm::vec3(0);
        glm::vec3 specularColor  = glm::vec3(0);
        float specularShininess  = 32.0f;
        float specularStrength   = 1.0f;

        Sampler2D emissiveMap;
        Sampler2D diffuseMap;
        Sampler2D normalMap;
        Sampler2D aoMap;
    };

    class BlinnPhongVertexShader : public BaseVertexShader
    {
    public:
        const glm::mat4& getModelMatrix(BlinnPhongShaderAttributes* attributes, BlinnPhongShaderUniforms* uniforms) const
        {
            return uniforms->modelMatrix;
        }

        void shaderMain() override
        {
            auto* a = (BlinnPhongShaderAttributes*)attributes;
            auto* u = (BlinnPhongShaderUniforms*)uniforms;
            auto* v = (BlinnPhongShaderVaryings*)varyings;

            glm::mat4 modelMatrix = u->modelMatrix;
            glm::mat4 modelViewProjMatrix = u->modelViewProjectMatrix;

            glm::vec4 localPosition = glm::vec4(a->position, 1.0f);
            glm::vec3 worldPosition = glm::vec3(modelMatrix * localPosition);

            gl_Position = u->modelViewProjectMatrix * localPosition;

            v->textureCoord = a->textureCoord;

            // world space
            v->worldNormal = glm::normalize(u->inverseTransposeModelMatrix * a->normal);
            v->cameraDirection = u->cameraPostion - worldPosition;
            v->lightDirection  = u->lightPosition - worldPosition;


            if (!u->normalMap.isEmpty())
            {
                //TBN
                glm::vec3 N = glm::normalize(u->inverseTransposeModelMatrix * a->normal);
                glm::vec3 T = glm::normalize(u->inverseTransposeModelMatrix * a->tangent);
                // Gram-Schmidt process re-orthogonalize T with respect to N
                T = glm::normalize(T - glm::dot(T, N) * N);
                glm::vec3 B = glm::cross(T, N);
                glm::mat3 TBN = glm::transpose(glm::mat3(T, B, N));

                // Transform to TBN space
                v->lightDirection = TBN * v->lightDirection;
                v->cameraDirection = TBN * v->cameraDirection;
            }    
        }

        CLONE_VERTEX_SHADER(BlinnPhongVertexShader)
    };

    class BlinnPhongFragmentShader : public BaseFragmentShader
    {
    public:
        BlinnPhongShaderUniforms* u = nullptr;
        BlinnPhongShaderVaryings* v = nullptr;
        
        glm::vec3 getNormal() const
        {
            if(u->normalMap.isEmpty())
            {
                return glm::normalize(v->worldNormal);
            }

            glm::vec3 normal = u->normalMap.texture2D(v->textureCoord);
            return glm::normalize(normal * 2.0f - 1.0f);
        }


            const float pointLightRangeInverse = 1.0f / 5.0f;
            const float specularShininess = 128.0f;
            const float specularStrength = 1.0f;

        float getDistanceAttenuation( const float lightDistance, const float cutoffDistance, const float decayExponent ) 
        {
        #if defined ( PHYSICALLY_CORRECT_LIGHTS )
            // based upon Frostbite 3 Moving to Physically-based Rendering
            // page 32, equation 26: E[window1]
            // https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
            float distanceFalloff = 1.0 / glm::max( glm::pow( lightDistance, decayExponent ), 0.01 );
            if ( cutoffDistance > 0.0 ) 
            {
                distanceFalloff *= glm::pow2( glm::clamp( 1.0f - glm::pow4( lightDistance / cutoffDistance ), 0.0f, 1.0f) );
            }
            return distanceFalloff;
        #else
            if ( cutoffDistance > 0.0 && decayExponent > 0.0f ) 
            {
                return glm::pow( glm::clamp( - lightDistance / cutoffDistance + 1.0f, 0.0f, 1.0f), decayExponent);
            }
            return 1.0f;
        #endif
        }

        glm::vec3 BRDF_Lambert( const glm::vec3& diffuseColor ) 
        {
	        return glm::one_over_pi<float>() * diffuseColor;
        }

        glm::vec3 F_Schlick( const glm::vec3& f0, const float f90, const float dotVH ) 
        {
            // Original approximation by Christophe Schlick '94
            // float fresnel = pow( 1.0 - dotVH, 5.0 );
            // Optimized variant (presented by Epic at SIGGRAPH '13)
            // https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
            float fresnel = glm::exp2(( - 5.55473f * dotVH - 6.98316f ) * dotVH );
            return f0 * ( 1.0f - fresnel ) + ( f90 * fresnel );
        }

        float G_BlinnPhong_Implicit( /* const float dotNL, const float dotNV */ ) 
        {
            // geometry term is (n dot l)(n dot v) / 4(n dot l)(n dot v)
            return 0.25f;
        }

        float D_BlinnPhong( const float shininess, const float dotNH ) 
        {
	        return glm::one_over_pi<float>() * ( shininess * 0.5f + 1.0f) * glm::pow( dotNH, shininess );
        }

        glm::vec3 BRDF_BlinnPhong( const glm::vec3& lightDir, const glm::vec3& viewDir, const glm::vec3& normal, const glm::vec3& specularColor, const float shininess ) 
        {
            glm::vec3 halfDir = normalize( lightDir + viewDir );
            float dotNH = glm::clamp(glm::dot( normal, halfDir ), 0.0f, 1.0f );
            float dotVH = glm::clamp(glm::dot( viewDir, halfDir ), 0.0f, 1.0f );
            glm::vec3 F = F_Schlick( specularColor, 1.0, dotVH );
            float G = G_BlinnPhong_Implicit( /* dotNL, dotNV */ );
            float D = D_BlinnPhong( shininess, dotNH );
            return F * ( G * D );
        }

        void shaderMain() override
        {

            BaseFragmentShader::shaderMain();

            gl_FragColor = glm::vec4(1.0);
            return;
            
            u = (BlinnPhongShaderUniforms*)uniforms;
            v = (BlinnPhongShaderVaryings*)varyings;

            glm::vec3 outDiffuseColor = glm::vec3(0.0f);
            glm::vec3 outSpecularColor = glm::vec3(0.0f);

            glm::vec2 uv = v->textureCoord;

            // ambient
            glm::vec3 ambientColor = glm::vec3(0);

            // ambient occlusion
            float ao = 1.0f;
            if(!u->aoMap.isEmpty())
            {
                ao = u->aoMap.texture2D(v->textureCoord).r;
            }

            // emissive
            glm::vec3 emissiveColor = u->emmissiveColor;
            if(!u->emissiveMap.isEmpty())
            {
                emissiveColor *= glm::vec3(u->emissiveMap.texture2D(uv));
            }

            // diffuse
            glm::vec3 diffuseColor = u->diffuseColor;
            if(!u->diffuseMap.isEmpty())
            {
                glm::vec4 sampledDiffuseColor = u->diffuseMap.texture2D(uv);
                diffuseColor *= glm::vec3(sampledDiffuseColor);
            }

            // specular
            glm::vec3 specularColor = u->specularColor;
            float specularStrength  = u->specularStrength;
            float specularShiness   = u->specularShininess;

            // light info
            glm::vec3 lightColor = u->lightColor;
            float lightDistance = u->lightDistance;
            float lightDecay = u->lightDecay;

            glm::vec3 L = glm::normalize(v->lightDirection);
            glm::vec3 N = glm::normalize(getNormal());
            glm::vec3 V = glm::normalize(v->cameraDirection);

            float lightAttenuation = getDistanceAttenuation(glm::length(L), lightDistance, lightDecay);

            // direct radiance
            float dotNL = glm::clamp(glm::dot( N, L ), 0.0f, 1.0f );
	        glm::vec3 directIrradiance = dotNL * lightColor * lightAttenuation;
            glm::vec3 directDiffuse  = directIrradiance * BRDF_Lambert(diffuseColor);
            glm::vec3 directSpecular = directIrradiance * BRDF_BlinnPhong(L, V, N, specularColor, specularShiness);

            // indirect radiance
            glm::vec3 indirectIrradiance = ambientColor;
            glm::vec3 indirectDiffuse = indirectIrradiance * BRDF_Lambert( diffuseColor );

            outDiffuseColor  = directDiffuse + indirectDiffuse;
            outSpecularColor = directSpecular;

            gl_FragColor = glm::vec4(diffuseColor + outSpecularColor + emissiveColor , 1.0f);
        }

        CLONE_FRAGMENT_SHADER(BlinnPhongFragmentShader)
    };

    class BlinnPhongProgram : public Program
    {
    private:
        size_t getVaringsCount() override
        {
            return sizeof(BlinnPhongShaderVaryings) / sizeof(float);
        }
    };
}