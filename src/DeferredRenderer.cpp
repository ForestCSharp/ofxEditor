#include "DeferredRenderer.h"

#include "ofFbo.h"
#include "ofShader.h"
#include "ofGraphics.h"
#include <string>

#define STRINGIFY(x) #x

namespace ofxDeferredRenderer
{
	ofFbo GBufferFBO;
	ofShader GBufferShader;
	ofShader LightingShader;


	void Setup(int Width, int Height)
	{
		ofDisableArbTex();

		ofFbo::Settings GBufferSettings;
		GBufferSettings.minFilter = GL_NEAREST;
		GBufferSettings.maxFilter = GL_NEAREST;
		GBufferSettings.width = Width;
		GBufferSettings.height = Height;
		GBufferSettings.numColorbuffers = 4;
		GBufferSettings.useDepth = true;
		GBufferSettings.useStencil = true;
		GBufferSettings.depthStencilAsTexture = true;

		GBufferFBO.allocate(GBufferSettings);

		std::string GBufferVertShader = STRINGIFY(
			#version 400 \n

			uniform mat4 modelMatrix; //oF Default
			uniform mat4 modelViewMatrix; // oF Default
			uniform mat4 modelViewProjectionMatrix; // oF Default

			layout (location = 0) in vec4 position; //oF Default
			layout (location = 2) in vec3 normal; //oF Default
			layout (location = 3) in vec2 texcoord; //oF Default

			//Custom Uniforms 
			uniform float farClip;
			uniform float nearClip;

			out vec4 vWorldPosition;
			out float vDepth;
			out vec3 vNormal;
			out vec2 vTexCoord;

			void main() {

				gl_Position = modelViewProjectionMatrix * position;
				vec4 viewPos = modelViewMatrix * position;

				vWorldPosition = modelMatrix * position;
				vDepth = -viewPos.z / (farClip - nearClip);

				vNormal = normalize( vec3( modelMatrix * vec4( normal, 0.0 ) ) );

				vTexCoord = texcoord;
			}
		);

		std::string GBufferFragShader = STRINGIFY(
			#version 400 \n

			in vec4 vWorldPosition;
			in float vDepth;
			in vec3 vNormal;
			in vec2 vTexCoord;

			//TODO: Bools to switch between value / texture sample

			uniform vec4 AlbedoValue = vec4(1.0, 1.0, 1.0, 1.0);
			uniform sampler2D AlbedoTexture;

			uniform float MetallicValue = 0.0;
			uniform sampler2D MetallicTexture;

			uniform float RoughnessValue = 1.0;
			uniform sampler2D RoughnessTexture;

			uniform float NormalIntensity = 1.0;
			uniform sampler2D NormalTexture;     //TODO: Bool to use/not use normal map

			layout( location = 0 ) out vec4 Albedo;
			layout( location = 1 ) out vec4 MetallicRoughnessAO; //TODO: Store Specular and AO  in z,w-channels respectively
			layout( location = 2 ) out vec4 WorldPosition;
			layout( location = 3 ) out vec4 WorldNormal;

			void main() {

				Albedo = texture( AlbedoTexture, vTexCoord ); //TODO: Sample Albedo Texture
				MetallicRoughnessAO = vec4( texture( MetallicTexture, vTexCoord ).x, texture( RoughnessTexture, vTexCoord ).x, 0.0, 1.0 );
				WorldPosition = vWorldPosition;

				vec3 Normal = texture( NormalTexture, vTexCoord ).rgb;
				//Normal = normalize( Normal * 2.0 - 1.0 );
				WorldNormal = vec4( normalize( vNormal * Normal ), 1.0 );
			}
		);

		GBufferShader.setupShaderFromSource(GL_VERTEX_SHADER, GBufferVertShader);
		GBufferShader.setupShaderFromSource(GL_FRAGMENT_SHADER, GBufferFragShader);
		GBufferShader.linkProgram();

		//Lighting Shader
		
		std::string LightingFragShader = STRINGIFY(
			#version 400 \n

			#define MAX_NUMBER_OF_LIGHTS 10 \n

			struct Light {
				vec3 Position;
				vec3 Color;
				float Intensity;
			};

			Light Lights[MAX_NUMBER_OF_LIGHTS];

			uniform vec2 resolution;

			uniform sampler2D AlbedoTexture;
			uniform sampler2D MetallicRoughnessAO; //TODO: Store Specular and AO  in z,w-channels respectively
			uniform sampler2D WorldPosition;
			uniform sampler2D WorldNormal;

			layout( location = 0 ) out vec4 Color;

			void main() {

				Lights[0].Position = vec3( 1, 0, 0 );
				Lights[0].Color = vec3( 1, 1, 1 );
				Lights[0].Intensity = 2.0f;

				vec3 Position = texture( WorldPosition, gl_FragCoord.xy  * resolution ).xyz;
				vec3 Normal = texture( WorldNormal, gl_FragCoord.xy  * resolution ).xyz;

				vec3 LightVector = Lights[0].Position - Position;
				float LightDistance = length( LightVector );

				Color = vec4( Position / 3.0f , 1.0f );
				//Color = texture( AlbedoTexture, gl_FragCoord.xy  * resolution );
			}
		);
		
		LightingShader.setupShaderFromSource( GL_FRAGMENT_SHADER, LightingFragShader );
		LightingShader.linkProgram();
	}

	void RenderScene( std::set<ofxGameMesh*> Meshes, std::set<ofxGameLight*> Lights, ofCamera& Camera )
	{
		ofDisableArbTex();

		float ClearColor[4] = { 0.39f, 0.58f, 0.925f, 1.0f };

		//GBuffer Pass
		GBufferFBO.bind();
		//Note this has to be done INSIDE the bind
		GBufferFBO.activateAllDrawBuffers();
		ofEnableDepthTest();
		//Clear Color Targets and Depth Target
		ofClear( ofFloatColor( 0.39f, 0.58f, 0.925f, 1.0f ) );

		//TODO: Ensure all draw buffers are activated upon bind automatically
		//TODO: Ensure depth gets enabled/disabled in bind/unbind?

		ofDisableAlphaBlending();
		
		Camera.begin();

		//TODO: Make GBufferShader part of material so it can change per-object
		GBufferShader.begin();

		GBufferShader.setUniform1f( "nearClip", Camera.getNearClip() );
		GBufferShader.setUniform1f( "farClip", Camera.getFarClip() );

		for (auto& Mesh : Meshes)
		{
			//Todo: Have Shader Manage texture bindings (each "begin" resets to 0, and then just increment)

			GBufferShader.setUniformTexture( "AlbedoTexture", Mesh->AlbedoTexture, 0 );
			GBufferShader.setUniform4f( "AlbedoValue", Mesh->AlbedoValue );

			GBufferShader.setUniformTexture( "MetallicTexture", Mesh->MetallicTexture, 1 );
			GBufferShader.setUniform1f( "MetallicValue", Mesh->MetallicValue );

			GBufferShader.setUniformTexture( "RoughnessTexture", Mesh->RoughnessTexture, 2 );
			GBufferShader.setUniform1f( "RoughnessValue", Mesh->RoughnessValue );

			GBufferShader.setUniformTexture( "NormalTexture", Mesh->NormalTexture, 3 );
			GBufferShader.setUniform1f( "NormalIntensity", Mesh->NormalIntensity );

			Mesh->draw();
		}
		GBufferShader.end();

		Camera.end();
		ofDisableDepthTest();
		GBufferFBO.unbind();

		//Flip our textures
		for ( int i = 0; i < 4; ++i )
		{
			GBufferFBO.getTexture( i ).getTextureData().bFlipTexture = true;
		}

		//Lighting Pass
		LightingShader.begin();
		LightingShader.setUniform2f( "resolution", 1.f / GBufferFBO.getWidth(), 1.f / GBufferFBO.getHeight() );
		LightingShader.setUniformTexture( "AlbedoTexture", GBufferFBO.getTexture( 0 ), 0 );
		LightingShader.setUniformTexture( "MetallicRoughnessAO", GBufferFBO.getTexture( 1 ), 1 );
		LightingShader.setUniformTexture( "WorldPosition", GBufferFBO.getTexture( 2 ), 2 );
		LightingShader.setUniformTexture( "WorldNormal", GBufferFBO.getTexture( 3 ), 3 );
		ofDrawRectangle( -ofGetWidth() / 2, -ofGetHeight() / 2, ofGetWidth(), ofGetHeight() );
		LightingShader.end();

		//Testing Progress / Final Present

		//GBufferFBO.draw(0, 0);

		float w2 = ofGetWidth();
		float h2 = ofGetHeight();
		float ws = w2 * 0.25;
		float hs = h2 * 0.25;

		GBufferFBO.getTexture(1).draw(ws, hs * 3, ws, hs);
		GBufferFBO.getTexture(2).draw(ws * 2, hs * 3, ws, hs);
		GBufferFBO.getTexture(3).draw(ws * 3, hs * 3, ws, hs);
	}
}