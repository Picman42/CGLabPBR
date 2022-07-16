#pragma once

#include <memory>
#include <string>

#include "../base/application.h"
#include "../base/model.h"
#include "../base/light.h"
#include "../base/shader.h"
#include "../base/texture.h"
#include "../base/camera.h"
#include "../base/skybox.h"


enum class RenderMode {
	Simple, FBR
};

enum class RenderScene {
	Geometry, Extintor
};


class Object {
public:
	std::string objPath;
	std::shared_ptr<Model> model;
	std::string texPathAlbedo, texPathRoughness, texPathMetallic, texPathNormal, texPathAO;
	std::shared_ptr<Texture> _texAlbedo, _texNormal, _texMetallic, _texRoughness, _texAO;
	bool _showTexAlbedo, _showTexNormal, _showTexMetallic, _showTexRoughness, _showTexAO;

	glm::vec3 Albedo = { 1.0f, 1.0f, 1.0f };
	float Roughness = 0.0f;
	float Metallic = 0.0f;
	std::string Name = "DefaultObject";

	bool hidden = false;

	Object() {}
	Object(std::string path_model, std::string name = "DefaultObject",
		std::string path_albedo = "", std::string path_normal = "", std::string path_roughness = "",
		std::string path_metallic = "", std::string path_ao = "");

	virtual void SetPosition(float x, float y, float z);
	virtual void SetScale(float x, float y, float z);
	virtual void SetRotation(glm::quat rotation);
	virtual void SetRotation(glm::vec3 axis, float angle);
	virtual std::shared_ptr<Model> GetModel() const;
	virtual glm::vec3 GetPosition() const;
	virtual glm::vec3 GetScale() const;
	virtual glm::quat GetRotation() const;

	virtual void Render(std::shared_ptr<Shader> shader, RenderMode render_mode, float delta_time, std::shared_ptr<Texture> texture);

	int ObjectType = 0;
	int index_2048 = -1;
};

class ObjectSequence : public Object
{
public:
	int frameNumber = 0;
	float currentTime = 0.0;
	int FPS = 24;
	int currentFrame = 0;
	std::vector< std::shared_ptr<Model> > models;
	ObjectSequence() : Object() {}
	ObjectSequence(std::string path_model, int frame_num, int fps=24, std::string name = "DefaultObject",
		std::string path_albedo = "", std::string path_normal = "", std::string path_roughness = "",
		std::string path_metallic = "", std::string path_ao = "");

	virtual void Render(std::shared_ptr<Shader> shader, RenderMode render_mode, float delta_time, std::shared_ptr<Texture> texture) override;
	void SetPosition(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void SetRotation(glm::quat rotation);
	void SetRotation(glm::vec3 axis, float angle);
	std::shared_ptr<Model> GetModel() const;
	glm::vec3 GetPosition() const;
	glm::vec3 GetScale() const;
	glm::quat GetRotation() const;

};

class TextureMapping : public Application {
public:
	TextureMapping();
	
	~TextureMapping();

	void ParseArguments(int argc, char* argv[]);

private:
	std::vector<Object*> _objects;
	
	//std::unique_ptr<Model> _extintor;

	enum RenderMode _renderMode = RenderMode::FBR;
	enum RenderScene _renderScene = RenderScene::Extintor;

	std::shared_ptr<Shader> _simpleShader;

	std::shared_ptr<Shader> _FBRShader;
	glm::vec3 _albedo = { 1.0f, 1.0f, 1.0f };
	float _roughness = 0.0f;
	float _metallic = 0.0f;


	std::string _pathModel, _pathAlbedo, _pathNormal, _pathMetallic, _pathRoughness, _pathAO;
	std::unique_ptr<Texture> _texAlbedo, _texNormal, _texMetallic, _texRoughness, _texAO, _texWhite, _texBlack, _texBlue;
	bool _showTexAlbedo, _showTexNormal, _showTexMetallic, _showTexRoughness, _showTexAO;

	std::unique_ptr<SkyBox> _skybox;

	std::unique_ptr<PerspectiveCamera> _camera;

	std::unique_ptr<DirectionalLight> _directionalLight;
	std::unique_ptr<SpotLight> _spotLight;

	bool _firstFrame = true;

	void initSimpleShader();

	void initFBRShader();

	void handleInput() override;

	void renderFrame() override;

	std::shared_ptr<Texture> _texAlbedoList[16];
	std::vector<Object*> _2048bricks;
	int number[6][6];
	int tar[6][6];
	int score = 0;
	int max_score;
	void addBrick();
	void update();
	void handle_moving(int direct);

	void takeScreenshot();

	void exportObject(Object* obj);
};