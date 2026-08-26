#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera{
    public:
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
        glm::mat4 viewMatrix;
        glm::mat4 perspectiveMatrix;

        Camera(glm::vec3 position, float aspectRatio);

        void update_viewMatrix();
};
