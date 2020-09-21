#ifndef LINE_H
#define LINE_H

#include <glm/glm.hpp>

class Line
{
public:

   Line(glm::vec3 startPoint,
        glm::vec3 endPoint);
   ~Line();

   Line(const Line&) = delete;
   Line& operator=(const Line&) = delete;

   Line(Line&& rhs) noexcept;
   Line& operator=(Line&& rhs) noexcept;

   glm::vec3 getStartPoint() const;
   glm::vec3 getEndPoint() const;

   void bindVAO() const;

private:

   void configureVAO(glm::vec3 startPoint,
                     glm::vec3 endPoint);

   glm::vec3 mStartPoint;
   glm::vec3 mEndPoint;

   unsigned int mVAO;
   unsigned int mVBO;
};

#endif
