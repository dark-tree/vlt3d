
#include "renderer.hpp"
#include "client/vertices.hpp"
#include "buffer/allocator.hpp"
#include "buffer/atlas.hpp"

void ScreenRenderer::getBuffers(Allocator& allocator, Buffer* buf_3d, int* len_3d, Buffer* buf_2d, int* len_2d) {

	std::vector<Vertex3D> mesh_3d;
	std::vector<Vertex2D> mesh_2d;

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

	mesh_3d.emplace_back( ox + 0, oy + 0,  -1,   0,   0, 255,   0,   0, 255);
	mesh_3d.emplace_back( ox + 1, oy + 0,  -1,   1,   0,   0, 255,   0, 255);
	mesh_3d.emplace_back( ox + 1, oy + 1,  -1,   1,   1,   0,   0, 255, 255);

	mesh_3d.emplace_back( ox + 0, oy + 0,  -1,   0,   0, 255,   0,   0, 255);
	mesh_3d.emplace_back( ox + 1, oy + 1,  -1,   1,   1,   0,   0, 255, 255);
	mesh_3d.emplace_back( ox + 0, oy + 1,  -1,   0,   1,   0, 255,   0, 255);

	mesh_2d.emplace_back(0.0, 0.1, 0, 0, 255, 0, 0, 255);
	mesh_2d.emplace_back(0.0, 0.0, 0, 0, 255, 0, 0, 255);
	mesh_2d.emplace_back(0.1, 0.0, 0, 0, 255, 0, 0, 255);

	if ((int) mesh_3d.size() > *len_3d) {
		BufferInfo buffer_builder {mesh_3d.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		if (*len_3d != -1) {
			buf_3d->close();
		}

		*buf_3d = allocator.allocateBuffer(buffer_builder);
		*len_3d = mesh_3d.size() * sizeof(Vertex);
		logger::info("Reallocated 3D immediate buffer");
	}

	MemoryMap map_3d = buf_3d->access().map();
	map_3d.write(mesh_3d.data(), mesh_3d.size() * sizeof(Vertex));
	map_3d.flush();
	map_3d.unmap();

	if ((int) mesh_2d.size() > *len_2d) {
		BufferInfo buffer_builder {mesh_2d.size() * sizeof(Vertex2D), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		if (*len_2d != -1) {
			buf_2d->close();
		}

		*buf_2d = allocator.allocateBuffer(buffer_builder);
		*len_2d = mesh_2d.size() * sizeof(Vertex);
		logger::info("Reallocated 2D immediate buffer");
	}

	MemoryMap map_2d = buf_2d->access().map();
	map_2d.write(mesh_2d.data(), mesh_2d.size() * sizeof(Vertex));
	map_2d.flush();
	map_2d.unmap();

}