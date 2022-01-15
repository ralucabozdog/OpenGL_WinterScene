#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    glm::vec3 Camera::getCameraFrontDirection() {
        return this->cameraFrontDirection;
    }

    glm::vec3 Camera::getCameraUpDirection() {
        return this->cameraUpDirection;
    }

    glm::vec3 Camera::getCameraRightDirection() {
        return this->cameraRightDirection;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    void Camera::setCamera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD) {
            if ((this->cameraPosition + speed * cameraFrontDirection).y > 0.6) {
                this->cameraTarget += speed * cameraFrontDirection;
                this->cameraPosition += speed * cameraFrontDirection;
            }
        }
        else {
            if (direction == MOVE_BACKWARD) {
                if ((this->cameraPosition - speed * cameraFrontDirection).y > 0.6) {
                    this->cameraTarget -= speed * cameraFrontDirection;
                    this->cameraPosition -= speed * cameraFrontDirection;
                }
            }
            else {
                if (direction == MOVE_RIGHT) {
                    if ((this->cameraPosition + speed * glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection))).y > 0.6) {
                        this->cameraTarget += speed * glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
                        this->cameraPosition += speed * glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
                    }
                }
                else {
                    if (direction == MOVE_LEFT) {
                        if ((this->cameraPosition + speed * glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection))).y > 0.6) {
                            this->cameraTarget -= speed * glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
                            this->cameraPosition -= speed * glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
                        }
                    }
                }
            }
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        this->yawAngle += yaw;
        this->pitchAngle += pitch;

        this->cameraTarget.x = this->cameraPosition.x + sin(this->yawAngle) * cos(this->pitchAngle);
        this->cameraTarget.z = this->cameraPosition.z - cos(this->yawAngle) * cos(this->pitchAngle);
        this->cameraTarget.y = this->cameraPosition.y + sin(this->pitchAngle);

        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }
}