#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <exception>

class Shader
{
public:
    unsigned int ID = 0;

    Shader() { }

    Shader& Use();

    static Shader LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr);
    void Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr);

    void SetFloat(const char* name, float value, bool useShader = false);
    void SetInt(const char* name, int value, bool useShader = false);
    void SetVec2f(const char* name, float x, float y, bool useShader = false);
    void SetVec2f(const char* name, const glm::vec2& value, bool useShader = false);
    void SetVec3f(const char* name, float x, float y, float z, bool useShader = false);
    void SetVec3f(const char* name, const glm::vec3& value, bool useShader = false);
    void SetVec4f(const char* name, float x, float y, float z, float w, bool useShader = false);
    void SetVec4f(const char* name, const glm::vec4& value, bool useShader = false);
    void SetMat4(const char* name, const glm::mat4& matrix, bool useShader = false);
    void SetMat3(const char* name, const glm::mat3& matrix, bool useShader = false);
private:
    void checkCompileErrors(unsigned int object, std::string type);
};