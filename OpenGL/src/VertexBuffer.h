#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	void UpdateData(const void* data, unsigned int size);

private:
	unsigned int m_RenderedID;
};