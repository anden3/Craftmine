#include <string>

#include <glm/glm.hpp>

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

class Text {
public:
	float X = 0;
	float Scale = 1.0f;
	glm::vec3 Color = glm::vec3(1.0f);

	Text(std::string font, int font_size);
	~Text();

	void Add(std::string name, std::string text, float yTop);

	void Set_Text(std::string name, std::string newText);
	void Set_X(std::string name, float x);
	void Set_Y(std::string name, float y);
	void Set_Scale(std::string name, float scale);
	void Set_Color(std::string name, glm::vec3 color);

	void Draw(std::string name);
	void Draw_All();
};