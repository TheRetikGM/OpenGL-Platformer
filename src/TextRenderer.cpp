#include "TextRenderer.h"
#include <iostream>
#include <exception>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include "config.h"
#include FT_FREETYPE_H

#include "resource_manager.h"

TextRenderer::TextRenderer(unsigned int width, unsigned int height)
{
    this->TextShader = ResourceManager::LoadShader(SHADERS_DIR "text_2d.vert", SHADERS_DIR "text_2d.frag", nullptr, "text");
    this->TextShader.SetMat4("projection", glm::ortho(0.0f, (float)width, (float)height, 0.0f), true);
    this->TextShader.SetInt("textBitmap", 0);

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::Load(std::string font, unsigned int fontSize)
{
    this->Characters.clear();
    this->Errors.clear();

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("Could not init FreeType Library.");
    
    FT_Face face;
    if(FT_New_Face(ft, font.c_str(), 0, &face))
        throw std::runtime_error("Failed to load font '" + font + "'.");
    
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            this->Errors.push_back("Failed to load Glyph. C = " + std::to_string(c));
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED, 
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}
void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    this->TextShader.Use();
    this->TextShader.SetVec3f("TextColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(this->VAO);

    float x_orig = x;

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        if (*c == '\n')
        {
            x = x_orig;
            y += (this->Characters['H'].Size.y + RowSpacing) * scale;
            continue;
        }

        Character ch = this->Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f},
            { xpos + w, ypos,       1.0f, 0.0f},
            { xpos,     ypos,       0.0f, 0.0f},
            
            { xpos,     ypos + h,   0.0f, 1.0f},
            { xpos + w, ypos + h,   1.0f, 1.0f},
            { xpos + w, ypos,       1.0f, 0.0f}
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}