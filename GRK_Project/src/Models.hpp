#include <string>
#include <filesystem>

//models ------------------------------------------------------------------------------------------------------------------------------------------------------------- models
namespace models
{
	//structures
	Core::RenderContext room;
	Core::RenderContext roof;
	Core::RenderContext ceiling;
	Core::RenderContext floor;
	Core::RenderContext sphere;
	Core::RenderContext ground;
	Core::RenderContext walls;
	Core::RenderContext skybox;

	//furnitures
	Core::RenderContext hugeWindow;
	Core::RenderContext smallWindow1;
	Core::RenderContext smallWindow2;
	Core::RenderContext bed;
	Core::RenderContext chair;
	Core::RenderContext desk;
	Core::RenderContext door;
	Core::RenderContext jamb;
	Core::RenderContext drawer;
	Core::RenderContext marbleBust;
	Core::RenderContext mattress;
	Core::RenderContext pencils;
	Core::RenderContext painting;
	Core::RenderContext carpet;
	Core::RenderContext stool;
	Core::RenderContext barbells;
	Core::RenderContext mat;
	Core::RenderContext poster;
	Core::RenderContext tableLamp;
	Core::RenderContext lamp;
	Core::RenderContext switch_;
	Core::RenderContext pillow1;
	Core::RenderContext pillow2;
	Core::RenderContext duvet;
	Core::RenderContext shelf;
	Core::RenderContext books;
	Core::RenderContext book;
	Core::RenderContext openBook;
	Core::RenderContext dogBed;
	Core::RenderContext waterBowl;
	Core::RenderContext foodBowl;
	Core::RenderContext boneToy;
	Core::RenderContext tennisBall;
	Core::RenderContext umbrella;
	Core::RenderContext umbrellaHolder;
	Core::RenderContext hatstand;
	Core::RenderContext hatShelf;
	Core::RenderContext capHat;
	Core::RenderContext highHat;
	Core::RenderContext beret;
	Core::RenderContext jacket;



	//player
	Core::RenderContext fly0;
	Core::RenderContext fly1;
	Core::RenderContext fly2;
	Core::RenderContext fly3;
	Core::RenderContext fly4;
	Core::RenderContext fly5;

	//environment
	Core::RenderContext tree;
	Core::RenderContext bush;
	Core::RenderContext rock;
	Core::RenderContext dog;
	Core::RenderContext dogTail;

}

//textures ----------------------------------------------------------------------------------------------------------------------------------------------------- textures
namespace textures
{
	//structures
	GLuint room;
	GLuint roof;
	GLuint ceiling;
	GLuint floor;
	GLuint sphere;
	GLuint ground;
	GLuint walls;
	GLuint skybox;

	//furnitures
	GLuint hugeWindow;
	GLuint smallWindow1;
	GLuint smallWindow2;
	GLuint bed;
	GLuint chair;
	GLuint desk;
	GLuint door;
	GLuint jamb;
	GLuint drawer;
	GLuint marbleBust;
	GLuint mattress;
	GLuint pencils;
	GLuint painting;
	GLuint carpet;
	GLuint stool;
	GLuint barbells;
	GLuint mat;
	GLuint poster;
	GLuint tableLamp;
	GLuint lamp;
	GLuint switch_;
	GLuint pillow1;
	GLuint pillow2;
	GLuint duvet;
	GLuint shelf;
	GLuint books;
	GLuint book;
	GLuint openBook;
	GLuint dogBed;
	GLuint waterBowl;
	GLuint foodBowl;
	GLuint boneToy;
	GLuint tennisBall;
	GLuint umbrella;
	GLuint umbrellaHolder;
	GLuint hatstand;
	GLuint hatShelf;
	GLuint capHat;
	GLuint highHat;
	GLuint beret;
	GLuint jacket;

	//player
	GLuint fly0;
	GLuint fly1;
	GLuint fly2;
	GLuint fly3;
	GLuint fly4;
	GLuint fly5;

	//environment
	GLuint tree;
	GLuint bush;
	GLuint rock;
	GLuint dog;
	GLuint dogTail;
}

//paths ----------------------------------------------------------------------------------------------------------------------------------------------------------path
namespace objects_paths
{
	//structures
	std::string room = "./models/structures/room";
	std::string roof = "./models/structures/roof";
	std::string ceiling = "./models/structures/ceiling";
	std::string floor = "./models/structures/floor";
	std::string sphere = "./models/structures/sphere";
	std::string ground = "./models/structures/ground";
	std::string walls = "./models/structures/walls";
	std::string skybox = "./models/structures/skybox";

	//furnitures
	std::string hugeWindow = "./models/furnitures/huge_window";
	std::string smallWindow1 = "./models/furnitures/small_window_1";
	std::string smallWindow2 = "./models/furnitures/small_window_2";
	std::string bed = "./models/furnitures/bed";
	std::string chair = "./models/furnitures/chair";
	std::string desk = "./models/furnitures/desk";
	std::string door = "./models/furnitures/door";
	std::string jamb = "./models/furnitures/jamb";
	std::string drawer = "./models/furnitures/drawer";;
	std::string marbleBust = "./models/furnitures/marable_bust";
	std::string mattress = "./models/furnitures/mattress";
	std::string pencils = "./models/furnitures/pencils";
	std::string painting = "./models/furnitures/painting";
	std::string carpet = "./models/furnitures/carpet";
	std::string stool = "./models/furnitures/stool";
	std::string barbells = "./models/furnitures/barbells";
	std::string mat = "./models/furnitures/mat";
	std::string poster = "./models/furnitures/poster";
	std::string tableLamp = "./models/furnitures/table_lamp";
	std::string lamp = "./models/furnitures/lamp";
	std::string switch_ = "./models/furnitures/switch";
	std::string pillow1 = "./models/furnitures/pillow1";
	std::string pillow2 = "./models/furnitures/pillow2";
	std::string duvet = "./models/furnitures/duvet";
	std::string shelf = "./models/furnitures/shelf";
	std::string books = "./models/furnitures/books";
	std::string book = "./models/furnitures/book";
	std::string openBook = "./models/furnitures/open_book";
	std::string dogBed = "./models/furnitures/dog_bed";
	std::string waterBowl = "./models/furnitures/water_bowl";
	std::string foodBowl = "./models/furnitures/food_bowl";
	std::string boneToy = "./models/furnitures/bone_toy";
	std::string tennisBall = "./models/furnitures/tennis_ball";
	std::string umbrella = "./models/furnitures/umbrella";
	std::string umbrellaHolder = "./models/furnitures/umbrella_holder";
	std::string hatstand = "./models/furnitures/hatstand";
	std::string hatShelf = "./models/furnitures/hat_shelf";
	std::string capHat = "./models/furnitures/cap_hat";
	std::string highHat = "./models/furnitures/high_hat";
	std::string beret = "./models/furnitures/beret";
	std::string jacket = "./models/furnitures/jacket";


	//player
	std::string fly0 = "./models/player/fly0";
	std::string fly1 = "./models/player/fly1";
	std::string fly2 = "./models/player/fly2";
	std::string fly3 = "./models/player/fly3";
	std::string fly4 = "./models/player/fly4";
	std::string fly5 = "./models/player/fly5";

	//environment
	std::string tree = "./models/environment/tree";
	std::string bush = "./models/environment/bush";
	std::string rock = "./models/environment/rock";
	std::string dog = "./models/environment/dog";
	std::string dogTail = "./models/environment/dog_tail";
}

//get certain element path ------------------------------------------------------------------------------------------------------------------ get certain element path

std::string getModelPath(std::string path)
{
	std::string model_path = "";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string file = entry.path().generic_u8string();
		if (file.substr(file.length() - 4, file.length()) == ".obj")
		{
			model_path = file;
		}
	}

	return model_path;
}

std::string getMaterialPath(std::string path)
{
	std::string material_path = "";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string file = entry.path().generic_u8string();
		if (file.substr(file.length() - 4, file.length()) == ".mtl")
		{
			material_path = file;
		}
	}

	return material_path;
}

std::string getTexturePath(std::string path)
{
	std::string texture_path = "";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string file = entry.path().generic_u8string();
		if (file.substr(file.length() - 4, file.length()) == ".png")
		{
			texture_path = file;
		}
	}

	return texture_path;
}


//load models and their textures ------------------------------------------------------------------------------------------------------------ load models and their textures
void loadModelToContext(std::string pathObject, Core::RenderContext& context, GLuint& texture)
{
	Assimp::Importer import;

	const aiScene* scene = import.ReadFile(getModelPath(pathObject), aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);

	if (getTexturePath(pathObject) != "")
	{
		texture = Core::LoadTexture(getTexturePath(pathObject).c_str());
	}
}

//load skybox and it's texture -------------------------------------------------------------------------------------------------------------- load skybox and it's texture
void loadSkyboxWithTextures(std::string pathObject, Core::RenderContext& context)
{
	loadModelToContext(pathObject, context, textures::skybox);

	int w, h;

	glGenTextures(1, &textures::skybox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures::skybox);

	const char* filepaths[6] = {
		"models/structures/skybox/textures/side1.png",
		"models/structures/skybox/textures/side2.png",
		"models/structures/skybox/textures/top.png",
		"models/structures/skybox/textures/bottom.png",
		"models/structures/skybox/textures/side3.png",
		"models/structures/skybox/textures/side4.png"
	};
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* image = SOIL_load_image(filepaths[i], &w, &h, 0, SOIL_LOAD_RGBA);
		if (image) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
			);
		}
		else {
			std::cout << "Failed to load texture: " << filepaths[i] << std::endl;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

//init loading ----------------------------------------------------------------------------------------------------------------------------------------- init loading
void loadAllModels()
{
	//load structures and their textures
	loadModelToContext(objects_paths::sphere, models::sphere, textures::sphere);
	loadModelToContext(objects_paths::room, models::room, textures::room);
	loadModelToContext(objects_paths::roof, models::roof, textures::roof);
	loadModelToContext(objects_paths::ceiling, models::ceiling, textures::ceiling);
	loadModelToContext(objects_paths::floor, models::floor, textures::floor);
	loadModelToContext(objects_paths::ground, models::ground, textures::ground);
	loadModelToContext(objects_paths::walls, models::walls, textures::walls);

	//load furnitures and their textures
	loadModelToContext(objects_paths::bed, models::bed, textures::bed);
	loadModelToContext(objects_paths::chair, models::chair, textures::chair);
	loadModelToContext(objects_paths::desk, models::desk, textures::desk);
	loadModelToContext(objects_paths::door, models::door, textures::door);
	loadModelToContext(objects_paths::jamb, models::jamb, textures::jamb);
	loadModelToContext(objects_paths::drawer, models::drawer, textures::drawer);
	loadModelToContext(objects_paths::marbleBust, models::marbleBust, textures::marbleBust);
	loadModelToContext(objects_paths::mattress, models::mattress, textures::mattress);
	loadModelToContext(objects_paths::pencils, models::pencils, textures::pencils);
	loadModelToContext(objects_paths::hugeWindow, models::hugeWindow, textures::hugeWindow);
	loadModelToContext(objects_paths::smallWindow1, models::smallWindow1, textures::smallWindow1);
	loadModelToContext(objects_paths::smallWindow2, models::smallWindow2, textures::smallWindow2);
	loadModelToContext(objects_paths::painting, models::painting, textures::painting);
	loadModelToContext(objects_paths::carpet, models::carpet, textures::carpet);
	loadModelToContext(objects_paths::stool, models::stool, textures::stool);
	loadModelToContext(objects_paths::barbells, models::barbells, textures::barbells);
	loadModelToContext(objects_paths::mat, models::mat, textures::mat);
	loadModelToContext(objects_paths::poster, models::poster, textures::poster);
	loadModelToContext(objects_paths::tableLamp, models::tableLamp, textures::tableLamp);
	loadModelToContext(objects_paths::lamp, models::lamp, textures::lamp);
	loadModelToContext(objects_paths::switch_, models::switch_, textures::switch_);
	loadModelToContext(objects_paths::pillow1, models::pillow1, textures::pillow1);
	loadModelToContext(objects_paths::pillow2, models::pillow2, textures::pillow2);
	loadModelToContext(objects_paths::duvet, models::duvet, textures::duvet);
	loadModelToContext(objects_paths::shelf, models::shelf, textures::shelf);
	loadModelToContext(objects_paths::books, models::books, textures::books);
	loadModelToContext(objects_paths::book, models::book, textures::book);
	loadModelToContext(objects_paths::openBook, models::openBook, textures::openBook);
	loadModelToContext(objects_paths::dogBed, models::dogBed, textures::dogBed);
	loadModelToContext(objects_paths::waterBowl, models::waterBowl, textures::waterBowl);
	loadModelToContext(objects_paths::foodBowl, models::foodBowl, textures::foodBowl);
	loadModelToContext(objects_paths::boneToy, models::boneToy, textures::boneToy);
	loadModelToContext(objects_paths::tennisBall, models::tennisBall, textures::tennisBall);
	loadModelToContext(objects_paths::umbrella, models::umbrella, textures::umbrella);
	loadModelToContext(objects_paths::umbrellaHolder, models::umbrellaHolder, textures::umbrellaHolder);
	loadModelToContext(objects_paths::hatstand, models::hatstand, textures::hatstand);
	loadModelToContext(objects_paths::hatShelf, models::hatShelf, textures::hatShelf);
	loadModelToContext(objects_paths::capHat, models::capHat, textures::capHat);
	loadModelToContext(objects_paths::highHat, models::highHat, textures::highHat);
	loadModelToContext(objects_paths::beret, models::beret, textures::beret);
	loadModelToContext(objects_paths::jacket, models::jacket, textures::jacket);


	//load player and his texture
	loadModelToContext(objects_paths::fly0, models::fly0, textures::fly0);
	loadModelToContext(objects_paths::fly1, models::fly1, textures::fly1);
	loadModelToContext(objects_paths::fly2, models::fly2, textures::fly2);
	loadModelToContext(objects_paths::fly3, models::fly3, textures::fly3);
	loadModelToContext(objects_paths::fly4, models::fly4, textures::fly4);
	loadModelToContext(objects_paths::fly5, models::fly5, textures::fly5);

	//load skybox and it's textures
	loadSkyboxWithTextures(objects_paths::skybox, models::skybox);

	//load environment objects and their textures 
	loadModelToContext(objects_paths::tree, models::tree, textures::tree);
	loadModelToContext(objects_paths::bush, models::bush, textures::bush);
	loadModelToContext(objects_paths::rock, models::rock, textures::rock);
	loadModelToContext(objects_paths::dog, models::dog, textures::dog);
	loadModelToContext(objects_paths::dogTail, models::dogTail, textures::dogTail);
}
