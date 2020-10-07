#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <array>
#include <random>

#include "play_state.h"

PlayState::PlayState(const std::shared_ptr<FiniteStateMachine>&     finiteStateMachine,
                     const std::shared_ptr<Window>&                 window,
                     const std::shared_ptr<Camera>&                 camera,
                     const std::shared_ptr<Shader>&                 gameObject3DShader,
                     const std::shared_ptr<Shader>&                 lineShader,
                     const std::shared_ptr<GameObject3D>&           table,
                     const std::shared_ptr<GameObject3D>&           teapot)
   : mFSM(finiteStateMachine)
   , mWindow(window)
   , mCamera(camera)
   , mGameObject3DShader(gameObject3DShader)
   , mLineShader(lineShader)
   , mTable(table)
   , mTeapot(teapot)
   , mWorldXAxis(glm::vec3(0.0f), glm::vec3(20.0f, 0.0f, 0.0f), glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)) // Red
   , mWorldYAxis(glm::vec3(0.0f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)) // Green
   , mWorldZAxis(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) // Blue

   , mLocalXAxis(glm::vec3(0.0f), glm::vec3(16.0f, 0.0f, 0.0f), glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(1.0f, 1.0f, 0.0f)) //
   , mLocalYAxis(glm::vec3(0.0f), glm::vec3(0.0f, 16.0f, 0.0f), glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f, 1.0f, 1.0f)) //
   , mLocalZAxis(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 16.0f), glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(1.0f, 0.0f, 1.0f)) //
{

}

void PlayState::enter()
{
   resetCamera();
   resetScene();
}

void PlayState::processInput(float deltaTime)
{
   // Close the game
   if (mWindow->keyIsPressed(GLFW_KEY_ESCAPE)) { mWindow->setShouldClose(true); }

   // Make the game full screen or windowed
   if (mWindow->keyIsPressed(GLFW_KEY_F) && !mWindow->keyHasBeenProcessed(GLFW_KEY_F))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_F);
      mWindow->setFullScreen(!mWindow->isFullScreen());

      // In the play state, the following rules are applied to the cursor:
      // - Fullscreen: Cursor is always disabled
      // - Windowed with a free camera: Cursor is disabled
      // - Windowed with a fixed camera: Cursor is enabled
      if (mWindow->isFullScreen())
      {
         // Disable the cursor when fullscreen
         mWindow->enableCursor(false);
         if (mCamera->isFree())
         {
            // Going from windowed to fullscreen changes the position of the cursor, so we reset the first move flag to avoid a jump
            mWindow->resetFirstMove();
         }
      }
      else if (!mWindow->isFullScreen())
      {
         if (mCamera->isFree())
         {
            // Disable the cursor when windowed with a free camera
            mWindow->enableCursor(false);
            // Going from fullscreen to windowed changes the position of the cursor, so we reset the first move flag to avoid a jump
            mWindow->resetFirstMove();
         }
         else
         {
            // Enable the cursor when windowed with a fixed camera
            mWindow->enableCursor(true);
         }
      }
   }

   // Change the number of samples used for anti aliasing
   if (mWindow->keyIsPressed(GLFW_KEY_1) && !mWindow->keyHasBeenProcessed(GLFW_KEY_1))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_1);
      mWindow->setNumberOfSamples(1);
   }
   else if (mWindow->keyIsPressed(GLFW_KEY_2) && !mWindow->keyHasBeenProcessed(GLFW_KEY_2))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_2);
      mWindow->setNumberOfSamples(2);
   }
   else if (mWindow->keyIsPressed(GLFW_KEY_4) && !mWindow->keyHasBeenProcessed(GLFW_KEY_4))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_4);
      mWindow->setNumberOfSamples(4);
   }
   else if (mWindow->keyIsPressed(GLFW_KEY_8) && !mWindow->keyHasBeenProcessed(GLFW_KEY_8))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_8);
      mWindow->setNumberOfSamples(8);
   }

   // Reset the camera
   if (mWindow->keyIsPressed(GLFW_KEY_R)) { resetCamera(); }

   // Make the camera free or fixed
   if (mWindow->keyIsPressed(GLFW_KEY_C) && !mWindow->keyHasBeenProcessed(GLFW_KEY_C))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_C);
      mCamera->setFree(!mCamera->isFree());

      if (!mWindow->isFullScreen())
      {
         if (mCamera->isFree())
         {
            // Disable the cursor when windowed with a free camera
            mWindow->enableCursor(false);
         }
         else
         {
            // Enable the cursor when windowed with a fixed camera
            mWindow->enableCursor(true);
         }
      }

      mWindow->resetMouseMoved();
   }

   // Move and orient the camera
   if (mCamera->isFree())
   {
      // Move
      if (mWindow->keyIsPressed(GLFW_KEY_W)) { mCamera->processKeyboardInput(Camera::MovementDirection::Forward, deltaTime); }
      if (mWindow->keyIsPressed(GLFW_KEY_S)) { mCamera->processKeyboardInput(Camera::MovementDirection::Backward, deltaTime); }
      if (mWindow->keyIsPressed(GLFW_KEY_A)) { mCamera->processKeyboardInput(Camera::MovementDirection::Left, deltaTime); }
      if (mWindow->keyIsPressed(GLFW_KEY_D)) { mCamera->processKeyboardInput(Camera::MovementDirection::Right, deltaTime); }

      // Orient
      if (mWindow->mouseMoved())
      {
         mCamera->processMouseMovement(mWindow->getCursorXOffset(), mWindow->getCursorYOffset());
         mWindow->resetMouseMoved();
      }

      // Zoom
      if (mWindow->scrollWheelMoved())
      {
         mCamera->processScrollWheelMovement(mWindow->getScrollYOffset());
         mWindow->resetScrollWheelMoved();
      }
   }
}

void PlayState::update(float deltaTime)
{

}

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void PlayState::rotateSceneWRTLocalCoordFrame(const quat& rot)
{
   mTeapot->rotateByMultiplyingCurrentRotationFromTheLeft(rot);
   mLocalXAxis.rotateByMultiplyingCurrentRotationFromTheLeft(rot);
   mLocalYAxis.rotateByMultiplyingCurrentRotationFromTheLeft(rot);
   mLocalZAxis.rotateByMultiplyingCurrentRotationFromTheLeft(rot);
}

void PlayState::rotateSceneWRTWorldCoordFrame(const quat& rot)
{
   mTeapot->rotateByMultiplyingCurrentRotationFromTheRight(rot);
   mLocalXAxis.rotateByMultiplyingCurrentRotationFromTheRight(rot);
   mLocalYAxis.rotateByMultiplyingCurrentRotationFromTheRight(rot);
   mLocalZAxis.rotateByMultiplyingCurrentRotationFromTheRight(rot);
}

void PlayState::render()
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

   {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
          counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      if (ImGui::Button("Reset rotation"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         mTeapot->setRotation(quat());
         mLocalXAxis.setRotation(quat());
         mLocalYAxis.setRotation(quat());
         mLocalZAxis.setRotation(quat());
      }

      if (ImGui::Button("Rotate by 45 degrees around local Y"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
         rotateSceneWRTLocalCoordFrame(rot);
      }

      if (ImGui::Button("Rotate by 45 degrees around local Z"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
         rotateSceneWRTLocalCoordFrame(rot);
      }

      if (ImGui::Button("Rotate by 45 degrees around world Y"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
         rotateSceneWRTWorldCoordFrame(rot);
      }

      if (ImGui::Button("Rotate by 90 degrees around world Z"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
         rotateSceneWRTWorldCoordFrame(rot);
      }

      if (ImGui::Button("Rotate by 45 degrees around local (1, 1, 1)"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 1.0f));
         rotateSceneWRTLocalCoordFrame(rot);
      }

      if (ImGui::Button("Rotate by 90 degrees around world X"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
         rotateSceneWRTWorldCoordFrame(rot);
      }

      if (ImGui::Button("Rotate by 90 degrees around local X"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rot = angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
         rotateSceneWRTLocalCoordFrame(rot);
      }

      if (ImGui::Button("Child (X) then parent (Z)"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rotZParent = angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
         quat rotXChild  = angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
         quat rot = rotZParent * rotXChild;
         rotateSceneWRTLocalCoordFrame(rot);
      }

      if (ImGui::Button("Parent (Z) then child (X)"))     // Buttons return true when clicked (most widgets return true when edited/activated)
      {
         quat rotZParent = angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
         quat rotXChild  = angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
         quat rot =  rotXChild * rotZParent;
         rotateSceneWRTLocalCoordFrame(rot);
      }

      ImGui::End();
   }

   // 3. Show another simple window.
   if (show_another_window)
   {
      ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
         show_another_window = false;
      ImGui::End();
   }

   mWindow->clearAndBindMultisampleFramebuffer();

   // Enable depth testing for 3D objects
   glEnable(GL_DEPTH_TEST);

   mLineShader->use();
   mLineShader->setMat4("projectionView", mCamera->getPerspectiveProjectionViewMatrix());

   mWorldXAxis.render(*mLineShader);
   mWorldYAxis.render(*mLineShader);
   mWorldZAxis.render(*mLineShader);

   mLocalXAxis.render(*mLineShader);
   mLocalYAxis.render(*mLineShader);
   mLocalZAxis.render(*mLineShader);

   mGameObject3DShader->use();
   mGameObject3DShader->setMat4("projectionView", mCamera->getPerspectiveProjectionViewMatrix());
   mGameObject3DShader->setVec3("cameraPos", mCamera->getPosition());

   mTable->render(*mGameObject3DShader);

   // Disable face culling so that we render the inside of the teapot
   glDisable(GL_CULL_FACE);
   mTeapot->render(*mGameObject3DShader);
   glEnable(GL_CULL_FACE);

   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

   mWindow->generateAntiAliasedImage();

   mWindow->swapBuffers();
   mWindow->pollEvents();
}

void PlayState::exit()
{

}

void PlayState::resetScene()
{

}

void PlayState::resetCamera()
{
   mCamera->reposition(glm::vec3(30.0f, 30.0f, 30.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f),
                       -45.0f,
                       -30.0f,
                       45.0f);
}
