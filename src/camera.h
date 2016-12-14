#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = 180.0f;
const float PITCH = 0.0f;
const float SPEED = 3.0f;
const float SENSITIVTY = 0.25f;
const float ZOOM = 45.0f;

class Camera {
public:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	// Eular Angles

	float yaw;
	float pitch;

	// Camera options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

	// Constructor with vectors
	Camera(
		glm::vec3 position = glm::vec3(0.0f, 200.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW,
		float pitch = PITCH) :
			front(glm::vec3(1.0f, 0.0f, 0.0f)),
			movementSpeed(SPEED),
			mouseSensitivity(SENSITIVTY),
			zoom(ZOOM) {

		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->Update();
	}

	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM) {
		this->position = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->Update();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix() {
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime = 1.0f) {
		float velocity = this->movementSpeed * deltaTime;

		if (direction == FORWARD) {
			this->position += this->front * velocity;
		}
		if (direction == BACKWARD) {
			this->position -= this->front * velocity;
		}
		if (direction == LEFT) {
			this->position -= this->right * velocity;
		}
		if (direction == RIGHT) {
			this->position += this->right * velocity;
		}
		if (direction == UP) {
			this->position += this->up * velocity;
		}
		if (direction == DOWN) {
			this->position -= this->up * velocity;
		}
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xOffset, float yOffset, float constrainPitch = true) {

		xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;

		// Make sure when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch) {
			if (this->pitch > 89.0f) {
				this->pitch = 89.0f;
			}
			if (this->pitch < -89.0f) {
				this->pitch = -89.0f;
			}
		}

		// Update the camera
		this->Update();
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void Update() {

		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}
};
