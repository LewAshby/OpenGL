#pragma once

#include <string>
#include <unordered_map>

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

class ShaderHandler
{
	private:
		std::string m_FilePath;
		unsigned int m_RenderedID;
		std::unordered_map<std::string, int> m_UniformLocationCache;
	public:
		ShaderHandler(const std::string& filepath);
		~ShaderHandler();

		void Bind() const;
		void Unbind() const;

		void setUniform1f(const std::string& name, float value);
		void setUniform1i(const std::string& name, int value);
		void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
		int GetUniformLocation(const std::string& name);
		ShaderProgramSource ParseShader(const std::string&);
		unsigned int CompileShader(unsigned int, const std::string&);
		unsigned int CreateShader(const std::string&, const std::string&);

};
