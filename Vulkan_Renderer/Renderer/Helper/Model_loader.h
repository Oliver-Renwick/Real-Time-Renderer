#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Initializer.h"
#include "Renderer/Helper/Texture.h"
#include "Renderer/Helper/Vulkan_Buffer.h"

namespace tde
{
	class Vulkan_gltfModel
	{
	public:
		VkQueue copyQueue;

		const Renderer* m_renderer;

		//Vertex Layout
		struct Vertex
		{
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec3 color;
			glm::vec4 tangent;
		};
		//single vertex buffer for all primitives
		struct
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
		}vertices;

		//Single index buffer for all Primitives
		struct
		{
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		}indices;

		// The following structures roughly represent the glTF scene structure
		// To keep things simple, they only contain those properties that are required for this sample
		struct Node;

		//Primitive contains data for single draw call
		struct Primitive
		{
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t materialIndex;
		};

		struct Mesh
		{
			std::vector<Primitive> primitives;
		};

		// A node represents an object in the glTF scene graph
		struct Node
		{
			Node* parent = nullptr;
			std::vector<Node*> children;
			Mesh mesh;
			glm::mat4 matrix;
			std::string name;
			~Node()
			{
				for (auto& child : children)
					delete child;
			}
		};

		// A glTF material stores information in e.g. the texture that is attached to it and colors
		struct Material
		{
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			uint32_t baseColorTextureIndex;
			uint32_t normalTextureIndex;
			std::string alphaMode = "OPAQUE";
			float alphaCutOff;
			bool doubleSided = false;
			VkDescriptorSet descriptorSet;
			VkPipeline pipeline;
		};

		// Contains the texture for a single glTF image
		// Images may be reused by texture objects and are as such separated
		struct Image
		{
			Texture2D texture;
		};

		// A glTF texture stores a reference to the image and a sampler
		// In this sample, we are only interested in the image
		struct Texture
		{
			int32_t image_index;
		};

		/*

		Model Data

		*/

		std::vector<Image> g_images;
		std::vector<Material> g_materials;
		std::vector<Texture> g_Textures;
		std::vector<Node*> nodes;

		std::vector<uint32_t> IndexBuffer;	
		std::vector<Vertex> VertexBuffer;

		std::string path;

		/*
			Gltf Loading Functions
			The following functions take a glTF input model loaded via tinyglTF and convert all required data into our own structure
		*/
		VkDescriptorImageInfo getTextureDescriptor(const size_t index);
		void LoadImages(tinygltf::Model& input);
		void LoadTextures(tinygltf::Model& input);
		void LoadMaterial(tinygltf::Model& input);
		void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, Vulkan_gltfModel::Node* parent,
			std::vector<uint32_t>& indexBuffer, std::vector<Vulkan_gltfModel::Vertex>& vertexBuffer);
		void draw_node(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Vulkan_gltfModel::Node* node);
		void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);


		void LoadFromFile(std::string filename, const Renderer* renderer);

	};

}