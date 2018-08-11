#pragma once

class ofxPostProcessEffect
{
public:

	//TODO: Set Vertex Shader

	void SetSize(int NewWidth, int NewHeight)
	{
		PostProcessFbo.allocate(NewWidth, NewHeight, GL_RGBA);
	}

	void SetFragmentShader(std::string FragmentShaderSource)
	{
		PostProcessShader.setupShaderFromSource(GL_FRAGMENT_SHADER, FragmentShaderSource);
		PostProcessShader.linkProgram();
	}

	ofFbo& Process(ofFbo& InputFbo)
	{
		ofDisableArbTex();

		PostProcessFbo.bind();
		PostProcessShader.begin();

		//TODO: Remove shader specifics, have a callback to set those up (differs per post process effect)
		{
			//Basic Testing Shader
			PostProcessShader.setUniform1f("windowWidth", ofGetWidth());
			PostProcessShader.setUniform1f("windowHeight", ofGetHeight());

			//Fxaa Testing shader
			PostProcessShader.setUniform2f("resolution", 1.f / PostProcessFbo.getWidth(), 1.f / PostProcessFbo.getHeight());
		}

		//Used by all Post Process Shaders
		PostProcessShader.setUniformTexture("InputTexture", InputFbo.getTexture(), 0);

		ofDrawRectangle(-ofGetWidth() / 2, -ofGetHeight() / 2, ofGetWidth(), ofGetHeight());

		PostProcessShader.end();
		PostProcessFbo.unbind();

		return PostProcessFbo;
	}

	ofFbo& GetFbo()
	{
		return PostProcessFbo;
	}

	void Draw()
	{
		PostProcessFbo.getTexture().getTextureData().bFlipTexture = true;
		PostProcessFbo.draw(0, 0);
	}

	ofFbo PostProcessFbo;
	ofShader PostProcessShader;
};