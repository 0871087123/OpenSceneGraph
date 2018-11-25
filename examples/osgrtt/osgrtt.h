
#include <osg/Camera>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/RenderInfo>
#include <functional>

class FunctionalNodeCBK : public osg::NodeCallback {
public:
	FunctionalNodeCBK(const std::function<void(osg::Node *, osg::NodeVisitor *)> &functor) : functor(functor) {}
	std::function<void(osg::Node*, osg::NodeVisitor *)> functor;
	virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
		functor(node, nv);
	}
};

class FunctionalCullCBK : public osg::NodeCallback {
public:
	FunctionalCullCBK(const std::function<bool(osg::Node *, osg::NodeVisitor *)> &functor) : functor(functor) {}
	std::function<bool(osg::Node*, osg::NodeVisitor *)> functor;
	virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
		if (!functor(node, nv))
			traverse(node, nv);
	}
};

class FunctionalDrawCBK : public osg::Camera::DrawCallback {
public:
	FunctionalDrawCBK(const std::function<void(osg::RenderInfo &)> &functor) : functor(functor) {}
	std::function<void(osg::RenderInfo &)> functor;
	virtual void operator()(osg::RenderInfo &renderInfo) const {
		DrawCallback::operator()(renderInfo);
		functor(renderInfo);
	}
};

class FunctionalUniformCallback : public osg::Uniform::Callback
{
public:
	FunctionalUniformCallback(const std::function<void(osg::Uniform *, osg::NodeVisitor *)> &functor) : functor(functor) {}
	std::function<void(osg::Uniform* uniform, osg::NodeVisitor* nv)> functor;
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv){
		functor(uniform, nv);
	}
};
