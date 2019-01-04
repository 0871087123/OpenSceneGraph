
/* OpenSceneGraph example, osgdeferred.
 *
 *  Original code by Michael Kapelko, published to osg example with permission
 *  OSG Deferred Shading ( https://bitbucket.org/kornerr/osg-deferred-shading )
 *  Shader cleanup, removal of osgFX EffectCompositor and exchange of textures
 *  done by Christian Buchner
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include "osgrtt.h"

#include <osg/AnimationPath>
#include <osg/PolygonMode>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgUtil/TangentSpaceGenerator>

#ifdef OSG_LIBRARY_STATIC
// in case of a static build...
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(glsl)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_GRAPHICSWINDOW()
#endif


class RTTCame : public osg::Camera {
public:
	RTTCame()
	{
		setClearColor(osg::Vec4());
		setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		setRenderOrder(osg::Camera::PRE_RENDER);
		setName("RTT");

		_texture = new osg::Texture2D();
		_texture->setSourceFormat(GL_R);
		_texture->setInternalFormat(GL_R32F);
		_texture->setSourceType(GL_FLOAT);
		_texture->setTextureSize(1920, 1080);
		_texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		_texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		attach(osg::Camera::COLOR_BUFFER, _texture);

		// set draw callback
		setInitialDrawCallback(new FunctionalDrawCBK([=](osg::RenderInfo &ri){
			ri.getState()->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);
			ri.getState()->setUseModelViewAndProjectionUniforms(true);
			ri.getState()->setUseVertexAttributeAliasing(true);
		}));
		setFinalDrawCallback(new FunctionalDrawCBK([=](osg::RenderInfo &ri){
			auto state = ri.getState();
			auto ext = osg::GLExtensions::Get(ri.getContextID(), false);
			state->applyAttribute(_texture);
			std::cout << "==============" << std::endl;
			std::cout << std::to_string(glGetError()) << std::endl;
			std::cout << std::to_string(glGetError()) << std::endl;
			osg::ref_ptr<osg::Image> img = new osg::Image;
			img->readImageFromCurrentTexture(ri.getContextID(), true, GL_FLOAT);
			GLenum err = glGetError();
			std::cout << std::to_string(err) << std::endl;
			std::cout << std::to_string(*((float *)img->data() + 1)) << std::endl;
		}));
	}

	void updateSize(int width, int height) {
		setViewport(0, 0, width, height);
		resize(width, height);
		_texture->setTextureSize(width, height);
	}

	virtual ~RTTCame(){}
	osg::Texture2D * getTexture() const
	{
		return _texture.get();
	}
	osg::ref_ptr<osg::FrameBufferObject> fbo;
	osg::ref_ptr<osg::Texture2D> _texture;
};


int main()
{
	// graphic contexts
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->x = 0;
	traits->y = 0;
	traits->width = 1920;
	traits->height = 1080;
	traits->depth = 24;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->setInheritedWindowPixelFormat = true;
	// 4.3 core profile
	// traits->glContextVersion = "3.3";
	// traits->glContextFlags = 0x00000002;
	// traits->glContextProfileMask = 0x00000001;
	osg::ref_ptr<osg::GraphicsContext> gctx = osg::GraphicsContext::createGraphicsContext(traits);

    // Scene.
	osg::ref_ptr<osg::Node> scene = osgDB::readNodeFile("cow.osgt");

    // Display everything.
    osgViewer::Viewer * viewer;
	viewer = new osgViewer::Viewer;
	viewer->getCamera()->setGraphicsContext(gctx);
	viewer->getCamera()->setViewport(0, 0, traits->width, traits->height);
	viewer->getCamera()->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	viewer->getCamera()->setProjectionMatrixAsPerspective(
		30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0, 10000.0);
	viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    // add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);

    // Make screenshots with 'c'.
	viewer->addEventHandler(
        new osgViewer::ScreenCaptureHandler(
            new osgViewer::ScreenCaptureHandler::WriteToFile(
                "screenshot",
                "png",
                osgViewer::ScreenCaptureHandler::WriteToFile::OVERWRITE)));
	//viewer->setUpViewInWindow(0, 0, 1920, 1080);
	viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	// config rtt cam for test
	RTTCame * rtt = new RTTCame;
	rtt->setGraphicsContext(gctx);
	rtt->updateSize(1920, 1080);
	rtt->addChild(scene);
	viewer->addSlave(rtt, false);

	viewer->setSceneData(scene.get());
	
	return viewer->run();
}

