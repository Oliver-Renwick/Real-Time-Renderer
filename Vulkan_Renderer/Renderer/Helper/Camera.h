#pragma once
#pragma once

/*To -: Movement Controls for KeyBoard */

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Helper/Inputs/Input_Sytem.h"
#include <iostream>

namespace tde
{
	class Camera
	{
	public:
		struct
		{
			glm::mat4 view;
			glm::mat4 perspective;
		}_matrices;

		enum CameraType { Mouse, Keyboard };

		CameraType type = CameraType::Mouse;

		glm::vec3 m_position = glm::vec3();
		glm::vec3 m_rotation = glm::vec3();
		glm::vec3 UpDir{ 0.0f, -1.0f, 0.0f };



		glm::vec3 m_Target = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::vec4 viewPos = glm::vec4();

		float m_movementSpeed = 1.0;
		float m_rotationSpeed = 50.0;

		float m_fov = 0.0;
		float m_aspect = 0.0;
		float m_znear = 0.0;
		float m_zfar = 0.0;


		void UpdateViewMatrix()
		{
			
			if (type == Camera::Keyboard)
			{


				glm::mat4 rot_mat = glm::mat4(1.0f);
				glm::mat4 trans_mat;

				rot_mat = glm::rotate(rot_mat, glm::radians(m_rotation.x * -1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				rot_mat = glm::rotate(rot_mat, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				rot_mat = glm::rotate(rot_mat, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

				trans_mat = glm::translate(glm::mat4(1.0f), m_position);

				if (type == Camera::Keyboard)
				{
					_matrices.view = rot_mat * trans_mat;
				}
				else
				{
					_matrices.view = trans_mat * rot_mat;
				}

				viewPos = glm::vec4(m_position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

			}
		}

		void Update(float deltaTime)
		{
			if (CameraType::Keyboard)
			{
				if (moving())
				{
					glm::vec3 camFront{};

					camFront.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
					camFront.y = sin(glm::radians(m_rotation.x));
					camFront.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));

					camFront = glm::normalize(camFront);

					float moveSpeed = m_movementSpeed * deltaTime;

					if (keys.forward)
					{
						m_position += camFront * moveSpeed; 
					}
					if (keys.backward)
					{
						m_position -= camFront * moveSpeed; 

					}
					if (keys.right)
					{
						m_position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed * 2.0f; 

					}
					if (keys.left)
					{
						m_position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed * 2.0f; 

					}
					if (keys.up)
					{
						m_position += UpDir * moveSpeed;
					}
					if (keys.down)
					{
						m_position -= UpDir * moveSpeed;
					}
				}
				if (rotating())
				{
					glm::vec3 camFront{};

					camFront.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
					camFront.y = sin(glm::radians(m_rotation.x));
					camFront.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));

					camFront = glm::normalize(camFront);

					float rotationSpeed = m_rotationSpeed * deltaTime;

					if (keys.rot_up)
					{
						m_rotation.x += rotationSpeed;
					}
					if (keys.rot_down)
					{
						m_rotation.x -= rotationSpeed;
					}
					if (keys.rot_left)
					{
						m_rotation.y -= rotationSpeed;
					}
					if (keys.rot_right)
					{
						m_rotation.y += rotationSpeed;
					}
				}
			}
			UpdateViewMatrix();
		}

		void setPrespective(float fov, float aspect, float znear, float zfar)
		{
			this->m_fov = fov;
			this->m_aspect = aspect;
			this->m_znear = znear;
			this->m_zfar = zfar;

			_matrices.perspective = glm::perspective(m_fov, m_aspect, m_znear, m_zfar);

			_matrices.perspective[1][1] *= -1;
		}


		void SetPosition(glm::vec3 pos)
		{
			m_position = pos;
		}

		void SetRotation(glm::vec3 rot)
		{
			m_rotation = rot;
		}

		void SetMovementSpeed(float movespeed)
		{
			m_movementSpeed = movespeed;
		}

		void SetRotationSpeed(float rotspeed)
		{
			m_rotationSpeed = rotspeed;
		}
	};

}