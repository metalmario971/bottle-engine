#include "../base/BaseHeader.h"
#include "../base/GraphicsWindow.h"
#include "../gfx/Picker.h"
#include "../gfx/RenderPipe.h"
#include "../gfx/LightManager.h"
#include "../world/NodeUtils.h"
#include "../world/PhysicsWorld.h"
#include "../world/Scene.h"


namespace BR2 {
std::shared_ptr<Scene> NodeUtils::getScene(std::shared_ptr<SceneNode> lm) {
  std::shared_ptr<TreeNode> p = std::dynamic_pointer_cast<TreeNode>(lm);

  for (int i = 0; i < 999999; ++i) {
    std::shared_ptr<SceneNode> sn = std::dynamic_pointer_cast<SceneNode>(p);
    if (sn != nullptr) {
      std::shared_ptr<Scene> s = std::dynamic_pointer_cast<Scene>(p);
      if (s != nullptr) {
        return s;
      }
    }
    else {
      break;
    }
    p = sn->getParent();
  }

  return nullptr;
}
std::shared_ptr<CameraNode> NodeUtils::getActiveCamera(std::shared_ptr<LightManager> lm) {
  return lm->getScene()->getActiveCamera();
}
std::shared_ptr<CameraNode> NodeUtils::getActiveCamera(std::shared_ptr<PhysicsWorld> lm) {
  return lm->getScene()->getActiveCamera();
}
std::shared_ptr<CameraNode> NodeUtils::getActiveCamera(std::shared_ptr<SceneNode> lm) {
  std::shared_ptr<CameraNode> ret = nullptr;
  std::shared_ptr<Scene> scene = lm->findParent<Scene>();
  if (scene) {
    ret = scene->getActiveCamera();
  }
  return ret;
}
std::shared_ptr<PhysicsWorld> NodeUtils::getPhysicsWorld(std::shared_ptr<SceneNode> lm) {
  std::shared_ptr<PhysicsWorld> ret = nullptr;
  std::shared_ptr<Scene> scene = lm->findParent<Scene>();
  if (scene) {
    ret = scene->getPhysicsManager();
  }
  return ret;
}
std::shared_ptr<Picker> NodeUtils::getPicker(std::shared_ptr<SceneNode> lm) {
  std::shared_ptr<Picker> ret = nullptr;
  std::shared_ptr<Scene> scene = lm->findParent<Scene>();
  if (scene) {
    ret = scene->getWindow()->getRenderPipe()->getPicker();
  }
  return ret;
}
}//ns Game
