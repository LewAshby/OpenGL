#pragma once

#include "Renderer.h"

class Texture
{
	private:
		unsigned int m_RenderedID;
		std::string m_FilePath;
		unsigned char* m_LocalBuffer;
		int m_Width, m_Height, m_BPP;	// m_BPP - bytes per pixel

	public:
		Texture(const std::string& path);
		~Texture();

		void Bind(unsigned int slot = 0) const;
		void UnBind() const;

		inline int GetWith() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
};