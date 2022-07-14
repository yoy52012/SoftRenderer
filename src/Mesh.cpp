#include "Mesh.h"

namespace SoftRenderer
{
	SubMesh::SubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<int>& indices)
	{
		


	}

	void SubMesh::addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		int size = indices.size();
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);

		indices.push_back(size);
		indices.push_back(size + 1);
		indices.push_back(size + 2);
	}

	SubMesh& SubMesh::setPositions(const std::vector<glm::vec3>& positions)
	{
		this->positions = positions;
		flag |= MeshFlags::HasPosition;
		return *this;
	}

	SubMesh& SubMesh::setNormals(const std::vector<glm::vec3>& normals)
	{
		this->normals = normals;
		flag |= MeshFlags::HasNormal;
		return *this;
	}

	SubMesh& SubMesh::setTangents(const std::vector<glm::vec4>& tangents)
	{
		this->tangents = tangents;
		flag |= MeshFlags::HasTangent;
		return *this;
	}

	SubMesh& SubMesh::setColors(const std::vector<glm::vec4>& colors)
	{
		this->colors = colors;
		flag |= MeshFlags::HasColor;
		return *this;
	}

	SubMesh& SubMesh::setUvs(const std::vector<glm::vec2>& uvs)
	{
		this->uvs = uvs;
		flag |= MeshFlags::HasTexcoord;
		return *this;
	}

	SubMesh& SubMesh::setIndices(const std::vector<int>& indices)
	{
		this->indices = indices;
		return *this;
	}

	SubMesh& SubMesh::build()
	{
		if (indices.empty() || positions.empty())
			return SubMesh();

		for (int i = 0; i < positions.size(); i++)
		{
			Vertex vertex;
			vertex.position = positions[i];
			vertices.emplace_back(vertex);
		}

		if ((flag & Mesh::MeshFlags::HasTexcoord) && (positions.size() == uvs.size()))
		{
			for (int i = 0; i < uvs.size(); i++)
			{
				auto& vertex = vertices[i];
				vertex.texcoord = uvs[i];
			}
		}

		if ((flag & Mesh::MeshFlags::HasNormal) && (positions.size() == normals.size()))
		{
			for (int i = 0; i < normals.size(); i++)
			{
				auto& vertex = vertices[i];
				vertex.normal = normals[i];
			}
		}

		if ((flag & Mesh::MeshFlags::HasTangent) && (positions.size() == tangents.size()))
		{
			for (int i = 0; i < tangents.size(); i++)
			{
				auto& vertex = vertices[i];
				vertex.tangent = tangents[i];
			}
		}

		if ((flag & Mesh::MeshFlags::HasColor) && (positions.size() == colors.size()))
		{
			for (int i = 0; i < colors.size(); i++)
			{
				auto& vertex = vertices[i];
				vertex.color = colors[i];
			}
		}

		return *this;
	}

	SubMesh SubMesh::createPlaneMesh()
	{
		glm::vec2 size(2.0f, 2.0f);
		int subdivide_d = 1;
		int subdivide_w = 1;

		int i, j, prevrow, thisrow, point;
		float x, z;

		glm::vec2 start_pos = size * -0.5f;


		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> tangents;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec4> colors;
		
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		point = 0;

		/* top + bottom */
		z = start_pos.y;
		thisrow = point;
		prevrow = 0;
		for (j = 0; j <= (subdivide_d + 1); j++) {
			x = start_pos.x;
			for (i = 0; i <= (subdivide_w + 1); i++) {
				float u = i;
				float v = j;
				u /= (subdivide_w + 1.0);
				v /= (subdivide_d + 1.0);

				glm::vec3 position = glm::vec3(-x, 0.0, -z);
				glm::vec3 normal = glm::vec3(0.0, 1.0, 0.0);
				glm::vec2 uv = glm::vec2(1.0 - u, 1.0 - v); /* 1.0 - uv to match orientation with Quad */
				glm::vec4 tangent  = glm::vec4(1.0, 0.0, 0.0, 1.0);
				glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

				//Vertex vert;
				//vert.position = position;
				//vert.normal = normal;
				//vert.texcoord = uv;
				//vert.color = color;
				//vertices.push_back(vert);

				positions.emplace_back(position);
				uvs.emplace_back(uv);
				normals.emplace_back(normal);
				tangents.emplace_back(tangent);
				colors.emplace_back(color);

				point++;

				if (i > 0 && j > 0) {
					indices.push_back(prevrow + i - 1);
					indices.push_back(prevrow + i);
					indices.push_back(thisrow + i - 1);
					indices.push_back(prevrow + i);
					indices.push_back(thisrow + i);
					indices.push_back(thisrow + i - 1);
				};

				x += size.x / (subdivide_w + 1.0);
			};

			z += size.y / (subdivide_d + 1.0);
			prevrow = thisrow;
			thisrow = point;
		};

		SubMesh mesh;
		mesh.setPositions(positions)
			.setUvs(uvs)
			.setNormals(normals)
			.setTangents(tangents)
			.setColors(colors)
			.setIndices(indices)
			.build();

		//Mesh mesh;
		//mesh.vertices = vertices;
		//mesh.indices = indices;
		//return mesh;

		return mesh;
	}

	SubMesh SubMesh::createPlaneMesh2()
	{
		int width = 1;
		int height = 1;
		int widthSegments = 1;
		int heightSegments = 1;

		const float width_half = width / 2.0f;
		const float height_half = height / 2.0f;

		const int gridX = widthSegments;
		const int gridY = heightSegments;

		const int gridX1 = gridX + 1;
		const int gridY1 = gridY + 1;

		const float segment_width = width / gridX;
		const float segment_height = height / gridY;

		std::vector<int> indices;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;

		for (int iy = 0; iy < gridY1; iy++) {
			float y = iy * segment_height - height_half;

			for (int ix = 0; ix < gridX1; ix++) {
				float x = ix * segment_width - width_half;
				positions.push_back(glm::vec3(x, -y, 0));
				normals.push_back(glm::vec3(0, 0, 1));
				uvs.push_back(glm::vec2(ix / gridX, 1 - (iy / gridY)));
			}
		}

		for (int iy = 0; iy < gridY; iy++) {

			for (int ix = 0; ix < gridX; ix++) {

				int a = ix + gridX1 * iy;
				int b = ix + gridX1 * (iy + 1);
				int c = (ix + 1) + gridX1 * (iy + 1);
				int d = (ix + 1) + gridX1 * iy;

				indices.push_back(d);
				indices.push_back(b);
				indices.push_back(a);

				indices.push_back(d);
				indices.push_back(c);
				indices.push_back(b);
			}
		}

		SubMesh mesh;
		mesh.setPositions(positions)
			.setUvs(uvs)
			.setNormals(normals)
			.setIndices(indices)
			.build();

		return mesh;
	}

	SubMesh SubMesh::createBoxMesh()
	{
		glm::vec3 size(1.0f, 1.0f, 1.0f);
		int subdivide_h = 1;
		int subdivide_w = 1;
		int subdivide_d = 1;

		int i, j, prevrow, thisrow, point;
		float x, y, z;
		float onethird = 1.0 / 3.0;
		float twothirds = 2.0 / 3.0;

		glm::vec3 start_pos = size * -0.5f;

		// set our bounding box

		//std::vector<glm::vec3> positions;
		//std::vector<glm::vec3> normals;
		//std::vector<float> tangents;
		//std::vector<glm::vec2> uvs;
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

		// front + back
		y = start_pos.y;
		thisrow = point;
		prevrow = 0;
		for (j = 0; j <= subdivide_h + 1; j++) {
			x = start_pos.x;
			for (i = 0; i <= subdivide_w + 1; i++) {
				float u = i;
				float v = j;
				u /= (3.0 * (subdivide_w + 1.0));
				v /= (2.0 * (subdivide_h + 1.0));

				// front
				Vertex vertF;
				vertF.position = glm::vec3(x, -y, -start_pos.z); // double negative on the Z!
				vertF.normal = glm::vec3(0.0f, 0.0f, 1.0f);
				//ADD_TANGENT(1.0f, 0.0f, 0.0f, 1.0f);
				vertF.texcoord = glm::vec2(u, v);
				vertices.emplace_back(vertF);
				point++;

				// back
				Vertex vertB;
				vertB.position = glm::vec3(-x, -y, start_pos.z);
				vertB.normal = glm::vec3(0.0, 0.0, -1.0);
				//ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
				vertB.texcoord = glm::vec2(twothirds + u, v);
				vertices.emplace_back(vertB);
				point++;

				if (i > 0 && j > 0) {
					int i2 = i * 2;

					// front
					indices.push_back(prevrow + i2 - 2);
					indices.push_back(prevrow + i2);
					indices.push_back(thisrow + i2 - 2);
					indices.push_back(prevrow + i2);
					indices.push_back(thisrow + i2);
					indices.push_back(thisrow + i2 - 2);

					// back
					indices.push_back(prevrow + i2 - 1);
					indices.push_back(prevrow + i2 + 1);
					indices.push_back(thisrow + i2 - 1);
					indices.push_back(prevrow + i2 + 1);
					indices.push_back(thisrow + i2 + 1);
					indices.push_back(thisrow + i2 - 1);
				};

				x += size.x / (subdivide_w + 1.0);
			};

			y += size.y / (subdivide_h + 1.0);
			prevrow = thisrow;
			thisrow = point;
		};

		// left + right
		y = start_pos.y;
		thisrow = point;
		prevrow = 0;
		for (j = 0; j <= (subdivide_h + 1); j++) {
			z = start_pos.z;
			for (i = 0; i <= (subdivide_d + 1); i++) {
				float u = i;
				float v = j;
				u /= (3.0 * (subdivide_d + 1.0));
				v /= (2.0 * (subdivide_h + 1.0));

				// right
				Vertex vertR;
				vertR.position = glm::vec3(-start_pos.x, -y, -z);
				vertR.normal = glm::vec3(1.0, 0.0, 0.0);
				//ADD_TANGENT(0.0, 0.0, -1.0, 1.0);
				vertR.texcoord = glm::vec2(onethird + u, v);
				vertices.emplace_back(vertR);
				point++;

				// left
				Vertex vertL;
				vertL.position = glm::vec3(start_pos.x, -y, z);
				vertL.normal = glm::vec3(-1.0, 0.0, 0.0);
				//ADD_TANGENT(0.0, 0.0, 1.0, 1.0);
				vertL.texcoord = glm::vec2(u, 0.5 + v);
				vertices.emplace_back(vertL);
				point++;

				if (i > 0 && j > 0) {
					int i2 = i * 2;

					// right
					indices.push_back(prevrow + i2 - 2);
					indices.push_back(prevrow + i2);
					indices.push_back(thisrow + i2 - 2);
					indices.push_back(prevrow + i2);
					indices.push_back(thisrow + i2);
					indices.push_back(thisrow + i2 - 2);

					// left
					indices.push_back(prevrow + i2 - 1);
					indices.push_back(prevrow + i2 + 1);
					indices.push_back(thisrow + i2 - 1);
					indices.push_back(prevrow + i2 + 1);
					indices.push_back(thisrow + i2 + 1);
					indices.push_back(thisrow + i2 - 1);
				};

				z += size.z / (subdivide_d + 1.0);
			};

			y += size.y / (subdivide_h + 1.0);
			prevrow = thisrow;
			thisrow = point;
		};

		// top + bottom
		z = start_pos.z;
		thisrow = point;
		prevrow = 0;
		for (j = 0; j <= (subdivide_d + 1); j++) {
			x = start_pos.x;
			for (i = 0; i <= (subdivide_w + 1); i++) {
				float u = i;
				float v = j;
				u /= (3.0 * (subdivide_w + 1.0));
				v /= (2.0 * (subdivide_d + 1.0));

				// top
				Vertex vertT;
				vertT.position = glm::vec3(-x, -start_pos.y, -z);
				vertT.normal = glm::vec3(0.0, 1.0, 0.0);
				//ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
				vertT.texcoord = glm::vec2(onethird + u, 0.5 + v);
				vertices.emplace_back(vertT);
				point++;

				// bottom
				Vertex vertB;
				vertB.position = glm::vec3(x, start_pos.y, -z);
				vertB.normal = glm::vec3(0.0, -1.0, 0.0);
				//ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
				vertB.texcoord = glm::vec2(twothirds + u, 0.5 + v);
				vertices.emplace_back(vertB);
				point++;

				if (i > 0 && j > 0) {
					int i2 = i * 2;

					// top
					indices.push_back(prevrow + i2 - 2);
					indices.push_back(prevrow + i2);
					indices.push_back(thisrow + i2 - 2);
					indices.push_back(prevrow + i2);
					indices.push_back(thisrow + i2);
					indices.push_back(thisrow + i2 - 2);

					// bottom
					indices.push_back(prevrow + i2 - 1);
					indices.push_back(prevrow + i2 + 1);
					indices.push_back(thisrow + i2 - 1);
					indices.push_back(prevrow + i2 + 1);
					indices.push_back(thisrow + i2 + 1);
					indices.push_back(thisrow + i2 - 1);
				};

				x += size.x / (subdivide_w + 1.0);
			};

			z += size.z / (subdivide_d + 1.0);
			prevrow = thisrow;
			thisrow = point;
		};

		SubMesh mesh;
		mesh.vertices = vertices;
		mesh.indices = indices;
		return mesh;
	}

	SubMesh SubMesh::createBoxMesh2()
	{
		int width = 1;
		int height = 1;
		int depth = 1;
		int widthSegments = 1;
		int heightSegments = 1;
		int depthSegments = 1;

		std::vector<int> indices;
		std::vector<glm::vec3>  vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;

		int numberOfVertices = 0;

		auto buildPlane = [&](int u, int v, int w, float udir, float vdir, int width, int height, int depth, int gridX, int gridY) {
			const int segmentWidth = width / gridX;
			const int segmentHeight = height / gridY;

			const int widthHalf = width / 2;
			const int heightHalf = height / 2;
			const int depthHalf = depth / 2;

			const int gridX1 = gridX + 1;
			const int gridY1 = gridY + 1;

			int vertexCounter = 0;

			glm::vec3 vector;

			for (int iy = 0; iy < gridY1; iy++) {
				const int y = iy * segmentHeight - heightHalf;

				for (int  ix = 0; ix < gridX1; ix++) {

					const int x = ix * segmentWidth - widthHalf;

					// set values to correct vector component
					vector[u] = x * udir;
					vector[v] = y * vdir;
					vector[w] = depthHalf;

					// now apply vector to vertex buffer
					vertices.push_back({ vector.x, vector.y, vector.z });

					// set values to correct vector component
					vector[u] = 0;
					vector[v] = 0;
					vector[w] = depth > 0 ? 1 : -1;

					// now apply vector to normal buffer
					normals.push_back({ vector.x, vector.y, vector.z });

					// uvs
					uvs.push_back({ ix / gridX, 1 - (iy / gridY) });

					// counters
					vertexCounter += 1;
				}
			}

			for (int iy = 0; iy < gridY; iy++) {
				for (int ix = 0; ix < gridX; ix++) {

					int a = numberOfVertices + ix + gridX1 * iy;
					int b = numberOfVertices + ix + gridX1 * (iy + 1);
					int c = numberOfVertices + (ix + 1) + gridX1 * (iy + 1);
					int d = numberOfVertices + (ix + 1) + gridX1 * iy;

					// faces
					indices.push_back(a);
					indices.push_back(b);
					indices.push_back(d);
					indices.push_back(b);
					indices.push_back(c);
					indices.push_back(d);

					// increase counter
					numberOfVertices += vertexCounter;
				}

			}

			// update total number of vertices
			numberOfVertices += vertexCounter;
		};

		buildPlane(2, 1, 0, -1, -1, depth, height, width, depthSegments, heightSegments); // px
		buildPlane(2, 1, 0, 1, -1, depth, height, -width, depthSegments, heightSegments); // nx
		buildPlane(0, 2, 2, 1, 1, width, depth, height, widthSegments, depthSegments); // py
		buildPlane(0, 2, 1, 1, -1, width, depth, -height, widthSegments, depthSegments); // ny
		buildPlane(0, 1, 2, 1, -1, width, height, depth, widthSegments, heightSegments); // pz
		buildPlane(0, 1, 2, -1, -1, width, height, -depth, widthSegments, heightSegments); // nz

		return SubMesh();
	}
}


