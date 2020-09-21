#include <glad/glad.h>

#include <array>

#include "line.h"

Line::Line(glm::vec3 startPoint,
           glm::vec3 endPoint)
   : mStartPoint(startPoint)
   , mEndPoint(endPoint)
   , mVAO(0)
   , mVBO(0)
{
   configureVAO(startPoint, endPoint);
}

Line::~Line()
{
   glDeleteVertexArrays(1, &mVAO);
   glDeleteBuffers(1, &mVBO);
}

Line::Line(Line&& rhs) noexcept
   : mStartPoint(std::exchange(rhs.mStartPoint, glm::vec3(0.0f)))
   , mEndPoint(std::exchange(rhs.mEndPoint, glm::vec3(0.0f)))
   , mVAO(std::exchange(rhs.mVAO, 0))
   , mVBO(std::exchange(rhs.mVBO, 0))
{

}

Line& Line::operator=(Line&& rhs) noexcept
{
   mStartPoint = std::exchange(rhs.mStartPoint, glm::vec3(0.0f));
   mEndPoint   = std::exchange(rhs.mEndPoint, glm::vec3(0.0f));
   mVAO        = std::exchange(rhs.mVAO, 0);
   mVBO        = std::exchange(rhs.mVBO, 0);
   return *this;
}

glm::vec3 Line::getStartPoint() const
{
   return mStartPoint;
}

glm::vec3 Line::getEndPoint() const
{
   return mEndPoint;
}

void Line::bindVAO() const
{
   glBindVertexArray(mVAO);
}

void Line::configureVAO(glm::vec3 startPoint, glm::vec3 endPoint)
{
   glGenVertexArrays(1, &mVAO);
   glGenBuffers(1, &mVBO);

   std::array<float, 6> vertices = {startPoint.x, startPoint.y, startPoint.z, endPoint.x, endPoint.y, endPoint.z};

   std::array<unsigned int, 2> indices = {0, 1};

   // Configure the VAO of the line
   glBindVertexArray(mVAO);

   // Load the line's data into the buffers

   // Positions
   glBindBuffer(GL_ARRAY_BUFFER, mVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

   // Set the vertex attribute pointers
   // Positions
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

   glBindVertexArray(0);
}
