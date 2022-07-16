#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>
#include "time.h"

#include "texture_mapping.h"

const std::string modelPath = "../data/ext/Extintor.obj";

const std::string texPathAlbedo = "../data/ext/Extintor_BaseColor.png";
const std::string texPathRoughness = "../data/ext/Extintor_Roughness.png";
const std::string texPathMetallic = "../data/ext/Extintor_Metallic.png";
const std::string texPathNormal = "../data/ext/Extintor_Normal.png";
const std::string texPathAO = "../data/ext/Extintor_AO.png";
const std::string texPathWhite = "../data/ext/Extintor_White.png";
const std::string texPathBlack = "../data/ext/Extintor_Black.png";
const std::string texPathBlue = "../data/ext/Extintor_Blue.png";

const std::vector<std::string> skyboxTexturePaths = {
	"../data/starfield/Left_Tex.jpg",
	"../data/starfield/Right_Tex.jpg",
	"../data/starfield/Down_Tex.jpg",
	"../data/starfield/Up_Tex.jpg",
	"../data/starfield/Front_Tex.jpg",
	"../data/starfield/Back_Tex.jpg"
};

Object::Object(std::string path_model, std::string name,
	std::string path_albedo, std::string path_normal, std::string path_roughness,
	std::string path_metallic, std::string path_ao): 
	objPath(path_model), Name(name),
	texPathAlbedo(path_albedo), texPathNormal(path_normal), texPathRoughness(path_roughness),
	texPathMetallic(path_metallic), texPathAO(path_ao)
{
	if (path_model != "")
	{
		model.reset(new Model(path_model));
	}
	if (path_albedo != "")
	{
		_texAlbedo.reset(new Texture2D(path_albedo));
	}
	else _showTexAlbedo = false;
	if (path_normal != "")
	{
		_texNormal.reset(new Texture2D(path_normal));
	}
	else _showTexNormal = false;
	if (path_roughness != "")
	{
		_texRoughness.reset(new Texture2D(path_roughness));
	}
	else _showTexRoughness = false;
	if (path_metallic != "")
	{
		_texMetallic.reset(new Texture2D(path_metallic));
	}
	else _showTexMetallic = false;
	if (path_ao != "")
	{
		_texAO.reset(new Texture2D(path_ao));
	}
	else _showTexAO = false;
}

void Object::SetPosition(float x, float y, float z)
{
	model->position = glm::vec3(x, y, z);
}

void Object::SetScale(float x, float y, float z)
{
	model->scale = glm::vec3(x, y, z);
}

void Object::SetRotation(glm::quat rotation)
{
	model->rotation = rotation;
}

void Object::SetRotation(glm::vec3 axis, float angle)
{
	model->rotation = glm::angleAxis(angle, axis) * glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	//std::cout << "SET: " << model->rotation.x << " " << model->rotation.y << " " << model->rotation.z << std::endl;
}

std::shared_ptr<Model> Object::GetModel() const
{
	return model;
}

glm::vec3 Object::GetPosition() const
{
	return model->position;
}

glm::vec3 Object::GetScale() const
{
	return model->scale;
}

glm::quat Object::GetRotation() const
{
	//std::cout << "GET: " << model->rotation.x << " " << model->rotation.y << " " << model->rotation.z << std::endl;
	return model->rotation;
}

void Object::Render(std::shared_ptr<Shader> shader, RenderMode render_mode, float delta_time, std::shared_ptr<Texture> texture)
{
	if (hidden && index_2048<0)
		return;

	//std::cout << "Rendering " << objPath << std::endl;

	switch (render_mode) {
	case RenderMode::Simple:
		shader->setMat4("model", model->getModelMatrix());
		shader->setVec3("albedo", Albedo);
		shader->setBool("showAlbedo", _showTexAlbedo);
		shader->setInt("texAlbedo", 0);
		glActiveTexture(GL_TEXTURE0);
		if (_showTexAlbedo && _texAlbedo)
		{
			_texAlbedo->bind();
		}
		break;
	case RenderMode::FBR:
		shader->setMat4("model", model->getModelMatrix());

		shader->setVec3("material.albedo", Albedo);
		shader->setFloat("material.roughness", Roughness);
		shader->setFloat("material.metallic", Metallic);

		shader->setBool("showAlbedo", _showTexAlbedo);
		shader->setBool("showNormal", _showTexNormal);
		shader->setBool("showRoughness", _showTexRoughness);
		shader->setBool("showMetallic", _showTexMetallic);
		shader->setBool("showAO", _showTexAO);

		shader->setInt("diffuse", 0);
		shader->setInt("normal", 1);
		shader->setInt("roughness", 2);
		shader->setInt("metallic", 3);
		shader->setInt("ao", 4);

		glActiveTexture(GL_TEXTURE0);
		if (ObjectType)
		{
			texture->bind();
		}
		else if (_showTexAlbedo && _texAlbedo)
		{
			_texAlbedo->bind();
		}
		glActiveTexture(GL_TEXTURE1);
		if (_showTexNormal && _texNormal)
		{
			_texNormal->bind();
		}
		glActiveTexture(GL_TEXTURE2);
		if (_showTexRoughness && _texRoughness)
		{
			_texRoughness->bind();
		}
		glActiveTexture(GL_TEXTURE3);
		if (_showTexMetallic && _texMetallic)
		{
			_texMetallic->bind();
		}
		glActiveTexture(GL_TEXTURE4);
		if (_showTexAO && _texAO)
		{
			_texAO->bind();
		}
		//----------------------------------------------------------------
		break;
	}

	model->draw();
}

ObjectSequence::ObjectSequence(std::string path_model, int frame_num, int fps, std::string name,
	std::string path_albedo, std::string path_normal, std::string path_roughness,
	std::string path_metallic, std::string path_ao) :
	Object("", name, path_albedo, path_normal, path_roughness, path_metallic, path_ao), frameNumber(frame_num), FPS(fps)
{
	for (int i = 0; i < frame_num; i++)
	{
		std::string path = path_model + std::to_string(i) + ".obj";
		std::shared_ptr<Model> model_frame;
		model_frame.reset(new Model(path));
		models.push_back(model_frame);
	}
}

void ObjectSequence::Render(std::shared_ptr<Shader> shader, RenderMode render_mode, float delta_time, std::shared_ptr<Texture> texture)
{	
	currentTime += delta_time;
	if (currentTime > FPS * frameNumber)
	{
		currentTime -= FPS * frameNumber;
	}
	currentFrame = (int)(currentTime * FPS) % frameNumber;
	
	if (hidden)
		return;

	//std::cout << "Rendering Frame " << currentFrame << std::endl;

	switch (render_mode) {
	case RenderMode::Simple:
		shader->setMat4("model", models[currentFrame]->getModelMatrix());
		shader->setVec3("albedo", Albedo);
		shader->setBool("showAlbedo", _showTexAlbedo);
		shader->setInt("texAlbedo", 0);
		glActiveTexture(GL_TEXTURE0);
		if (_showTexAlbedo && _texAlbedo)
		{
			_texAlbedo->bind();
		}
		break;
	case RenderMode::FBR:
		shader->setMat4("model", models[currentFrame]->getModelMatrix());

		shader->setVec3("material.albedo", Albedo);
		shader->setFloat("material.roughness", Roughness);
		shader->setFloat("material.metallic", Metallic);

		shader->setBool("showAlbedo", _showTexAlbedo);
		shader->setBool("showNormal", _showTexNormal);
		shader->setBool("showRoughness", _showTexRoughness);
		shader->setBool("showMetallic", _showTexMetallic);
		shader->setBool("showAO", _showTexAO);

		shader->setInt("diffuse", 0);
		shader->setInt("normal", 1);
		shader->setInt("roughness", 2);
		shader->setInt("metallic", 3);
		shader->setInt("ao", 4);

		glActiveTexture(GL_TEXTURE0);
		if (_showTexAlbedo && _texAlbedo)
		{
			_texAlbedo->bind();
		}
		glActiveTexture(GL_TEXTURE1);
		if (_showTexNormal && _texNormal)
		{
			_texNormal->bind();
		}
		glActiveTexture(GL_TEXTURE2);
		if (_showTexRoughness && _texRoughness)
		{
			_texRoughness->bind();
		}
		glActiveTexture(GL_TEXTURE3);
		if (_showTexMetallic && _texMetallic)
		{
			_texMetallic->bind();
		}
		glActiveTexture(GL_TEXTURE4);
		if (_showTexAO && _texAO)
		{
			_texAO->bind();
		}
		//----------------------------------------------------------------
		break;
	}

	models[currentFrame]->draw();
}

void ObjectSequence::SetPosition(float x, float y, float z)
{
	for (auto& model : models)
		model->position = glm::vec3(x, y, z);
}

void ObjectSequence::SetScale(float x, float y, float z)
{
	for (auto& model : models)
		model->scale = glm::vec3(x, y, z);
}

void ObjectSequence::SetRotation(glm::quat rotation)
{
	for (auto& model : models)
		model->rotation = rotation;
}

void ObjectSequence::SetRotation(glm::vec3 axis, float angle)
{
	for (auto& model : models)
		model->rotation = glm::angleAxis(angle, axis) * glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
}

std::shared_ptr<Model> ObjectSequence::GetModel() const
{
	return models[currentFrame];
}

glm::vec3 ObjectSequence::GetPosition() const
{
	return models[0]->position;
}

glm::vec3 ObjectSequence::GetScale() const
{
	return models[0]->scale;
}

glm::quat ObjectSequence::GetRotation() const
{
	return models[0]->rotation;
}


TextureMapping::TextureMapping() {
	_windowTitle = "Texture Mapping";

	std::cout << "Loading model.." << std::endl;

	_pathModel = _pathAlbedo = _pathNormal = _pathMetallic = _pathRoughness = _pathAO = "";
	
	//create new 2048 bricks 
	//for (int i = 0;i < _objects.size();i++) _objects[i]->hidden = true;
	std::string s[16] = {
	"../data/2048bricks/tex1.png",	"../data/2048bricks/tex2.png",
	"../data/2048bricks/tex3.png",	"../data/2048bricks/tex4.png",
	"../data/2048bricks/tex5.png",	"../data/2048bricks/tex6.png",
	"../data/2048bricks/tex7.png",	"../data/2048bricks/tex8.png",
	"../data/2048bricks/tex9.png",	"../data/2048bricks/tex10.png",
	"../data/2048bricks/tex11.png",	"../data/2048bricks/tex12.png",
	"../data/2048bricks/tex13.png",	"../data/2048bricks/tex14.png",
	"../data/2048bricks/tex15.png",	"../data/2048bricks/tex16.png"
	};
	for(int i=0;i<16;i++) _texAlbedoList[i].reset(new Texture2D(s[i]));
	float width = 2.2f, start = -3.3f;
	for (int i = 0;i < 16;i++)
	{
		Object* brick = new Object("../data/cube.obj", "Cube", "../data/2048bricks/tex1.png");
		brick->ObjectType = 1;
		brick->SetPosition(start + (i / 4) * width, 0.0f, start + (i % 4) * width);
		brick->hidden = true;
		brick->SetScale(1.0f, 0.5f, 1.0f);
		_2048bricks.push_back(brick);
	}
	 

	/*
	// init model
	_extintor.reset(new Model(modelPath));
	_extintor->scale = glm::vec3(0.1f, 0.1f, 0.1f);
	_extintor->position = glm::vec3(0.0f, -3.5f, 0.0f);
	std::cout << "Model loaded." << std::endl;
	*/
	// init skybox
	_skybox.reset(new SkyBox(skyboxTexturePaths));

	// init camera
	_camera.reset(new PerspectiveCamera(glm::radians(50.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 10000.0f));
	_camera->position = { 0.0f, 0.0f, 20.0f };
	_camera->rotation = { 1.0f, 0.0f, 0.0f, 0.0f };

	// init light
	_directionalLight.reset(new DirectionalLight);
	_directionalLight->position = glm::vec3(10.0f, 10.0f, 10.0f);
	_directionalLight->rotation = glm::angleAxis(glm::radians(45.0f), -glm::vec3(1.0f, 1.0f, 1.0f));
	_directionalLight->intensity = 0.2f;
	//_directionalLight->

	_spotLight.reset(new SpotLight);
	_spotLight->intensity = 20.0f;
	_spotLight->position = glm::vec3(0.0f, 3.0f, 3.0f);
	//_spotLight->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	/*
	_texAlbedo.reset(new Texture2D(texPathAlbedo));
	_texRoughness.reset(new Texture2D(texPathRoughness));
	_texMetallic.reset(new Texture2D(texPathMetallic));
	_texNormal.reset(new Texture2D(texPathNormal));
	_texAO.reset(new Texture2D(texPathAO));
	_texWhite.reset(new Texture2D(texPathWhite));
	_texBlack.reset(new Texture2D(texPathBlack));
	_texBlue.reset(new Texture2D(texPathBlue));
	*/
	_showTexAlbedo = _showTexNormal = _showTexMetallic = _showTexRoughness = _showTexAO = true;
	//_showTexNormal = false;

	// init shaders
	initSimpleShader();

	initFBRShader();

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();
}

TextureMapping::~TextureMapping() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void TextureMapping::ParseArguments(int argc, char* argv[])
{
	float size = 1.0f;
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-model")) {
			i++;
			_pathModel = argv[i];
		}
		else if (!strcmp(argv[i], "-albedo")) {
			i++;
			_pathAlbedo = argv[i];
		}
		else if (!strcmp(argv[i], "-normal")) {
			i++;
			_pathNormal = argv[i];
		}
		else if (!strcmp(argv[i], "-roughness")) {
			i++;
			_pathRoughness = argv[i];
		}
		else if (!strcmp(argv[i], "-metallic")) {
			i++;
			_pathMetallic = argv[i];
		}
		else if (!strcmp(argv[i], "-ao")) {
			i++;
			_pathAO = argv[i];
		}
		else if (!strcmp(argv[i], "-size")) {
			i++;
			size = atof(argv[i]);
		}
	}
	if (_pathModel == "")
	{
		_pathModel = "../data/ext/Extintor.obj";
		_pathAlbedo = "../data/ext/Extintor_BaseColor.png";
		_pathNormal = "../data/ext/Extintor_Normal.png";
		_pathRoughness = "../data/ext/Extintor_Roughness.png";
		_pathMetallic = "../data/ext/Extintor_Metallic.png";
		_pathAO = "../data/ext/Extintor_AO.png";
		size = 0.1f;
	}

	std::cout << "Model: " << _pathModel << std::endl <<
		"Size: " << size << std::endl <<
		"Albedo: " << _pathAlbedo << std::endl <<
		"Normal: " << _pathNormal << std::endl <<
		"Roughness: " << _pathRoughness << std::endl <<
		"Metallic: " << _pathMetallic << std::endl <<
		"AO: " << _pathAO << std::endl;

	Object* obj = new Object(_pathModel, "Object",
		_pathAlbedo, _pathNormal, _pathRoughness, _pathMetallic, _pathAO);
	obj->SetPosition(0.0f, 0.0f, 0.0f);
	obj->SetScale(size, size, size);
	_objects.push_back(obj);

}


void TextureMapping::initSimpleShader() {
	const char* vertCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"

		"void main() {\n"
		"	TexCoord = aTexCoord;\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* fragCode =
		"#version 330 core\n"
		"in vec2 TexCoord;\n"
		"out vec4 color;\n"
		"uniform sampler2D texAlbedo;\n"
		"uniform bool showAlbedo;\n"
		"uniform vec3 albedo;\n"
		"void main() {\n"
		"	color = showAlbedo ? texture(texAlbedo, TexCoord) : vec4(albedo, 1.0);\n"
		"}\n";

	_simpleShader.reset(new Shader(vertCode, fragCode));
}

void TextureMapping::initFBRShader() {
	const char* vertCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"
		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"

		"void main() {\n"
		"	FragPos = vec3(model * vec4(aPosition, 1.0f));\n"
		"	Normal = mat3(transpose(inverse(model))) * aNormal;\n"
		"	TexCoord = aTexCoord;\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	//----------------------------------------------------------------
	// write your code here
	// change the following code to achieve the following goals
	// + blend of the two textures
	// + lambert shading, i.e the color is affected by the light
	const char* fragCode =
		"#version 330 core\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TexCoord;\n"
		"out vec4 color;\n"

		"uniform vec3 CamPos;\n"

		"// spot light data structure declaration\n"
		"struct SpotLight {\n"
		"	vec3 position;\n"
		"	vec3 direction;\n"
		"	float intensity;\n"
		"	vec3 color;\n"
		"	float angle;\n"
		"	float kc;\n"
		"	float kl;\n"
		"	float kq;\n"
		"};\n"

		"struct DirectionalLight {\n"
		"	vec3 direction;\n"
		"	vec3 color;\n"
		"	float intensity;\n"
		"};\n"

		"struct Material {\n"
		"	vec3 albedo;\n"
		"	float roughness;\n"
		"	float metallic;\n"
		"};\n"

		"uniform DirectionalLight directionalLight;\n"
		"uniform SpotLight spotLight;\n"
		"uniform Material material;\n"

		"uniform sampler2D diffuse;\n"
		"uniform sampler2D normal;\n"
		"uniform sampler2D roughness;\n"
		"uniform sampler2D metallic;\n"
		"uniform sampler2D ao;\n"

		"uniform bool showAlbedo;\n"
		"uniform bool showNormal;\n"
		"uniform bool showRoughness;\n"
		"uniform bool showMetallic;\n"
		"uniform bool showAO;\n"

		"const float PI = 3.14159265359;\n"

		"vec3 calcDirectionalLight(vec3 normal, vec3 ambient) {\n"
		"	vec3 lightDir = normalize(-directionalLight.direction);\n"
		"   vec3 reflectDir = reflect(-lightDir, normal);\n"
		"   vec3 viewDir = normalize(-FragPos);\n"
		"	// diffuse color\n"
		"	vec3 diffuse = directionalLight.color * max(dot(lightDir, normal), 0.0f) * ambient;\n"
		"	return directionalLight.intensity * diffuse ;\n"
		"}\n"

		"vec3 getNormalFromMap()\n"
		"{\n"
		"	if (showNormal)\n"
		"	{\n"
		"		vec3 tangentNormal = texture(normal, TexCoord).xyz * 2.0 - 1.0;\n"

		"		vec3 Q1 = dFdx(FragPos);\n"
		"		vec3 Q2 = dFdy(FragPos);\n"
		"		vec2 st1 = dFdx(TexCoord);\n"
		"		vec2 st2 = dFdy(TexCoord);\n"

		"		vec3 N = normalize(Normal);\n"
		"		vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);\n"
		"		vec3 B = -normalize(cross(N, T));\n"
		"		mat3 TBN = mat3(T, B, N);\n"
			
		"		return normalize(TBN * tangentNormal);\n"
		"   }\n"
		"   else\n"
		"	{\n"
		"		return normalize(Normal);\n"
		"	}\n"
		"}\n"

		"// ----------------------------------------------------------------------------\n"
		"float DistributionGGX(vec3 N, vec3 H, float roughness)\n"
		"{\n"
		"	float a = roughness * roughness;\n"
		"	float a2 = a * a;\n"
		"	float NdotH = max(dot(N, H), 0.0);\n"
		"	float NdotH2 = NdotH * NdotH;\n"

		"	float nom = a2;\n"
		"	float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n"
		"	denom = PI * denom * denom;\n"

		"	return nom / denom;\n"
		"}\n"
		"// ----------------------------------------------------------------------------\n"
		"float GeometrySchlickGGX(float NdotV, float roughness)\n"
		"{\n"
		"	float r = (roughness + 1.0);\n"
		"	float k = (r * r) / 8.0;\n"

		"	float nom = NdotV;\n"
		"	float denom = NdotV * (1.0 - k) + k;\n"
		
		"	return nom / denom;\n"
		"}\n"
		"// ----------------------------------------------------------------------------\n"
		"float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)\n"
		"{\n"
		"	float NdotV = max(dot(N, V), 0.0);\n"
		"	float NdotL = max(dot(N, L), 0.0);\n"
		"	float ggx2 = GeometrySchlickGGX(NdotV, roughness);\n"
		"	float ggx1 = GeometrySchlickGGX(NdotL, roughness);\n"

		"	return ggx1 * ggx2;\n"
		"}\n"
		"// ----------------------------------------------------------------------------\n"
		"vec3 fresnelSchlick(float cosTheta, vec3 F0)\n"
		"{\n"
		"	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);\n"
		"}\n"

		"void main() {\n"
		"	vec3 normal = normalize(Normal);\n"
		"	// ambient color\n"
		"	vec3 col_albedo = showAlbedo ? pow(texture(diffuse, TexCoord).rgb, vec3(2.2)) : pow(material.albedo, vec3(2.2));\n"
		"	float col_roughness = showRoughness ? texture(roughness, TexCoord).r : material.roughness;\n"
		"	float col_metallic = showMetallic ? texture(metallic, TexCoord).r : material.metallic;\n"
		"	float col_ao = showAO ? texture(ao, TexCoord).r : 1.0;\n"

		"	vec3 N = getNormalFromMap();\n"
		"	vec3 Lo = vec3(0.0);\n"
		"	vec3 F0 = vec3(0.04);\n"
		"	F0 = mix(F0, col_albedo, col_metallic);\n"
		"	vec3 viewDir = normalize(CamPos-FragPos);\n"
		"	vec3 L = normalize(spotLight.position - FragPos);\n"
		"	vec3 H = normalize(viewDir + L);\n"
		"	float distance = length(spotLight.position - FragPos);\n"
		"	float attenuation = 1.0 / (distance * distance);\n"
		"	vec3 radiance = spotLight.color * attenuation * spotLight.intensity;\n"
			
		"	// Cook-Torrance BRDF\n"
		"	float NDF = DistributionGGX(N, H, col_roughness);\n"
		"	float G = GeometrySmith(N, viewDir, L, col_roughness);\n"
		"	vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);\n"

		"	vec3 numerator = NDF * G * F;\n"
		"	float denominator = 4 * max(dot(N, viewDir), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero\n"
		"	vec3 specular = numerator / denominator;\n"

		"	// kS is equal to Fresnel\n"
		"	vec3 kS = F;\n"
		"	// for energy conservation, the diffuse and specular light can't\n"
		"	// be above 1.0 (unless the surface emits light); to preserve this\n"
		"	// relationship the diffuse component (kD) should equal 1.0 - kS.\n"
		"	vec3 kD = vec3(1.0) - kS;\n"
		"	// multiply kD by the inverse metalness such that only non-metals \n"
		"	// have diffuse lighting, or a linear blend if partly metal (pure metals\n"
		"	// have no diffuse light).\n"
		"	kD *= 1.0 - col_metallic;\n"

		"	// scale light by NdotL\n"
		"	float NdotL = max(dot(N, L), 0.0);\n"

		"	// add to outgoing radiance Lo\n"
		"	Lo += (kD * col_albedo / PI + specular) * radiance * NdotL;\n"
		"	Lo += calcDirectionalLight(N, col_albedo);\n"
		"	vec3 ambient = vec3(0.03) * col_albedo * col_ao;\n"

		"	vec3 col = ambient + Lo;\n"

		"	// HDR tonemapping\n"
		"	col = col / (col + vec3(1.0));\n"
		"	// gamma correct\n"
		"	col = pow(col, vec3(1.0 / 2.2));\n"
		"	color = vec4(col, 1.0);\n"
		"}\n";
	//----------------------------------------------------------------


	_FBRShader.reset(new Shader(vertCode, fragCode));
}

void TextureMapping::update() {
	int i;
	for (i = 0;i < 16;i++)
		_2048bricks[i]->index_2048= number[i / 4][i % 4]-1;
}

void TextureMapping::addBrick() {
	int i, j, tot = 0,t;
	for (i = 0;i < 16;i++) tot += (number[i/4][i%4]==0);
	t = rand() % tot;
	for(i=0;i<16;i++) 
		if (number[i/4][i%4]==0)
		{
			if (t)t--;
			else { number[i / 4][i % 4] = (rand()%4+1)/4+1; break; }
		}
}

void swp(int& x, int& y) { int t = x;x = y;y = t; }
int mx(int x, int y) {return  x >= y ? x : y; }

void TextureMapping::handle_moving(int direct) {
	int i, j, tot, t,pd=0;
	static int gameover = 0;
	if (direct == 1)
	{
		for (i = 0;i < 4;i++)
			for (j = 0;j < 2;j++)
				swp(number[i][j], number[i][3 - j]);
	}
	else if (direct == 2)
	{
		for (i = 0;i < 4;i++)
			for (j = 0;j < i;j++)
				swp(number[i][j], number[j][i]);
	}
	else if(direct==3)
	{
		for (i = 0;i < 4;i++)
			for (j = 0;j < 3 - i;j++)
				swp(number[i][j], number[3 - j][3 - i]);
	}

	memset(tar, 0, sizeof(tar));
	for (i = 0;i < 4;i++)
	{
		for (j = 0, t = 0, tot = 0;j < 4;j++)
		{
			if (!number[i][j]) continue;
			if (t == number[i][j]) tar[i][tot] = t + 1, tot++, score+=(pow(2,t+1)+0.5), t = 0;
			else if (t) tar[i][tot] = t, tot++, t = number[i][j];
			else t = number[i][j];
		}
		if (t) tar[i][tot] = t, tot++, t = number[i][j];
	}
	for (i = 0;i < 4;i++)
		for (j = 0;j < 4;j++)
			pd+=(number[i][j]!=tar[i][j]),number[i][j] = tar[i][j] ;

	if (direct == 1)
	{
		for (i = 0;i < 4;i++)
			for (j = 0;j < 2;j++)
				swp(number[i][j], number[i][3 - j]);
	}
	else if (direct == 2)
	{
		for (i = 0;i < 4;i++)
			for (j = 0;j < i;j++)
				swp(number[i][j], number[j][i]);
	}
	else if (direct == 3)
	{
		for (i = 0;i < 4;i++)
			for (j = 0;j < 3 - i;j++)
				swp(number[i][j], number[3 - j][3 - i]);
	}
	if (pd) {
		addBrick();
		std::cout << "\nsuccess moving\n";
		std::cout << "current score: " << score << "\n";
		gameover = 0;
	}
	pd = 0;
	for (i = 0;i < 4;i++)
		for (j = 0;j < 4;j++) 
	{
		if (number[i][j] == 0) pd = 1;
		if (number[i][j] == number[i + 1][j]) pd = 1;
		if (number[i][j] == number[i][j + 1]) pd = 1;

	}
	if (!gameover && !pd)
	{
		gameover = 1;
		std::cout << "\n------------------------\n\n";
		std::cout << "gameover\n";
		std::cout << "final score: " << score << "\n";
		if (score > max_score) max_score = score;
		std::cout << "best score: " << max_score << "\n";
		std::cout << "\n------------------------\n\n";
		std::cout << "press SPACE to restart\n";
	}
}

void TextureMapping::handleInput() {
	const float angluarVelocity = 0.1f;
	const float angle = angluarVelocity * static_cast<float>(_deltaTime);
	const glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
	//_objects[0]->SetRotation(glm::angleAxis(angle, axis) * _objects[0]->GetRotation());
	//_extintor->rotation = glm::angleAxis(angle, axis) * _extintor->rotation;

	const float cameraMoveSpeed = 0.25f;
	const float cameraRotateSpeed = 0.1f;

	if (_keyboardInput.keyStates[GLFW_KEY_LEFT_CONTROL] == GLFW_PRESS) {
		// set input mode
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_mouseInput.move.xOld = _mouseInput.move.xCurrent = 0.5 * _windowWidth;
		_mouseInput.move.yOld = _mouseInput.move.yCurrent = 0.5 * _windowHeight;
		glfwSetCursorPos(_window, _mouseInput.move.xCurrent, _mouseInput.move.yCurrent);
	}
	else if (_keyboardInput.keyStates[GLFW_KEY_LEFT_CONTROL] == GLFW_RELEASE) {
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (_keyboardInput.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_ENTER] == GLFW_PRESS)
	{
		_camera->position = { 0.0f, 22.0f, 0.0f };
		_camera->rotation = { 0.707106f,-0.707106f, 0.0f,0.0f };

	}

	if (_keyboardInput.keyStates[GLFW_KEY_LEFT_CONTROL] != GLFW_RELEASE) {
		int pd = 0;
		glm::vec3 temp = _camera->position;
		if (_keyboardInput.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
			_camera->position += _camera->getFront() * cameraMoveSpeed; pd = 1;// glm::vec3(0.0f, 0.0f, -);
		}

		if (_keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
			_camera->position -= _camera->getRight() * cameraMoveSpeed; pd = 1;
		}

		if (_keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
			_camera->position -= _camera->getFront() * cameraMoveSpeed; pd = 1;
		}

		if (_keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
			_camera->position += _camera->getRight() * cameraMoveSpeed; pd = 1;
		}
		//collision detection
		if (pd) {
			//std::cout << _camera->position.x << " " << _camera->position.y << " " << _camera->position.z << std::endl;
			pd = 0;
			for (auto obj : _objects)
			{
				if (pd) break;
				if (obj->GetModel()->inside(_camera->position)) pd = 1;
			}
			for (auto obj : _2048bricks)
			{
				if (pd) break;
				if (obj->GetModel()->inside(_camera->position)) pd = 1;
			}
			if (pd) _camera->position = temp;
		}

		if (_mouseInput.move.xCurrent != _mouseInput.move.xOld) {
			const float deltaX = static_cast<float>(_mouseInput.move.xCurrent - _mouseInput.move.xOld);
			const float angle = -cameraRotateSpeed * _deltaTime * deltaX;
			const glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
			_camera->rotation = glm::quat(
				cos(angle / 2.0f), sin(angle / 2.0f) * axis.x,
				sin(angle / 2.0f) * axis.y, sin(angle / 2.0f) * axis.z) * _camera->rotation;
			_mouseInput.move.xOld = _mouseInput.move.xCurrent;
			
		}

		if (_mouseInput.move.yCurrent != _mouseInput.move.yOld) {
			const float deltaY = static_cast<float>(_mouseInput.move.yCurrent - _mouseInput.move.yOld);
			const float angle = -cameraRotateSpeed * _deltaTime * deltaY;
			const glm::vec3 axis = _camera->getRight();
			_camera->rotation = glm::quat(
				cos(angle / 2.0f), sin(angle / 2.0f) * axis.x,
				sin(angle / 2.0f) * axis.y, sin(angle / 2.0f) * axis.z) * _camera->rotation;
			_mouseInput.move.yOld = _mouseInput.move.yCurrent;
		}
	}
	else//play 2048 game
	{
		
		static int last_state[1000] = {0};
		static int step = 0;

		//start game
		/*
		if (_keyboardInput.keyStates[GLFW_KEY_SPACE] == GLFW_RELEASE && last_state[GLFW_KEY_SPACE] == GLFW_PRESS)
		{
			for (int i = 0;i < 16;i++) number[i / 4][i % 4] = 0;
			addBrick();
			addBrick();
			update();
			score = 0;
			std::cout << "\ngame start\n";
		}
		//handle input
		if (_keyboardInput.keyStates[GLFW_KEY_W] == GLFW_RELEASE && last_state[GLFW_KEY_W]== GLFW_PRESS) {
			handle_moving(0), update();
		}
		if (_keyboardInput.keyStates[GLFW_KEY_S] == GLFW_RELEASE && last_state[GLFW_KEY_S] == GLFW_PRESS) {
			handle_moving(1), update();
		}
		if (_keyboardInput.keyStates[GLFW_KEY_A] == GLFW_RELEASE && last_state[GLFW_KEY_A] == GLFW_PRESS) {
			handle_moving(2),  update();
		}
		if (_keyboardInput.keyStates[GLFW_KEY_D] == GLFW_RELEASE && last_state[GLFW_KEY_D] == GLFW_PRESS) {
			handle_moving(3), update();
		}
		last_state[GLFW_KEY_W] = _keyboardInput.keyStates[GLFW_KEY_W];
		last_state[GLFW_KEY_S] = _keyboardInput.keyStates[GLFW_KEY_S];
		last_state[GLFW_KEY_A] = _keyboardInput.keyStates[GLFW_KEY_A];
		last_state[GLFW_KEY_D] = _keyboardInput.keyStates[GLFW_KEY_D];
		last_state[GLFW_KEY_SPACE] = _keyboardInput.keyStates[GLFW_KEY_SPACE];
		*/
	}
}

void TextureMapping::renderFrame() {
	// some options related to imGUI
	static bool wireframe = false;
	
	// trivial things
	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	const glm::mat4& projection = _camera->getProjectionMatrix();
	const glm::mat4& view = _camera->getViewMatrix();

	switch (_renderMode) {
	case RenderMode::Simple:
		// 1. use the shader
		_simpleShader->use();
		// 2. transfer mvp matrices to gpu 
		_simpleShader->setMat4("projection", projection);
		_simpleShader->setMat4("view", view);
		//_simpleShader->setMat4("model", _extintor->getModelMatrix());
		// 3. enable textures and transform textures to gpu
		/*glActiveTexture(GL_TEXTURE0);
		_texAlbedo->bind();*/
		break;
	case RenderMode::FBR:
		// 1. use the shader
		_FBRShader->use();
		// 2. transfer mvp matrices to gpu 
		_FBRShader->setMat4("projection", projection);
		_FBRShader->setMat4("view", view);
		/*_FBRShader->setMat4("model", _extintor->getModelMatrix());*/
		_FBRShader->setVec3("CamPos", _camera->position);
		// 3. transfer light attributes to the shader
		_FBRShader->setVec3("spotLight.position", _spotLight->position);
		_FBRShader->setVec3("spotLight.direction", _spotLight->getFront());
		_FBRShader->setFloat("spotLight.intensity", _spotLight->intensity);
		_FBRShader->setVec3("spotLight.color", _spotLight->color);
		_FBRShader->setFloat("spotLight.angle", _spotLight->angle);
		_FBRShader->setFloat("spotLight.kc", _spotLight->kc);
		_FBRShader->setFloat("spotLight.kl", _spotLight->kl);
		_FBRShader->setFloat("spotLight.kq", _spotLight->kq);
		_FBRShader->setVec3("directionalLight.direction", _directionalLight->getFront());
		_FBRShader->setFloat("directionalLight.intensity", _directionalLight->intensity);
		_FBRShader->setVec3("directionalLight.color", _directionalLight->color);
		
		//_FBRShader->setVec3("material.albedo", _albedo);
		//_FBRShader->setFloat("material.roughness", _roughness);
		//_FBRShader->setFloat("material.metallic", _metallic);

		//_FBRShader->setBool("showAlbedo", _showTexAlbedo);
		//_FBRShader->setBool("showNormal", _showTexNormal);
		//_FBRShader->setBool("showRoughness", _showTexRoughness);
		//_FBRShader->setBool("showMetallic", _showTexMetallic);
		//_FBRShader->setBool("showAO", _showTexAO);

		//// 4.3 enable textures and transform textures to gpu
		////----------------------------------------------------------------
		//// write your code here
		//_FBRShader->setInt("diffuse", 0);
		//_FBRShader->setInt("normal", 1);
		//_FBRShader->setInt("roughness", 2);
		//_FBRShader->setInt("metallic", 3);
		//_FBRShader->setInt("ao", 4);

		//glActiveTexture(GL_TEXTURE0);
		//if (_showTexAlbedo)
		//{
		//	_texAlbedo->bind();
		//}
		//else
		//{
		//	_texWhite->bind();
		//}
		//glActiveTexture(GL_TEXTURE1);
		//if (_showTexNormal)
		//{
		//	_texNormal->bind();
		//}
		//else
		//{
		//	_texBlue->bind();
		//}
		//glActiveTexture(GL_TEXTURE2);
		//if (_showTexRoughness)
		//{
		//	_texRoughness->bind();
		//}
		//else
		//{
		//	_texBlack->bind();
		//}
		//glActiveTexture(GL_TEXTURE3);
		//if (_showTexMetallic)
		//{
		//	_texMetallic->bind();
		//}
		//else
		//{
		//	_texBlack->bind();
		//}
		//glActiveTexture(GL_TEXTURE4);
		//if (_showTexAO)
		//{
		//	_texAO->bind();
		//}
		//else
		//{
		//	_texWhite->bind();
		//}
		//----------------------------------------------------------------
		break;
	}

	/*_extintor->draw();*/

	for (auto obj : _objects)
	{
		switch (_renderMode) {
		case RenderMode::Simple:
			obj->Render(_simpleShader, RenderMode::Simple, _deltaTime,obj->_texAlbedo);
			break;
		case RenderMode::FBR:
			obj->Render(_FBRShader, RenderMode::FBR, _deltaTime, obj->_texAlbedo);
			break;
		}
		
		if (obj->texPathAlbedo != "")		obj->_showTexAlbedo = _showTexAlbedo;
		if (obj->texPathNormal != "")		obj->_showTexNormal = _showTexNormal;
		if (obj->texPathRoughness != "")	obj->_showTexRoughness = _showTexRoughness;
		if (obj->texPathMetallic != "")		obj->_showTexMetallic = _showTexMetallic;
		if (obj->texPathAO != "")			obj->_showTexAO = _showTexAO;

	}

	//draw 2048 bricks
	for (auto obj : _2048bricks)
	{
		switch (_renderMode) {
		case RenderMode::Simple:
			obj->Render(_simpleShader, RenderMode::Simple, _deltaTime, _texAlbedoList[mx(obj->index_2048,0)]);
			break;
		case RenderMode::FBR:
			obj->Render(_FBRShader, RenderMode::FBR, _deltaTime, _texAlbedoList[mx(obj->index_2048, 0)]);
			break;
		}
		if (obj->texPathAlbedo != "")		obj->_showTexAlbedo = _showTexAlbedo;
		if (obj->texPathNormal != "")		obj->_showTexNormal = _showTexNormal;
		if (obj->texPathRoughness != "")	obj->_showTexRoughness = _showTexRoughness;
		if (obj->texPathMetallic != "")		obj->_showTexMetallic = _showTexMetallic;
		if (obj->texPathAO != "")			obj->_showTexAO = _showTexAO;
	}

	// draw skybox
	_skybox->draw(projection, view);

	// draw ui elements
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::Begin("Control Panel", nullptr, flags)) {
		ImGui::End();
	} else {
		ImGui::Text("Render Mode");
		ImGui::Separator();
		ImGui::RadioButton("Simple Shading", (int*)&_renderMode, (int)(RenderMode::Simple));
		ImGui::RadioButton("FBR Shading", (int*)&_renderMode, (int)(RenderMode::FBR));
		ImGui::NewLine();
		/*
		ImGui::Text("Render Scene");
		ImGui::Separator();
		ImGui::RadioButton("Room of Geometries", (int*)&_renderScene, (int)(RenderScene::Geometry));
		ImGui::RadioButton("Extintor", (int*)&_renderScene, (int)(RenderScene::Extintor));
		ImGui::NewLine();
		*/
		ImGui::Text("Directional Light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##1", &_directionalLight->intensity, 0.0f, 20.0f);
		ImGui::ColorEdit3("color##1", (float*)&_directionalLight->color);
		ImGui::NewLine();

		ImGui::Text("Spot Light");
		ImGui::Separator();
		ImGui::SliderFloat3("Position", (float*)&_spotLight->position, -10.0f, 10.0f);
		ImGui::SliderFloat("intensity##2", &_spotLight->intensity, 0.0f, 50.0f);
		ImGui::ColorEdit3("color##2", (float*)&_spotLight->color);
		ImGui::NewLine();

		ImGui::Text("Texture");
		ImGui::Separator();
		ImGui::Checkbox("Albedo", &_showTexAlbedo);
		ImGui::ColorEdit3("albedo", (float*)&_albedo);
		ImGui::Checkbox("Normal", &_showTexNormal);
		ImGui::Checkbox("Roughness", &_showTexRoughness);
		ImGui::SliderFloat("roughness", &_roughness, 0.0f, 1.0f);
		ImGui::Checkbox("Metallic", &_showTexMetallic);
		ImGui::SliderFloat("metallic", &_metallic, 0.0f, 1.0f);
		ImGui::Checkbox("AO", &_showTexAO);
		ImGui::NewLine();

		if (ImGui::Button("Screenshot", ImVec2(80.0f, 20.0f)))
		{
			//std::cout << "Button Clicked\n";
			takeScreenshot();
		}
		/*
		if (ImGui::Button("Export .obj", ImVec2(80.0f, 20.0f)))
		{
			//std::cout << "Button Clicked\n";
			exportObject(_objects[9]);
		}
		*/

		ImGui::End();
	}

	if (_firstFrame)
	{
		ImGui::SetNextWindowPos(ImVec2(920.0f, 60.0f));
	}
	ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f));
	if (!ImGui::Begin("Transforms", nullptr, flags)) {
		ImGui::End();
	}
	else {
		for (int i=0; i<_objects.size(); i++)
		{
			Object* obj = _objects[i];
			ImGui::Text(obj->Name.c_str());
			ImGui::Separator();

			glm::vec3 pos = obj->GetPosition();
			glm::vec3 scale = obj->GetScale();
			glm::quat rotation = obj->GetRotation();

			ImGui::SliderFloat3(("Position##" + std::to_string(i+1)).c_str(), (float*)&pos, -10.0f, 10.0f);
			ImGui::SliderFloat3(("Scale##" + std::to_string(i+1)).c_str(), (float*)&scale, 0.02f, 2.0f);
			ImGui::SliderFloat4(("Rotation##" + std::to_string(i + 1)).c_str(), (float*)&rotation, -1.0f, 1.0f);

			obj->SetRotation(glm::normalize(rotation));
			obj->SetPosition(pos.x, pos.y, pos.z);
			obj->SetScale(scale.x, scale.y, scale.z);
			
			ImGui::NewLine();
		}

		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	_firstFrame = false;
	//double t = glfwGetTime();
	//_spotLight->position = glm::vec3(0.0f, 5.0f * sin(t * 3.1415926f), 5.0f);
}

void TextureMapping::takeScreenshot()
{
	FILE* fWrite;
	GLubyte* pixelData;
	GLubyte BMP_header[54];
	GLint i, j, dataLength;

	GLubyte header[54] = {
		0x42, 0x4D, 0x36, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	i = _windowWidth;
	j = _windowHeight;

	while (i % 4 != 0)
		i++;

	header[18] = i & 0x000000ff;
	header[19] = (i >> 8) & 0x000000ff;
	header[20] = (i >> 16) & 0x000000ff;
	header[21] = (i >> 24) & 0x000000ff;
	header[22] = j & 0x000000ff;
	header[23] = (j >> 8) & 0x000000ff;
	header[24] = (j >> 16) & 0x000000ff;
	header[25] = (j >> 24) & 0x000000ff;

	dataLength = i * _windowHeight * 3;
	//std::cout << i << j << dataLength << std::endl;
	/*
	fopen_s(&fDummy, "../data/dummy/dummy.bmp", "rb");
	if (fDummy == 0)
	{
		std::cout << "Unable to find dummy.bmp.\n";
		return;
	}
	*/
	time_t curTime;
	struct tm* curTime_tm = new tm{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	time(&curTime);
	localtime_s(curTime_tm, &curTime);
	char pth[100];
	sprintf_s(pth, "../screenshots/%d_%d_%d_%d_%d_%d.bmp",
		1900 + curTime_tm->tm_year, curTime_tm->tm_mon, curTime_tm->tm_mday,
		curTime_tm->tm_hour, curTime_tm->tm_min, curTime_tm->tm_sec);
	//std::cout << pth << std::endl;
	fopen_s(&fWrite, pth, "wb");
	if (fWrite == 0)
	{
		std::cout << "Unable to open " << pth << ".\n";
		return;
	}

	pixelData = (GLubyte*)malloc(dataLength);//new GLubyte[dataLength];

	//fread(BMP_header, sizeof(BMP_header), 1, fDummy);
	//fwrite(BMP_header, sizeof(BMP_header), 1, fWrite);
	fwrite(header, sizeof(header), 1, fWrite);
	//fseek(fWrite, 0x0012, SEEK_SET);
	//i = _windowWidth;
	//fwrite(&i, sizeof(i), 1, fWrite);
	//fwrite(&j, sizeof(j), 1, fWrite);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_PACK_SWAP_BYTES, 1);
	glReadPixels(0, 0, i, j,
		GL_BGR, GL_UNSIGNED_BYTE, pixelData);
	//std::cout << (int)pixelData[0] << " " << (int)pixelData[1] << " " << (int)pixelData[1800] << " " << (int)pixelData[1801] << " " << (int)pixelData[1802] << " " << (int)pixelData[1803] << " " << (int)pixelData[1804] << "\n";
	//fseek(fWrite, 0, SEEK_END);
	fwrite(pixelData, dataLength, 1, fWrite);

	//fclose(fDummy);
	fclose(fWrite);
	free(pixelData);
	//delete[] pixelData;

	std::cout << "Screenshot created as " << pth << ".\n";
}

void TextureMapping::exportObject(Object* obj)
{
	std::shared_ptr<Model> model = obj->GetModel();
	std::vector<Vertex>& vertices = model->_vertices;
	std::vector<uint32_t>& indices = model->_indices;
	char obj_path[100];
	sprintf_s(obj_path, "../exported/%s.obj", obj->Name.c_str());
	//std::cout << obj_path << std::endl;
	std::fstream f(obj_path, std::ios::out);

	for (auto& v : vertices)
	{
		f << "v " << v.position.x << " " << v.position.y << " " << v.position.z << std::endl;
	}
	for (auto& v : vertices)
	{
		f << "vt " << v.texCoord.x << " " << v.texCoord.y << std::endl;
	}
	for (auto& v : vertices)
	{
		f << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << std::endl;
	}
	
	for (int i = 0; i < indices.size(); i += 3)
	{
		f << "f " << indices[i] + 1 << " " << indices[i + 1] + 1 << " " << indices[i + 2] + 1 << std::endl;
	}
	f.close();

	std::cout << "Exported .obj file as " << obj_path << ".\n";
}