#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "Model_loader.h"
namespace tde
{
	void Vulkan_gltfModel::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, Vulkan_gltfModel::Node* parent,
		std::vector<uint32_t>& indexBuffer, std::vector<Vulkan_gltfModel::Vertex>& vertexBuffer)
	{
		Vulkan_gltfModel::Node* node = new Vulkan_gltfModel::Node{};
		node->parent = parent;
		node->name = inputNode.name;
		// Get the local node matrix
		// It's either made up from translation, rotation, scale or a 4x4 matrix

		node->matrix = glm::mat4(1.0f);
		if (inputNode.translation.size() == 3)
		{
			node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}
		if (inputNode.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->matrix *= glm::mat4(q);
		}

		if (inputNode.scale.size() == 3)
		{
			node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}

		if (inputNode.matrix.size() == 16)
		{
			node->matrix = glm::make_mat4x4(inputNode.matrix.data());
		}

		if (inputNode.children.size() > 0)
		{
			for (int i = 0; i < inputNode.children.size(); i++)
			{
				LoadNode(inputModel.nodes[inputNode.children[i]], inputModel, node, indexBuffer, vertexBuffer);
			}
		}

		// If the node contains mesh data, we load vertices and indices from the buffers
		// In glTF this is done via accessors and buffer views
		if (inputNode.mesh > -1)
		{
			const tinygltf::Mesh mesh = inputModel.meshes[inputNode.mesh];

			for (int i = 0; i < mesh.primitives.size(); i++)
			{
				const tinygltf::Primitive& gltfprimitive = mesh.primitives[i];
				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;

				//Vertices
				{
					const float* PositionBuffer = nullptr;
					const float* NormalBuffer = nullptr;
					const float* texCoordBuffer = nullptr;
					const float* tangentBuffer = nullptr;
					uint32_t vertexCount = 0;

					//Get Buffer Data Vertex Position
					if (gltfprimitive.attributes.find("POSITION") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfprimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];
						PositionBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[view.byteOffset + accessor.byteOffset]));
						vertexCount = accessor.count;
					}
					//Get Buffer Data Vertex NORMAL
					if (gltfprimitive.attributes.find("NORMAL") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfprimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];
						NormalBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					//Get Buffer Data Vertex TEXTURE
					if (gltfprimitive.attributes.find("TEXCOORD_0") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfprimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];
						texCoordBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					if (gltfprimitive.attributes.find("TANGENT") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfprimitive.attributes.find("TANGENT")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];
						tangentBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					for (size_t j = 0; j < vertexCount; j++)
					{
						Vertex vert{};
						vert.pos = glm::vec4(glm::make_vec3(&PositionBuffer[j * 3]), 1.0f);
						vert.uv = texCoordBuffer ? glm::make_vec2(&texCoordBuffer[j * 2]) : glm::vec2(0.0f);
						vert.normal = glm::normalize(NormalBuffer ? glm::make_vec3(&NormalBuffer[j * 3]) : glm::vec3(0.0f));
						vert.color = glm::vec3(1.0f);
						vert.tangent = tangentBuffer ? glm::make_vec4(&tangentBuffer[j * 4]) : glm::vec4(0.0f); 
						vertexBuffer.push_back(vert);
					}
				}

				//INDICES
				{
					const tinygltf::Accessor& accessor = inputModel.accessors[gltfprimitive.indices];
					const tinygltf::BufferView& bufferView = inputModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType)
					{

					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}

				Primitive _primitive{};
				_primitive.firstIndex = firstIndex;
				_primitive.indexCount = indexCount;
				_primitive.materialIndex = gltfprimitive.material;
				node->mesh.primitives.push_back(_primitive);
			}
		}

		if (parent)
		{
			parent->children.push_back(node);
		}
		else
		{
			nodes.push_back(node);
		}

	}

	void Vulkan_gltfModel::LoadMaterial(tinygltf::Model& input)
	{
		g_materials.resize(input.materials.size());
		for (size_t i = 0; i < input.materials.size(); i++)
		{
			tinygltf::Material gltf_Material = input.materials[i];

			//Base Color Factor
			if (gltf_Material.values.find("baseColorFactor") != gltf_Material.values.end())
			{
				g_materials[i].baseColorFactor = glm::make_vec4(gltf_Material.values["baseColorFactor"].ColorFactor().data());
			}
			// Base Color Texture Index
			if (gltf_Material.values.find("baseColorTexture") != gltf_Material.values.end())
			{
				g_materials[i].baseColorTextureIndex = gltf_Material.values["baseColorTexture"].TextureIndex();
			}
			//Get the normal map Texture Index
			if (gltf_Material.additionalValues.find("normalTexture") != gltf_Material.additionalValues.end())
			{
				g_materials[i].normalTextureIndex = gltf_Material.additionalValues["normalTexture"].TextureIndex();
			}

			g_materials[i].alphaMode = gltf_Material.alphaMode;
			g_materials[i].doubleSided = gltf_Material.doubleSided;
			g_materials[i].alphaCutOff = (float)gltf_Material.alphaCutoff;

		}
	}

	void Vulkan_gltfModel::LoadImages(tinygltf::Model& input)
	{
		// POI: The textures for the glTF file used in this sample are stored as external ktx files, so we can directly load them from disk without the need for conversion
		g_images.resize(input.images.size());
		for (int i = 0; i < input.images.size(); i++)
		{
			tinygltf::Image& gltfImage = input.images[i];
			g_images[i].texture.loadFromFile(path + "/" + gltfImage.uri, VK_FORMAT_R8G8B8A8_SRGB, m_renderer);
		}

	}

	VkDescriptorImageInfo Vulkan_gltfModel::getTextureDescriptor(const size_t index)
	{
		return g_images[index].texture.m_descriptorimageInfo;
	}

	void Vulkan_gltfModel::LoadTextures(tinygltf::Model& input)
	{
		g_Textures.resize(input.textures.size());
		for (size_t i = 0; i < input.textures.size(); i++)
		{
			g_Textures[i].image_index = input.textures[i].source;
		}
	}


	void Vulkan_gltfModel::draw_node(VkCommandBuffer commandBuffer, VkPipelineLayout layout, Vulkan_gltfModel::Node* node)
	{
		if (node->mesh.primitives.size() > 0)
		{
			glm::mat4 nodeMatrix = node->matrix;

			Vulkan_gltfModel::Node* parentNode = node->parent;
			while (parentNode)
			{
				nodeMatrix = parentNode->matrix * nodeMatrix;
				parentNode = parentNode->parent;
			}

			vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
			for (Vulkan_gltfModel::Primitive& primitive : node->mesh.primitives)
			{
				if (primitive.indexCount > 0)
				{
					// Get the texture index for this primitive
					Vulkan_gltfModel::Material& material = g_materials[primitive.materialIndex];
					if (material.pipeline)
					{
						vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.pipeline);
					}
					if (material.descriptorSet)
					{
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &material.descriptorSet, 0, nullptr);
					}
					vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
				}
			}
		}

		for (auto child : node->children)
		{
			draw_node(commandBuffer, layout, child);
		}
	}

	void Vulkan_gltfModel::LoadFromFile(std::string filename, const Renderer* renderer)
	{
		this->m_renderer = renderer;

		tinygltf::Model inputModel;
		tinygltf::TinyGLTF tinyGltfContext;
		std::string err, warning;

		bool fileLoaded = tinyGltfContext.LoadASCIIFromFile(&inputModel, &err, &warning, filename);
		std::cout << "Error : " << err << std::endl;
		std::cout << "Warning" << warning << std::endl;

		size_t pos = filename.find_last_of("/");
		path = filename.substr(0, pos);

		std::cout << "Mask Mode Path : " << path << std::endl;

		if (fileLoaded)
		{
			LoadImages(inputModel);
			LoadMaterial(inputModel);
			LoadTextures(inputModel);

			const tinygltf::Scene& scene = inputModel.scenes[0];
			for (int i = 0; i < scene.nodes.size(); i++)
			{
				tinygltf::Node& node = inputModel.nodes[scene.nodes[i]];
				LoadNode(node, inputModel, nullptr, IndexBuffer, VertexBuffer);
			}
		}

		else
		{
			throw std::runtime_error("Unable to load the Gltf File");
		}

		 /*Vertex Buffer*/
		{
			void* data;
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingmemory;

			VkDeviceSize Vertexbuffersize = VertexBuffer.size() * sizeof(Vulkan_gltfModel::Vertex);

			createBuffer(Vertexbuffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingmemory, m_renderer);

			vkMapMemory(m_renderer->getDevice(), stagingmemory, 0, Vertexbuffersize, 0, &data);
			memcpy(data, VertexBuffer.data(), (size_t)Vertexbuffersize);
			vkUnmapMemory(m_renderer->getDevice(), stagingmemory);

			createBuffer(Vertexbuffersize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				vertices.buffer, vertices.memory, m_renderer);

			m_renderer->copyBuffer(stagingBuffer, vertices.buffer, Vertexbuffersize);

			/*Destruction of Vertex Staging Buffer*/
			vkDestroyBuffer(m_renderer->getDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_renderer->getDevice(), stagingmemory, nullptr);
		}

		{
			void* data;
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VkDeviceSize IndexbufferSize = sizeof(uint32_t) * IndexBuffer.size();
			indices.count = static_cast<uint32_t>(IndexBuffer.size());


			createBuffer(IndexbufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, m_renderer);

			vkMapMemory(m_renderer->getDevice(), stagingBufferMemory, 0, IndexbufferSize, 0, &data);
			memcpy(data, IndexBuffer.data(), (size_t)IndexbufferSize);
			vkUnmapMemory(m_renderer->getDevice(), stagingBufferMemory);

			createBuffer(IndexbufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.buffer, indices.memory, m_renderer);

			m_renderer->copyBuffer(stagingBuffer, indices.buffer, IndexbufferSize);

			vkDestroyBuffer(m_renderer->getDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_renderer->getDevice(), stagingBufferMemory, nullptr);
		}


	}

	void Vulkan_gltfModel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout)
	{
		VkDeviceSize offset[1] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offset);
		vkCmdBindIndexBuffer(commandBuffer, indices.buffer, offset[0], VK_INDEX_TYPE_UINT32);

		for (auto& node : nodes)
		{
			draw_node(commandBuffer, layout, node);
		}
	}

}		