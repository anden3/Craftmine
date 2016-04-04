#include "Text.h"

#include <map>
#include <vector>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

struct Character {
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};

struct String {
	std::string Text;

	float X;
	float Y;
	float Scale;
	float Opacity;

	glm::vec3 Color;
};

const int TEXT_TEXTURE_UNIT = 10;

unsigned int textVAO, textVBO;

Shader* textShader;

std::map<char, Character> Characters;
std::map<std::string, String> Strings;

Text::Text(std::string font, int font_size) {
	textShader = new Shader("text");

	FT_Library ft;
	FT_Face face;

	FT_Init_FreeType(&ft);
	FT_New_Face(ft, std::string("fonts/" + font + ".ttf").c_str(), 0, &face);
	FT_Set_Pixel_Sizes(face, 0, font_size);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);

		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cerr << "ERROR::FREETYPE: Failed to load Glyph " << c << std::endl;
			continue;
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
			);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(unsigned int)face->glyph->advance.x
		};

		Characters.insert(std::pair<GLchar, Character>(c, character));
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);

	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	textShader->Bind();

	glm::mat4 projection = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
	glUniformMatrix4fv(glGetUniformLocation(textShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(glGetUniformLocation(textShader->Program, "text"), TEXT_TEXTURE_UNIT);

	textShader->Unbind();
}

Text::~Text() {
	delete textShader;
}

void Text::Add(std::string name, std::string text, float yTop) {
	String string;

	string.Text = text;

	string.X = X;
	string.Y = SCREEN_HEIGHT - yTop;

	string.Scale = Scale;
	string.Color = Color;
	string.Opacity = Opacity;

	Strings[name] = string;
}

void Text::Set_Text(std::string name, std::string newText) {
	Strings[name].Text = newText;
}

void Text::Set_X(std::string name, float x) {
	Strings[name].X = x;
}

void Text::Set_Y(std::string name, float y) {
	Strings[name].Y = y;
}

void Text::Set_Scale(std::string name, float scale) {
	Strings[name].Scale = scale;
}

void Text::Set_Opacity(std::string name, float opacity) {
	Strings[name].Opacity = opacity;
}

void Text::Set_Color(std::string name, glm::vec3 color) {
	Strings[name].Color = color;
}

void Text::Draw(std::string name) {
	if (!Strings.count(name)) return;

	String string = Strings[name];

	textShader->Bind();

	glUniform4f(glGetUniformLocation(textShader->Program, "textColor"), string.Color.x, string.Color.y, string.Color.z, string.Opacity);

	glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
	glBindVertexArray(textVAO);

	std::string::const_iterator c;

	for (c = string.Text.begin(); c != string.Text.end(); c++) {
		Character ch = Characters[*c];

		float xPos = string.X + ch.Bearing.x * string.Scale;
		float yPos = string.Y - (ch.Size.y - ch.Bearing.y) * string.Scale;

		float w = ch.Size.x * string.Scale;
		float h = ch.Size.y * string.Scale;

		float text_vertices[6][4] = {
			{ xPos,     yPos + h,   0.0, 0.0 },
			{ xPos,     yPos,       0.0, 1.0 },
			{ xPos + w, yPos,       1.0, 1.0 },

			{ xPos,     yPos + h,   0.0, 0.0 },
			{ xPos + w, yPos,       1.0, 1.0 },
			{ xPos + w, yPos + h,   1.0, 0.0 }
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(text_vertices), text_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		string.X += (ch.Advance >> 6) * string.Scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	textShader->Unbind();
}

void Text::Draw_All() {
	if (Strings.size() > 0) {
		for (auto const string : Strings) {
			Draw(string.first);
		}
	}
}